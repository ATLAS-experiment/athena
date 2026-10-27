# Copyright (C) 2026 CERN for the benefit of the ATLAS collaboration
"""Activate the temporary shim before importing Athena or loading Coin."""

import os
from pathlib import Path
import sys


def enableSnapshotWorkaround():
    """Re-exec the launcher once, consuming the opt-in flag."""
    arguments = sys.argv[1:]
    # Respect the standard end-of-options delimiter.
    end = arguments.index("--") if "--" in arguments else len(arguments)
    if "--fix-snapshots" not in arguments[:end]:
        return
    if "--help" in arguments[:end] or "-h" in arguments[:end]:
        return
    arguments = [arg for i, arg in enumerate(arguments)
                 if i >= end or arg != "--fix-snapshots"]
    launcher = Path(os.path.abspath(sys.argv[0]))
    candidates = [
        launcher.parent.parent / "share/VP1Algs/offscreen-workaround",
        launcher.resolve().parent / "offscreen-workaround",
    ]
    candidates.extend(Path(entry) / "VP1Algs/offscreen-workaround"
                      for entry in os.environ.get("DATAPATH", "").split(os.pathsep)
                      if entry)
    for directory in candidates:
        script = directory / "vp1-with-glx-workaround"
        if script.is_file():
            os.execv("/bin/bash", ["bash", str(script), "--vp1-exec",
                                  sys.executable, str(launcher), *arguments])
    sys.exit("vp1: --fix-snapshots support files are missing; rebuild/install VP1Algs.")
