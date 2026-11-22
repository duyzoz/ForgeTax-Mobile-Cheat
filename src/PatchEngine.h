// PatchEngine.h — runtime byte patching for UnityFramework.
//
// Method: find UnityFramework's in-memory base via dyld, write bytes at
// RVA (+ slide), verify original bytes FIRST, restore on disable.
// The file on disk is never touched; the game binary stays byte-identical.
#ifndef FORGETAX_PATCH_ENGINE_H
#define FORGETAX_PATCH_ENGINE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    FTC_PATCH_OK = 0,
    FTC_PATCH_ERR_NO_IMAGE,      // UnityFramework not found in dyld images
    FTC_PATCH_ERR_NOT_APPLYABLE, // entry disabled / unknown status
    FTC_PATCH_ERR_MISMATCH,      // bytes on disk in memory != expected original
    FTC_PATCH_ERR_ALREADY,       // already patched (apply) / already clean (remove)
    FTC_PATCH_ERR_PROTECT,       // vm_protect failed
} FTC_PatchResult;

// Human-readable (Vietnamese) message for a result, for display in the menu.
const char *FTC_PatchResultMessage(FTC_PatchResult r);

// Apply / remove ALL patch sites belonging to one feature id.
// apply: verify original bytes, then write patched bytes.
// remove: verify patched bytes (or original bytes), then restore originals.
// Returns the first error encountered; partial application is reported
// per-site in the out-status array when non-NULL.
FTC_PatchResult FTC_PatchApplyFeature(int featureId);
FTC_PatchResult FTC_PatchRemoveFeature(int featureId);

// True if every site of the feature currently holds patched bytes.
bool FTC_PatchFeatureIsApplied(int featureId);

// Address of RVA inside the loaded UnityFramework (base + slide + rva),
// or 0 when the image is not loaded yet.
uintptr_t FTC_UnityAddressForRVA(uint64_t rva);

#ifdef __cplusplus
}
#endif

#endif // FORGETAX_PATCH_ENGINE_H
