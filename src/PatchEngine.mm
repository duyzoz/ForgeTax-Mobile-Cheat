// PatchEngine.mm — see header. No patch bytes here; all bytes come from
// patch_table.h (the single table, generated from JSON).
#import "PatchEngine.h"
#import "patch_table.h"

#include <mach/mach.h>
#include <mach-o/dyld.h>
#include <string.h>
#include <libkern/OSCacheControl.h>

static uintptr_t s_unitySlide = 0;
static int s_unitySlideCached = 0;

const char *FTC_PatchResultMessage(FTC_PatchResult r) {
    switch (r) {
        case FTC_PATCH_OK: return "OK";
        case FTC_PATCH_ERR_NO_IMAGE: return "Khong tim thay UnityFramework trong bo nho";
        case FTC_PATCH_ERR_NOT_APPLYABLE: return "Patch nay bi vo hieu hoa (cho bang verified)";
        case FTC_PATCH_ERR_MISMATCH: return "Bytes goc khong khop — TU CHOI ghi (game co the khac ban 3.16.1)";
        case FTC_PATCH_ERR_ALREADY: return "Trang thai da dung roi";
        case FTC_PATCH_ERR_PROTECT: return "Khong doi duoc quyen bo nho";
        default: return "Loi khong ro";
    }
}

// Find UnityFramework's vmaddr slide. __TEXT vmaddr is 0 in this binary,
// so RVA == file offset == (address - image base).
static bool FTC_FindUnitySlide(uintptr_t *outSlide) {
    if (s_unitySlideCached) { *outSlide = s_unitySlide; return true; }
    uint32_t count = _dyld_image_count();
    for (uint32_t i = 0; i < count; i++) {
        const char *name = _dyld_get_image_name(i);
        if (name && strstr(name, "UnityFramework") != NULL) {
            s_unitySlide = (uintptr_t)_dyld_get_image_vmaddr_slide(i);
            s_unitySlideCached = 1;
            *outSlide = s_unitySlide;
            return true;
        }
    }
    return false;
}

uintptr_t FTC_UnityAddressForRVA(uint64_t rva) {
    uintptr_t slide = 0;
    if (!FTC_FindUnitySlide(&slide)) return 0;
    // Image base: the mach header address of UnityFramework.
    uint32_t count = _dyld_image_count();
    for (uint32_t i = 0; i < count; i++) {
        const char *name = _dyld_get_image_name(i);
        if (name && strstr(name, "UnityFramework") != NULL) {
            uintptr_t base = (uintptr_t)_dyld_get_image_header(i);
            return base + (uintptr_t)rva;
        }
    }
    return 0;
}

static bool FTC_StatusApplyable(const char *status) {
    // Allowlist is deliberately closed: only MATCH / DERIVED / CHECKED may
    // ever be applied. "PROVISIONAL" (and any unknown status, including
    // MISMATCH) is refused — a provisional entry must be verified first
    // and re-issued with a verified status before it can be applied.
    return status && (strcmp(status, "MATCH") == 0 ||
                      strcmp(status, "DERIVED") == 0 ||
                      strcmp(status, "CHECKED") == 0);
}

// Write `bytes` (len) at `addr` after verifying `expected` is currently
// there. Returns FTC_PATCH_OK or an error; never writes on mismatch.
static FTC_PatchResult FTC_WriteVerified(uint8_t *addr, const uint8_t *expected,
                                         const uint8_t *bytes, size_t len) {
    if (memcmp(addr, bytes, len) == 0) return FTC_PATCH_ERR_ALREADY; // desired state
    if (memcmp(addr, expected, len) != 0) return FTC_PATCH_ERR_MISMATCH;
    kern_return_t kr = vm_protect(mach_task_self(), (vm_address_t)addr, len, FALSE,
                                  VM_PROT_READ | VM_PROT_WRITE | VM_PROT_COPY);
    if (kr != KERN_SUCCESS) return FTC_PATCH_ERR_PROTECT;
    memcpy(addr, bytes, len);
    // Flush the CPU instruction cache so the new bytes execute.
    sys_icache_invalidate(addr, len);
    kr = vm_protect(mach_task_self(), (vm_address_t)addr, len, FALSE,
                    VM_PROT_READ | VM_PROT_EXECUTE);
    if (kr != KERN_SUCCESS) return FTC_PATCH_ERR_PROTECT;
    sys_icache_invalidate(addr, len);
    return FTC_PATCH_OK;
}

static FTC_PatchResult FTC_ApplyOrRemoveFeature(int featureId, bool apply) {
    FTC_PatchResult firstError = FTC_PATCH_OK;
    bool anySite = false;
    bool anyChanged = false;
    for (size_t i = 0; i < kFTC_PatchTableCount; i++) {
        const FTC_PatchEntry *e = &kFTC_PatchTable[i];
        if (e->feature_id != featureId) continue;
        anySite = true;
        if (!FTC_StatusApplyable(e->status)) {
            if (firstError == FTC_PATCH_OK) firstError = FTC_PATCH_ERR_NOT_APPLYABLE;
            continue;
        }
        uintptr_t addr = FTC_UnityAddressForRVA(e->rva);
        if (addr == 0) {
            if (firstError == FTC_PATCH_OK) firstError = FTC_PATCH_ERR_NO_IMAGE;
            continue;
        }
        FTC_PatchResult r;
        if (apply) {
            r = FTC_WriteVerified((uint8_t *)addr, e->original, e->patched, e->len);
        } else {
            r = FTC_WriteVerified((uint8_t *)addr, e->patched, e->original, e->len);
        }
        if (r == FTC_PATCH_OK) {
            anyChanged = true;
        } else if (r == FTC_PATCH_ERR_ALREADY) {
            // Already in the desired state — not an error for this site.
            continue;
        } else if (firstError == FTC_PATCH_OK) {
            firstError = r;
        }
    }
    if (!anySite) return FTC_PATCH_ERR_NOT_APPLYABLE;
    if (anyChanged) return FTC_PATCH_OK;
    return firstError; // OK when everything was already in the desired state
}

FTC_PatchResult FTC_PatchApplyFeature(int featureId) {
    @try {
        return FTC_ApplyOrRemoveFeature(featureId, true);
    } @catch (...) {
        return FTC_PATCH_ERR_PROTECT;
    }
}

FTC_PatchResult FTC_PatchRemoveFeature(int featureId) {
    @try {
        return FTC_ApplyOrRemoveFeature(featureId, false);
    } @catch (...) {
        return FTC_PATCH_ERR_PROTECT;
    }
}

bool FTC_PatchFeatureIsApplied(int featureId) {
    bool anySite = false;
    for (size_t i = 0; i < kFTC_PatchTableCount; i++) {
        const FTC_PatchEntry *e = &kFTC_PatchTable[i];
        if (e->feature_id != featureId) continue;
        anySite = true;
        uintptr_t addr = FTC_UnityAddressForRVA(e->rva);
        if (addr == 0) return false;
        if (memcmp((const void *)addr, e->patched, e->len) != 0) return false;
    }
    return anySite;
}
