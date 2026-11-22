#!/usr/bin/env python3
"""Insert an LC_LOAD_DYLIB for ForgeTaxCheat.dylib into the main executable
of an extracted .app, without touching UnityFramework or any other file.

Usage:
    python3 scripts/inject_dylib.py <path/to/AppName.app> <dylib-load-path>

The dylib load path is normally "@executable_path/Frameworks/ForgeTaxCheat.dylib".
Requires LIEF (pip install lief). Exits nonzero with a clear message if the
load command cannot be added (e.g. LIEF API differences) — the workflow
then fails instead of shipping an IPA without the cheat loaded.
"""
import hashlib
import os
import sys

LOAD_PATH = "@executable_path/Frameworks/ForgeTaxCheat.dylib"


def sha256_file(path):
    h = hashlib.sha256()
    with open(path, "rb") as f:
        for chunk in iter(lambda: f.read(1 << 20), b""):
            h.update(chunk)
    return h.hexdigest()


def main():
    app_dir = sys.argv[1]
    load_path = sys.argv[2] if len(sys.argv) > 2 else LOAD_PATH
    app_name = os.path.basename(app_dir.rstrip("/"))
    exe_name = app_name[:-4] if app_name.endswith(".app") else app_name
    exe_path = os.path.join(app_dir, exe_name)
    if not os.path.isfile(exe_path):
        # Fall back: the single Mach-O executable directly inside the .app.
        cands = [os.path.join(app_dir, f) for f in os.listdir(app_dir)
                 if os.path.isfile(os.path.join(app_dir, f))]
        exe_path = None
        for c in cands:
            with open(c, "rb") as f:
                if f.read(4) in (b"\xcf\xfa\xed\xfe", b"\xca\xfe\xba\xbe"):
                    exe_path = c
                    break
        if not exe_path:
            print("FAIL: no main executable found in %s" % app_dir)
            return 1
    unity_path = os.path.join(app_dir, "Frameworks", "UnityFramework.framework",
                              "UnityFramework")
    unity_before = sha256_file(unity_path) if os.path.isfile(unity_path) else None

    try:
        import lief
    except ImportError:
        print("FAIL: LIEF is not installed (pip install lief)")
        return 1
    binary = lief.parse(exe_path)
    if binary is None:
        print("FAIL: LIEF could not parse %s" % exe_path)
        return 1
    # Skip if the load command is already present (idempotent re-runs).
    # NOTE (AEGIS fix): LIEF 1.0.0 reports the enum name as "LOAD_DYLIB",
    # not "LC_LOAD_DYLIB" — comparing only against the latter never matches
    # and a second run adds a duplicate load command. Accept both spellings
    # so the check works across LIEF versions.
    for cmd in binary.commands:
        try:
            cmd_name = cmd.command.name
        except Exception:
            continue
        if cmd_name not in ("LC_LOAD_DYLIB", "LOAD_DYLIB"):
            continue
        try:
            if load_path in str(cmd.name):
                print("LC_LOAD_DYLIB for %s already present, skipping" % load_path)
                return 0
        except Exception:
            pass
    dylib_cmd = lief.MachO.DylibCommand.load_dylib(load_path)
    binary.add(dylib_cmd)
    binary.write(exe_path)
    print("Added LC_LOAD_DYLIB %s to %s" % (load_path, exe_path))

    if unity_before is not None:
        unity_after = sha256_file(unity_path)
        if unity_after != unity_before:
            print("FAIL: UnityFramework changed during injection! %s -> %s"
                  % (unity_before, unity_after))
            return 1
        print("UnityFramework unchanged: %s" % unity_after)
    return 0


if __name__ == "__main__":
    sys.exit(main())
