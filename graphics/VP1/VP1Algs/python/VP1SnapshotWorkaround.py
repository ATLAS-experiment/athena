# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
"""Choose remote mode and preload snapshot compatibility before loading Athena."""

import argparse
import ctypes
import os
from pathlib import Path
import platform
import re
import subprocess
import sys

from VP1Algs.VP1RemoteMode import addRemoteArguments, configureRemoteMode, remoteBootstrapKey


def _status(state, message):
    os.environ["VP1_SNAPSHOT_WORKAROUND_STATUS"] = state
    os.environ["VP1_SNAPSHOT_WORKAROUND_MESSAGE"] = message


def _supportScript(launcher):
    candidates = [launcher.parent.parent / "share/VP1Algs/offscreen-workaround",
                  launcher.resolve().parent / "offscreen-workaround"]
    candidates.extend(Path(entry) / "VP1Algs/offscreen-workaround"
                      for entry in os.environ.get("DATAPATH", "").split(os.pathsep) if entry)
    return next((directory / "vp1-with-glx-workaround" for directory in candidates
                 if (directory / "vp1-with-glx-workaround").is_file()), None)


def _activeLibrary():
    # The preload must actually be present, not merely an inherited status flag.
    library = os.environ.get("VP1_GLX_WORKAROUND_LIBRARY", "")
    preloads = re.split(r"[:\s]+", os.environ.get("LD_PRELOAD", ""))
    if not library or library not in preloads:
        return False
    try:
        ctypes.CDLL(library, mode=os.RTLD_NOLOAD)
        return True
    except (OSError, AttributeError):
        return False


def enableSnapshotWorkaround():
    """Bootstrap remote settings and restart at most once with the GLX shim."""
    arguments = sys.argv[1:]
    end = arguments.index("--") if "--" in arguments else len(arguments)
    if "--help" in arguments[:end] or "-h" in arguments[:end]:
        return  # Help never probes the platform, compiles or restarts.
    parser = argparse.ArgumentParser(prog="vp1", add_help=False, allow_abbrev=False)
    addRemoteArguments(parser)
    options, _ = parser.parse_known_args(arguments)
    remote = configureRemoteMode(no_remote=options.no_remote, force_remote=options.remote,
                                 announce=False, reuse_bootstrap=True)
    os.environ["VP1_REMOTE_BOOTSTRAP"] = remoteBootstrapKey(options.no_remote, options.remote)
    if _activeLibrary():
        message = "Active: GLX pixel-limit correction and failed indirect-context retry."
        if options.no_fix_snapshots:
            message += " Already preloaded externally; --no-fix-snapshots only prevents automatic activation."
        _status("active", message)
        print("[VP1 snapshots] " + message, flush=True)
        return  # Also the restart guard: the original arguments can be preserved.
    if options.no_fix_snapshots:
        _status("disabled", "Disabled by --no-fix-snapshots.")
        return
    if not options.fix_snapshots and not remote:
        _status("not-requested", "Not requested in normal mode (use --fix-snapshots if needed).")
        return

    def unavailable(reason):
        _status("unavailable", "Unavailable: " + reason)
        message = "vp1: snapshot compatibility unavailable: " + reason
        if options.fix_snapshots:
            sys.exit(message)  # Preserve the explicit opt-in's failure behavior.
        print(message + " VP1 will continue with remote rotation settings. "
              "Use --no-fix-snapshots to skip automatic setup.", file=sys.stderr, flush=True)

    if os.environ.get("VP1_GLX_WORKAROUND_ATTEMPT_PID") == str(os.getpid()):
        unavailable("the dynamic loader did not load the shim. Check the terminal and use "
                    "VP1_GLX_WORKAROUND_CACHE_DIR on a filesystem permitting shared libraries.")
        return
    if (platform.system() != "Linux" or platform.machine() not in ("aarch64", "x86_64")
            or platform.libc_ver()[0] != "glibc"):
        unavailable("requires Linux glibc on aarch64 or x86_64.")
        return
    launcher = Path(os.path.abspath(sys.argv[0]))
    script = _supportScript(launcher)
    if script is None:
        unavailable("support files are missing; rebuild/install VP1Algs.")
        return
    try:
        preparation = subprocess.run(["/bin/bash", str(script), "--prepare-only"],
                                     capture_output=True, text=True, timeout=30)
        if preparation.stderr:
            print(preparation.stderr, file=sys.stderr, end="", flush=True)
        if preparation.returncode:
            detail = preparation.stderr.strip().splitlines()
            unavailable(detail[-1] if detail else "GLX shim preparation failed.")
            return
        library = preparation.stdout.strip()
        if not library or not Path(library).is_file():
            unavailable("GLX shim preparation did not produce a library.")
            return
        environment = os.environ.copy()
        environment["VP1_GLX_WORKAROUND_LIBRARY"] = library
        environment["VP1_GLX_WORKAROUND_ATTEMPT_PID"] = str(os.getpid())
        environment["LD_PRELOAD"] = library + (":" + environment["LD_PRELOAD"]
                                                  if environment.get("LD_PRELOAD") else "")
        print("[VP1 snapshots] Restarting VP1 with GLX snapshot compatibility enabled.", flush=True)
        os.execve(sys.executable, [sys.executable, str(launcher), *arguments], environment)
    except (OSError, subprocess.TimeoutExpired) as error:
        unavailable(str(error))
