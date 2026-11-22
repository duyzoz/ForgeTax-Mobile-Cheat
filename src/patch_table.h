// GENERATED FILE — do not hand-edit.
// Generated from patch_table_verified.json by scripts/patch_table_to_header.py
// This is the ONLY place patch bytes live in the repo.
//
// VERIFIED table (CIPHER): 58 entries total = 20 patch applyable
//   + 37 call targets CHECKED (function entries verified, reference
//   only — see the JSON, not emitted here) + 1 disabled (below).
// Target: ForgeTax iOS 3.16.1 (build 95), bundle com.soucean.fruityzoo, Unity 6000.0.83f1
// UnityFramework SHA-256: 40a7f74082350afc2312b4c6e9fc224258e8900b01051825393936714c17f158
// IPA SHA-256: 65f89d3496820087b9476c826c0692ecca8b61c45dbe8a05a78d42f44af87cdf
// cryptid=0; MH_MAGIC_64, cputype arm64 (0x100000c), __TEXT vmaddr=0x0 fileoff=0x0 -> RVA == file offset (verified from segment headers, not assumed)
//
// Status meanings (per JSON meta): MATCH = README claim verified
//   against file bytes + capstone; DERIVED = patched bytes derived
//   by the verifier from disassembly + capstone (README gave no
//   bytes); CHECKED = call target is a valid function entry.
// Entries with status MISMATCH / do_not_apply=true are NOT in this
// table (see the disabled list below) and can never be applied.
//
// #4 Guard: the OLD patch (MOV w0,#3; RET at entry 0x2751550) was
//   REJECTED — it demotes Great(4) to Success(3). The entry below
//   patches 0x2751574 (csel w0,w8,w9,eq -> mov w0,w8): always the
//   AlwaysSuccess result (3, or 4 if the input was Great).
//   SIDE EFFECT: while this patch is ON, CycleForgeMode has no
//   effect on Guard anymore (always AlwaysSuccess). See
//   FeatureCatalog.h #4 and README.md.
// #14 OfferPrice: ONLY 0x275D430 is in this table. 0x275D428 is a
//   stale address (function entry `ldr w8,[x0,#0x200]`) and is in
//   the disabled list below — it must never be patched.
#ifndef FORGETAX_PATCH_TABLE_H
#define FORGETAX_PATCH_TABLE_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
    int feature_id;          // README feature number (#2..#40)
    const char *name;
    uint64_t rva;            // == file offset (__TEXT vmaddr 0)
    const uint8_t *original; // bytes expected at RVA before patch
    const uint8_t *patched;  // bytes written when the feature is ON
    size_t len;
    const char *status;      // "MATCH" | "DERIVED" | "CHECKED"
} FTC_PatchEntry;

static const uint8_t kPatchOrig_0[] = { 0x00, 0x01, 0x89, 0x1a }; // Guard (forge AlwaysSuccess)
static const uint8_t kPatchNew_0[] = { 0xe0, 0x03, 0x08, 0x2a };
static const uint8_t kPatchOrig_1[] = { 0xf6, 0x57, 0xbd, 0xa9, 0xf4, 0x4f, 0x01, 0xa9 }; // FailureChance (break rate -> 0)
static const uint8_t kPatchNew_1[] = { 0xe0, 0x03, 0x67, 0x9e, 0xc0, 0x03, 0x5f, 0xd6 };
static const uint8_t kPatchOrig_2[] = { 0xfc, 0x6f, 0xba, 0xa9, 0xfa, 0x67, 0x01, 0xa9 }; // TopGradeBreak.Tickets (top-grade break -> 0)
static const uint8_t kPatchNew_2[] = { 0x00, 0x00, 0x80, 0x52, 0xc0, 0x03, 0x5f, 0xd6 };
static const uint8_t kPatchOrig_3[] = { 0x00, 0x01, 0x00, 0xb4 }; // Fever Expire (fever never expires)
static const uint8_t kPatchNew_3[] = { 0xc0, 0x03, 0x5f, 0xd6 };
static const uint8_t kPatchOrig_4[] = { 0xf8, 0x5f, 0xbc, 0xa9, 0xf6, 0x57, 0x01, 0xa9, 0xf4, 0x4f, 0x02, 0xa9, 0xfd, 0x7b, 0x03, 0xa9, 0xfd, 0xc3, 0x00, 0x91 }; // ShowRewarded (skip rewarded-ad video)
static const uint8_t kPatchNew_4[] = { 0x03, 0x0c, 0x40, 0xf9, 0x02, 0x14, 0x40, 0xf9, 0x00, 0x20, 0x40, 0xf9, 0x21, 0x00, 0x80, 0x52, 0x60, 0x00, 0x1f, 0xd6 };
static const uint8_t kPatchOrig_5[] = { 0xd5, 0x00, 0x00, 0x37 }; // PurchaseMerchant (free merchant goods)
static const uint8_t kPatchNew_5[] = { 0x06, 0x00, 0x00, 0x14 };
static const uint8_t kPatchOrig_6[] = { 0x1f, 0x05, 0x00, 0x71 }; // OfferPrice (free shop relics)
static const uint8_t kPatchNew_6[] = { 0x1f, 0x01, 0x08, 0x6b };
static const uint8_t kPatchOrig_7[] = { 0xe9, 0x23, 0xbc, 0x6d }; // ChargeReroll (free reroll)
static const uint8_t kPatchNew_7[] = { 0xc0, 0x03, 0x5f, 0xd6 };
static const uint8_t kPatchOrig_8[] = { 0xf6, 0x57, 0xbd, 0xa9, 0xf4, 0x4f, 0x01, 0xa9 }; // ContractQuota (quota -> 1 gold)
static const uint8_t kPatchNew_8[] = { 0x00, 0x10, 0x6e, 0x1e, 0xc0, 0x03, 0x5f, 0xd6 };
static const uint8_t kPatchOrig_9[] = { 0x00, 0x28, 0x40, 0xb9, 0x01, 0x00, 0x80, 0xd2, 0x44, 0x67, 0x00, 0x14 }; // get_MaxLives (99 lives)
static const uint8_t kPatchNew_9[] = { 0x60, 0x0c, 0x80, 0x52, 0xc0, 0x03, 0x5f, 0xd6, 0x1f, 0x20, 0x03, 0xd5 };
static const uint8_t kPatchOrig_10[] = { 0x21, 0x38, 0x60, 0x1e }; // DamageDragon (one-shot dragon)
static const uint8_t kPatchNew_10[] = { 0xe1, 0x03, 0x67, 0x9e };
static const uint8_t kPatchOrig_11[] = { 0x00, 0x80, 0x44, 0x39, 0xc0, 0x03, 0x5f, 0xd6 }; // get_PlayerInvulnerable (invulnerable in dragon fight)
static const uint8_t kPatchNew_11[] = { 0x20, 0x00, 0x80, 0x52, 0xc0, 0x03, 0x5f, 0xd6 };
static const uint8_t kPatchOrig_12[] = { 0x00, 0x88, 0x40, 0xbd, 0x08, 0x20, 0x20, 0x1e }; // get_Stunned (dragon permanently stunned)
static const uint8_t kPatchNew_12[] = { 0x20, 0x00, 0x80, 0x52, 0xc0, 0x03, 0x5f, 0xd6 };
static const uint8_t kPatchOrig_13[] = { 0x08, 0x10, 0x44, 0x39, 0x68, 0x00, 0x00, 0x36 }; // get_Enraged (dragon never enraged)
static const uint8_t kPatchNew_13[] = { 0x00, 0x00, 0x80, 0x52, 0xc0, 0x03, 0x5f, 0xd6 };
static const uint8_t kPatchOrig_14[] = { 0x00, 0x8c, 0x41, 0xbd, 0xc0, 0x03, 0x5f, 0xd6 }; // get_FlightChance (dragon never flies)
static const uint8_t kPatchNew_14[] = { 0x00, 0x04, 0x00, 0x4f, 0xc0, 0x03, 0x5f, 0xd6 };
static const uint8_t kPatchOrig_15[] = { 0x29, 0x09, 0x00, 0x11 }; // HatchCocoon (cocoon hatches at once)
static const uint8_t kPatchNew_15[] = { 0xe9, 0x03, 0x08, 0x2a };
static const uint8_t kPatchOrig_16[] = { 0x00, 0x01, 0x00, 0x34 }; // TryFuse (fuse immediately)
static const uint8_t kPatchNew_16[] = { 0x08, 0x00, 0x00, 0x14 };
static const uint8_t kPatchOrig_17[] = { 0x3d, 0xfe, 0xff, 0x97 }; // UseConsumable (infinite consumable uses)
static const uint8_t kPatchNew_17[] = { 0x1f, 0x20, 0x03, 0xd5 };
static const uint8_t kPatchOrig_18[] = { 0x08, 0x74, 0x40, 0xb9, 0x09, 0xc8, 0x40, 0xb9 }; // get_HellDropDue (Hell Drop every dragon win)
static const uint8_t kPatchNew_18[] = { 0x20, 0x00, 0x80, 0x52, 0xc0, 0x03, 0x5f, 0xd6 };
static const uint8_t kPatchOrig_19[] = { 0xa1, 0xae, 0x40, 0xfd }; // SwordValuation.Value (sword value x10)
static const uint8_t kPatchNew_19[] = { 0x01, 0x90, 0x64, 0x1e };

static const FTC_PatchEntry kFTC_PatchTable[] = {
    { 4, "Guard (forge AlwaysSuccess)", 0x2751574, kPatchOrig_0, kPatchNew_0, 4, "DERIVED" }, // Guard fn entry 0x2751550; patch at +0x24 replaces final 'csel w0,w8,w9,eq' with 'mov w0,w8' -> forces the mode-2 (AlwaysSuccess) result (3, or 4 if input was Great) regardless of mode. REJECTED old CHEAT-SURFACE bytes 'MOV w0,#3; RET' (60008052c0035fd6) at entry: HammerStrike.Resolve (0x24d6eec) calls Guard with the Roll result, which can be Great(4); forcing 3 demotes Great->Success, contradicting README s2 'Great van Great'. Capstone: e003082a = mov w0, w8. Side effect: NoBreak/AlwaysSuccess menu toggle becomes moot for Guard (always AlwaysSuccess). [capstone(patched): mov w0, w8]
    { 5, "FailureChance (break rate -> 0)", 0x2750a24, kPatchOrig_1, kPatchNew_1, 8, "DERIVED" }, // dump.cs: 'internal static double FailureChance(int grade)' -> FMOV d0,xzr; RET. Capstone: e003679e = fmov d0, xzr. [capstone(patched): fmov d0, xzr; ret]
    { 5, "TopGradeBreak.Tickets (top-grade break -> 0)", 0x27739e8, kPatchOrig_2, kPatchNew_2, 8, "DERIVED" }, // dump.cs: 'public static int Tickets(bool fever, int trophies)' -> MOV w0,#0; RET. [capstone(patched): mov w0, #0; ret]
    { 10, "Fever Expire (fever never expires)", 0x27644f0, kPatchOrig_3, kPatchNew_3, 4, "MATCH" }, // README gives RET only; original bytes read from file: cbz x0,#0x2764510. Function is void (clears feverUntil +0xd4/+0x54); RET at entry is a safe no-op. Encoding verified by capstone. [capstone(patched): ret]
    { 11, "ShowRewarded (skip rewarded-ad video)", 0x273b224, kPatchOrig_4, kPatchNew_4, 20, "MATCH" }, // 20-byte delegate trampoline stub. Capstone decodes stub as: ldr x3,[x0,#0x18]; ldr x2,[x0,#0x28]; ldr x0,[x0,#0x40]; mov w1,#1; br x3 -- matches README delegate layout (method_ptr +0x18, method +0x28, target +0x40), calls finished(true). Original bytes (prologue) read from file; not stated in README. [capstone(patched): ldr x3, [x0, #0x18]; ldr x2, [x0, #0x28]; ldr x0, [x0, #0x40]; mov w1, #1; br x3]
    { 13, "PurchaseMerchant (free merchant goods)", 0x2757d60, kPatchOrig_5, kPatchNew_5, 4, "MATCH" }, // Original tbnz w21,#0,#0x2757d78 -> b #0x18; branch target 0x2757d78 identical to the tbnz target, skipping the wallet subtraction (ldr d0 wallet; fsub; str). [capstone(patched): b #0x2757d78]
    { 14, "OfferPrice (free shop relics)", 0x275d430, kPatchOrig_6, kPatchNew_6, 4, "MATCH" }, // Original cmp w8,#1 -> cmp w8,w8 (always equal) so the following b.eq always returns 0.0. Function entry is 0x275D428 (ldr w8,[x0,#0x200]); the cmp is at +8. [capstone(patched): cmp w8, w8]
    { 15, "ChargeReroll (free reroll)", 0x275da0c, kPatchOrig_7, kPatchNew_7, 4, "MATCH" }, // README gives RET only; original bytes read from file: stp d9,d8,[sp,#-0x40]! (prologue). dump.cs: 'private void ChargeReroll(double rerollPrice)' -> RET at entry is a safe void no-op. [capstone(patched): ret]
    { 16, "ContractQuota (quota -> 1 gold)", 0x27604cc, kPatchOrig_8, kPatchNew_8, 8, "MATCH" }, // Patched bytes decode (capstone) as 'fmov d0, #1.00000000; ret'. dump.cs: 'public double ContractQuota(int)'. [capstone(patched): fmov d0, #1.00000000; ret]
    { 17, "get_MaxLives (99 lives)", 0x27546d4, kPatchOrig_9, kPatchNew_9, 12, "MATCH" }, // Original 12B: ldr w0,[x0,#0x28]; mov x1,#0; b #0x276e3ec. Patched: mov w0,#99 (0x63); ret; nop. dump.cs: 'public int get_MaxLives()'. Capstone: 600c8052 = mov w0, #0x63. [capstone(patched): mov w0, #0x63; ret; nop]
    { 18, "DamageDragon (one-shot dragon)", 0x2778f74, kPatchOrig_10, kPatchNew_10, 4, "MATCH" }, // Original fsub d1,d1,d0 -> fmov d1,xzr; next insn stores d1 to HP at [x19,#0x198], so HP becomes 0. Capstone: e103679e = fmov d1, xzr. [capstone(patched): fmov d1, xzr]
    { 19, "get_PlayerInvulnerable (invulnerable in dragon fight)", 0x277639c, kPatchOrig_11, kPatchNew_11, 8, "DERIVED" }, // Body verified: ldrb w0,[x0,#0x120]; ret. Patch MOV w0,#1; RET (bytes derived by me, capstone-verified). Setter at 0x27763a4 untouched (patch is 8B only). [capstone(patched): mov w0, #1; ret]
    { 20, "get_Stunned (dragon permanently stunned)", 0x2777714, kPatchOrig_12, kPatchNew_12, 8, "DERIVED" }, // Body verified: ldr s0,[x0,#0x88]; fcmp s0,#0.0; cset w0,gt; ret (stunned while stun clock > 0). Patch MOV w0,#1; RET (bytes derived by me, capstone-verified); patch's own RET at +4 shadows the rest. [capstone(patched): mov w0, #1; ret]
    { 21, "get_Enraged (dragon never enraged)", 0x2777c8c, kPatchOrig_13, kPatchNew_13, 8, "DERIVED" }, // Body verified: ldrb w8,[x0,#0x104]; tbz w8,#0,...; HP < 30% max check (fmul 0.3). Patch MOV w0,#0; RET (bytes derived by me, capstone-verified). [capstone(patched): mov w0, #0; ret]
    { 22, "get_FlightChance (dragon never flies)", 0x277747c, kPatchOrig_14, kPatchNew_14, 8, "MATCH" }, // Original ldr s0,[x0,#0x18c]; ret. Patched 'movi v0.4s,#0; ret' zeroes s0 (float return). dump.cs: 'public float get_FlightChance()'. [capstone(patched): movi v0.4s, #0; ret]
    { 34, "HatchCocoon (cocoon hatches at once)", 0x275afc8, kPatchOrig_15, kPatchNew_15, 4, "MATCH" }, // Original add w9,w9,#2 -> mov w9,w8. Semantics verified from full function: w9=cocoon round, w8=current round; cmp w8,w9; b.ge 0x275b0ec (hatch path: clears cocoon, +3 supplyBonus at +0xbc, returns 1). With w9=w8 the branch is always taken. Capstone: e903082a = mov w9, w8. [capstone(patched): mov w9, w8]
    { 35, "TryFuse (fuse immediately)", 0x275a7b4, kPatchOrig_16, kPatchNew_16, 4, "MATCH" }, // Original cbz w0,#0x275a7d4 (skip fuse when Rank(fusion)!=0) -> b #0x20 to the SAME target 0x275a7d4 (fuse body). Always fuses, even if a fusion already exists (duplicate, as README notes). [capstone(patched): b #0x275a7d4]
    { 36, "UseConsumable (infinite consumable uses)", 0x27537a4, kPatchOrig_17, kPatchNew_17, 4, "MATCH" }, // Original bl #0x2753098 (AddUses, the use-count decrement) -> nop. The Uses>0 gate upstream is kept. [capstone(patched): nop]
    { 38, "get_HellDropDue (Hell Drop every dragon win)", 0x275f8ec, kPatchOrig_18, kPatchNew_18, 8, "MATCH" }, // Original ldr w8,[x0,#0x74]; ldr w9,[x0,#0xc8]. Patch MOV w0,#1; RET (bytes given in README, capstone-verified). dump.cs: 'public bool get_HellDropDue()'. [capstone(patched): mov w0, #1; ret]
    { 39, "SwordValuation.Value (sword value x10)", 0x276f380, kPatchOrig_19, kPatchNew_19, 4, "MATCH" }, // Original ldr d1,[x21,#0x158] -> fmov d1,#10.0. Capstone: 0190641e = fmov d1, #10.00000000. Replaces the multiplier load; Great x2.5 still applies on top. [capstone(patched): fmov d1, #10.00000000]
};
static const size_t kFTC_PatchTableCount = sizeof(kFTC_PatchTable) / sizeof(kFTC_PatchTable[0]);

// ---- DISABLED entries (present in source JSON, NOT applyable) ----
// feature #14 OfferPrice STALE address 0x275D428 (do not use) @ 0x275d428 status=MISMATCH do_not_apply=True — Stale address from old notes. File bytes are 'ldr w8,[x0,#0x200]' (function entry), NOT 'cmp w8,#1'. Patching the cmp bytes here would corrupt the load. Correct site is 0x275D430.

#endif // FORGETAX_PATCH_TABLE_H
