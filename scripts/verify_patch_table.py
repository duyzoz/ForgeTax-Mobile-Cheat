#!/usr/bin/env python3
"""Re-check patch_table_verified.json against a base IPA (or an extracted
UnityFramework) before/after a build.

Usage:
    python3 scripts/verify_patch_table.py <patch_table_verified.json>
        (--ipa <base.ipa> | --unity <UnityFramework path>)

Checks:
  1. The UnityFramework inside the IPA (or the given file) has the SHA-256
     recorded in the JSON meta (40a7f740...). Any other binary -> fail.
  2. EVERY entry's original_bytes match the file bytes at its RVA
     (RVA == file offset, __TEXT vmaddr 0) — all 58 entries: the 20
     applyable patch entries, the 37 call entries (their original_bytes
     are the function prologues), and the 1 disabled MISMATCH entry
     (its recorded bytes are the real file bytes at that address; only
     the old patch claim about them was wrong). Any mismatch -> fail.
  3. src/patch_table.h (next to the JSON) is in sync with the JSON: the
     header is re-generated in memory (to a temp file) with
     scripts/patch_table_to_header.py and diffed against the checked-in
     header — any drift -> fail. And the stale RVA 0x275D428 must not
     appear as the RVA field of any entry in the header's applyable
     table (mentions in comments do not count).
  4. No banned address (0x275D428, the stale OfferPrice site) appears as
     an applyable entry in the JSON.

Exit code 0 = all good, nonzero = mismatch (prints which entries failed).
This script never writes to the IPA or the binary.
"""
import hashlib
import io
import json
import os
import re
import subprocess
import sys
import tempfile
import zipfile


def sha256_file(path):
    h = hashlib.sha256()
    with open(path, "rb") as f:
        for chunk in iter(lambda: f.read(1 << 20), b""):
            h.update(chunk)
    return h.hexdigest()


def unity_from_ipa(ipa_path):
    with zipfile.ZipFile(ipa_path) as z:
        names = [n for n in z.namelist()
                 if n.endswith("UnityFramework.framework/UnityFramework")]
        if not names:
            raise SystemExit("IPA has no UnityFramework binary: %s" % ipa_path)
        return z.read(names[0])


def main():
    if len(sys.argv) < 2:
        print(__doc__)
        return 2
    table_path = sys.argv[1]
    ipa = unity = None
    args = sys.argv[2:]
    for i, a in enumerate(args):
        if a == "--ipa":
            ipa = args[i + 1]
        elif a == "--unity":
            unity = args[i + 1]
    if bool(ipa) == bool(unity):
        print("give exactly one of --ipa <base.ipa> or --unity <UnityFramework>")
        return 2

    table = json.load(open(table_path, encoding="utf-8"))
    meta = table.get("meta", {})
    expected_sha = meta.get("unityframework_sha256")
    data = unity_from_ipa(ipa) if ipa else open(unity, "rb").read()
    actual_sha = hashlib.sha256(data).hexdigest()
    print("UnityFramework SHA-256: %s" % actual_sha)
    if expected_sha and actual_sha != expected_sha:
        print("FAIL: SHA mismatch — expected %s (JSON meta)" % expected_sha)
        return 1
    print("SHA matches JSON meta.")

    failures = []
    kept = dropped = calls = 0
    checked = 0
    for e in table["entries"]:
        is_call = e.get("kind") == "call"
        is_disabled = bool(e.get("do_not_apply")) or e.get("status") == "MISMATCH" \
            or (not is_call and not e.get("patched_bytes"))
        if is_call:
            calls += 1
        elif is_disabled:
            dropped += 1
        else:
            kept += 1
        # Check 2: verify original_bytes for EVERY entry that carries them
        # (patch, call and disabled alike) — a call entry's prologue and
        # even the disabled entry's recorded bytes must match the binary.
        if not e.get("original_bytes"):
            failures.append("#%s %s @ %s: no original_bytes to verify" % (
                e.get("feature"), e["name"], e.get("rva")))
            continue
        checked += 1
        rva = int(e["rva"], 16)
        want = bytes.fromhex(e["original_bytes"])
        got = data[rva:rva + len(want)]
        if got != want:
            failures.append("#%s %s @ %s: file has %s, table says %s" % (
                e.get("feature"), e["name"], e["rva"], got.hex(), want.hex()))
    print("entries: %d patch applyable, %d call (reference), %d disabled; "
          "original bytes checked for %d/%d entries"
          % (kept, calls, dropped, checked, len(table["entries"])))
    if failures:
        print("FAIL: %d original-bytes mismatches:" % len(failures))
        for f in failures:
            print("  " + f)
        return 1
    print("All %d entries' original bytes match the binary." % checked)

    # Check 3: src/patch_table.h must be exactly what the JSON generates,
    # and the stale RVA must not be an entry in its applyable table.
    repo_root = os.path.dirname(os.path.abspath(table_path))
    header_path = os.path.join(repo_root, "src", "patch_table.h")
    gen_path = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                            "patch_table_to_header.py")
    if os.path.isfile(header_path) and os.path.isfile(gen_path):
        with tempfile.NamedTemporaryFile(suffix=".h", delete=False) as tmp:
            regen_path = tmp.name
        try:
            r = subprocess.run([sys.executable, gen_path, table_path, regen_path],
                               capture_output=True, text=True)
            if r.returncode != 0:
                print("FAIL: header regen failed: %s" % (r.stderr or r.stdout))
                return 1
            regen = open(regen_path, "rb").read()
            current = open(header_path, "rb").read()
            if regen != current:
                print("FAIL: src/patch_table.h is out of sync with %s" % table_path)
                print("Regenerate with: python3 scripts/patch_table_to_header.py "
                      "%s src/patch_table.h" % os.path.basename(table_path))
                return 1
            print("Header src/patch_table.h matches a fresh regen from the JSON.")
            # Grep the applyable table only: parse the RVA field of each
            # entry line inside kFTC_PatchTable[] — comments (which mention
            # 0x275D428 legitimately) do not count.
            text = current.decode("utf-8", "replace")
            m = re.search(r"kFTC_PatchTable\[\] = \{(.*?)\n\};", text, re.S)
            if not m:
                print("FAIL: could not find kFTC_PatchTable[] in src/patch_table.h")
                return 1
            stale_in_header = []
            for line in m.group(1).splitlines():
                rm = re.match(r'\s*\{\s*\d+,\s*"[^"]*",\s*(0x[0-9a-fA-F]+)', line)
                if rm and rm.group(1).lower() == "0x275d428":
                    stale_in_header.append(line.strip())
            if stale_in_header:
                print("FAIL: stale RVA 0x275D428 is an applyable entry in "
                      "src/patch_table.h: %s" % stale_in_header)
                return 1
            print("Stale RVA 0x275D428 is not an applyable entry in src/patch_table.h. OK.")
        finally:
            try:
                os.unlink(regen_path)
            except OSError:
                pass
    else:
        print("Header check skipped: %s or %s not found next to the JSON."
              % (header_path, gen_path))

    # Stale-address guard: 0x275D428 must never be an applyable entry.
    stale = [e for e in table["entries"]
             if str(e.get("rva", "")).lower() == "0x275d428"
             and not e.get("do_not_apply")]
    if stale:
        print("FAIL: stale OfferPrice address 0x275D428 is applyable: %s" % stale)
        return 1
    print("Stale address 0x275D428 is disabled (MISMATCH, do_not_apply=true). OK.")
    print("VERIFY OK")
    return 0


if __name__ == "__main__":
    sys.exit(main())
