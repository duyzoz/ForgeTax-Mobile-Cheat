#!/usr/bin/env python3
"""Convert a patch-table JSON into src/patch_table.h.

Usage:
    python3 scripts/patch_table_to_header.py <table.json> <out.h>

Input JSON format (CIPHER's patch_table_verified.json):
    {"meta": {...}, "entries": [
      {"feature": 13, "name": "PurchaseMerchant (free merchant goods)",
       "kind": "patch" | "call",
       "rva": "0x2757D60",
       "original_bytes": "d5000037", "patched_bytes": "06000014",
       "status": "MATCH" | "DERIVED" | "CHECKED" | "MISMATCH",
       "do_not_apply": false,
       "notes": "..."}]}

Rules (hard):
  - Only kind == "patch" entries with non-null patched_bytes, status in
    (MATCH, DERIVED, CHECKED) and do_not_apply != true are emitted into the
    applyable table. kind == "call" entries are reference only (function
    entries verified for GameBridge's IL2CPP calls); they carry no patch
    bytes and are NOT emitted.
  - Entries with status MISMATCH, or do_not_apply=true, are NOT emitted
    into the applyable table. They are listed in a comment block instead,
    so the feature is visibly disabled and cannot be applied by accident.
    (Known case: OfferPrice stale address 0x275D428 — the correct site is
    0x275D430 and is the only OfferPrice entry in the applyable table.)
  - The generated header is the SINGLE source of patch bytes in the repo.
    No patch bytes may live anywhere else in src/.
"""
import json
import sys


def c_bytes(hexstr):
    b = bytes.fromhex(hexstr)
    return ", ".join("0x%02x" % x for x in b), len(b)


def main():
    src, dst = sys.argv[1], sys.argv[2]
    with open(src, "r", encoding="utf-8") as f:
        table = json.load(f)
    meta = table.get("meta", {})
    entries = table["entries"]
    kept, dropped, calls = [], [], []
    for e in entries:
        if e.get("kind") == "call":
            calls.append(e)
            continue
        if e.get("do_not_apply") or e.get("status") == "MISMATCH":
            dropped.append(e)
        elif e.get("status") not in ("MATCH", "DERIVED", "CHECKED"):
            dropped.append(e)
        elif not e.get("patched_bytes"):
            dropped.append(e)
        else:
            kept.append(e)

    lines = []
    lines.append("// GENERATED FILE — do not hand-edit.")
    lines.append("// Generated from %s by scripts/patch_table_to_header.py" % src)
    lines.append("// This is the ONLY place patch bytes live in the repo.")
    lines.append("//")
    lines.append("// VERIFIED table (CIPHER): %d entries total = %d patch applyable"
                 % (len(entries), len(kept)))
    lines.append("//   + %d call targets CHECKED (function entries verified, reference"
                 % len(calls))
    lines.append("//   only — see the JSON, not emitted here) + %d disabled (below)." % len(dropped))
    if meta:
        lines.append("// Target: %s" % meta.get("target", ""))
        lines.append("// UnityFramework SHA-256: %s" % meta.get("unityframework_sha256", ""))
        lines.append("// IPA SHA-256: %s" % meta.get("ipa_sha256", ""))
        lines.append("// cryptid=%s; %s" % (meta.get("cryptid", "?"), meta.get("mach_o", "")))
    lines.append("//")
    lines.append("// Status meanings (per JSON meta): MATCH = README claim verified")
    lines.append("//   against file bytes + capstone; DERIVED = patched bytes derived")
    lines.append("//   by the verifier from disassembly + capstone (README gave no")
    lines.append("//   bytes); CHECKED = call target is a valid function entry.")
    lines.append("// Entries with status MISMATCH / do_not_apply=true are NOT in this")
    lines.append("// table (see the disabled list below) and can never be applied.")
    lines.append("//")
    lines.append("// #4 Guard: the OLD patch (MOV w0,#3; RET at entry 0x2751550) was")
    lines.append("//   REJECTED — it demotes Great(4) to Success(3). The entry below")
    lines.append("//   patches 0x2751574 (csel w0,w8,w9,eq -> mov w0,w8): always the")
    lines.append("//   AlwaysSuccess result (3, or 4 if the input was Great).")
    lines.append("//   SIDE EFFECT: while this patch is ON, CycleForgeMode has no")
    lines.append("//   effect on Guard anymore (always AlwaysSuccess). See")
    lines.append("//   FeatureCatalog.h #4 and README.md.")
    lines.append("// #14 OfferPrice: ONLY 0x275D430 is in this table. 0x275D428 is a")
    lines.append("//   stale address (function entry `ldr w8,[x0,#0x200]`) and is in")
    lines.append("//   the disabled list below — it must never be patched.")
    lines.append("#ifndef FORGETAX_PATCH_TABLE_H")
    lines.append("#define FORGETAX_PATCH_TABLE_H")
    lines.append("")
    lines.append('#include <stddef.h>')
    lines.append('#include <stdint.h>')
    lines.append("")
    lines.append("typedef struct {")
    lines.append("    int feature_id;          // README feature number (#2..#40)")
    lines.append("    const char *name;")
    lines.append("    uint64_t rva;            // == file offset (__TEXT vmaddr 0)")
    lines.append("    const uint8_t *original; // bytes expected at RVA before patch")
    lines.append("    const uint8_t *patched;  // bytes written when the feature is ON")
    lines.append("    size_t len;")
    lines.append('    const char *status;      // "MATCH" | "DERIVED" | "CHECKED"')
    lines.append("} FTC_PatchEntry;")
    lines.append("")

    for i, e in enumerate(kept):
        ob, n1 = c_bytes(e["original_bytes"])
        pb, n2 = c_bytes(e["patched_bytes"])
        assert n1 == n2, "len mismatch in %s" % e["name"]
        lines.append("static const uint8_t kPatchOrig_%d[] = { %s }; // %s" % (i, ob, e["name"]))
        lines.append("static const uint8_t kPatchNew_%d[] = { %s };" % (i, pb))

    lines.append("")
    lines.append("static const FTC_PatchEntry kFTC_PatchTable[] = {")
    for i, e in enumerate(kept):
        ob, n = c_bytes(e["original_bytes"])
        note = (e.get("notes") or "").replace('"', "'").replace("\n", " ")
        lines.append('    { %d, "%s", %s, kPatchOrig_%d, kPatchNew_%d, %d, "%s" }, // %s'
                     % (e["feature"], e["name"], e["rva"], i, i, n,
                        e["status"], note))
    lines.append("};")
    lines.append("static const size_t kFTC_PatchTableCount = sizeof(kFTC_PatchTable) / sizeof(kFTC_PatchTable[0]);")
    lines.append("")
    if dropped:
        lines.append("// ---- DISABLED entries (present in source JSON, NOT applyable) ----")
        for e in dropped:
            lines.append("// feature #%s %s @ %s status=%s do_not_apply=%s — %s" % (
                e.get("feature"), e["name"], e["rva"], e.get("status"),
                e.get("do_not_apply"), (e.get("notes") or "").replace("\n", " ")))
    else:
        lines.append("// No disabled entries in the source table.")
    lines.append("")
    lines.append("#endif // FORGETAX_PATCH_TABLE_H")
    with open(dst, "w", encoding="utf-8") as f:
        f.write("\n".join(lines) + "\n")
    print("wrote %s: %d applyable patch, %d call (reference only), %d disabled"
          % (dst, len(kept), len(calls), len(dropped)))


if __name__ == "__main__":
    main()
