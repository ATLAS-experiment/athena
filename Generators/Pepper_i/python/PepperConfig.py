# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
"""Driver that invokes the LCG-installed `pepper` binary from inside an
AthGeneration job.

This module exists because the pepper_kokkos package as shipped in LCG
needs three things wired up correctly before it will run:

1. The PEPPER_DATA_PATH env var must point at the process-data
   directory (the binary's compiled-in path is the build server's
   scratch dir and doesn't exist on cvmfs). The wrapper sets it
   automatically.

2. The output filename's extension governs the format. The pepper
   binary accepts `.lhef[.gz]`, `.hepmc3[.gz]`, `.hdf5`, and `.debug`.
   AthGeneration's Gen_tf.py post-job validator globs for `*.events*`
   (MadGraph/Powheg convention). We resolve the tension by accepting
   `.events.gz` (or `.events`) as the user-facing default: pepper is
   invoked with the equivalent `.lhef[.gz]` form, then symlinked back
   to the requested `.events.gz` name.

3. The binary deadlocks in Kokkos initialisation on CPU-only nodes,
   waiting for a CUDA device. Jobs MUST run on a GPU-equipped worker.
"""

import os
import shutil
import subprocess

from AthenaCommon.Logging import logging

log = logging.getLogger("PepperConfig")


# Map of user-facing output extensions to the pepper-CLI extension
# we actually invoke pepper with. .events[.gz] is the transform's
# expected pattern; pepper itself doesn't recognise it, so we
# transparently swap to .lhef[.gz] and symlink afterwards.
_EXT_MAP = {
    ".events.gz":  ".lhef.gz",
    ".events":     ".lhef",
    # Pass-through: pepper recognises these natively.
    ".lhef.gz":    ".lhef.gz",
    ".lhef":       ".lhef",
    ".hepmc3.gz":  ".hepmc3.gz",
    ".hepmc3":     ".hepmc3",
    ".hdf5":       ".hdf5",
    ".debug":      ".debug",
}


def _resolve_pepper_executable():
    """Return the absolute path to the `pepper` binary, or raise."""
    exe = os.environ.get("PEPPER_EXECUTABLE") or shutil.which("pepper")
    if not exe or not os.path.isfile(exe):
        raise RuntimeError(
            "Could not locate the `pepper` executable. "
            "Either set PEPPER_EXECUTABLE, or ensure that an LCG view "
            "containing pepper_kokkos is on PATH "
            "(e.g. /cvmfs/sft.cern.ch/lcg/views/LCG_109a/x86_64-el9-gcc13-opt)."
        )
    return exe


def _resolve_data_path(exe):
    """Return the directory containing the Pepper process CSVs.

    Tries (in order): PEPPER_DATA_PATH env var; the share/pepper/data
    directory next to the binary (1.8.x layout); the share/pepper_data
    directory next to the binary (1.1.x layout).
    """
    candidate = os.environ.get("PEPPER_DATA_PATH")
    if candidate and os.path.isfile(os.path.join(candidate, "2j.csv")):
        return candidate

    install_root = os.path.dirname(os.path.dirname(os.path.realpath(exe)))
    for sub in ("share/pepper/data", "share/pepper_data"):
        candidate = os.path.join(install_root, sub)
        if os.path.isfile(os.path.join(candidate, "2j.csv")):
            return candidate

    raise RuntimeError(
        "Could not locate Pepper's process-data directory. "
        "Set PEPPER_DATA_PATH explicitly to the directory containing "
        "2j.csv, z0j.csv, etc."
    )


def _split_ext(filename):
    """Return (stem, longest matching extension) where the extension
    is one of the keys in _EXT_MAP. Raises if no match."""
    for ext in sorted(_EXT_MAP.keys(), key=len, reverse=True):
        if filename.endswith(ext):
            return filename[:-len(ext)], ext
    raise ValueError(
        "Pepper output filename %r must end in one of: %s"
        % (filename, ", ".join(sorted(_EXT_MAP.keys()))))


def run_pepper(process,
               nevents,
               seed,
               ecm,
               output="pepper.events.gz",
               batch_size=None,
               n_batches=None,
               extra_args=None):
    """Invoke `pepper`, produce an event file, and return its path.

    The default output is `.events.gz` to match the convention used by
    AthGeneration's `Gen_tf.py` for upstream-generator input files
    (`Pythia8_LHEF` and the transform's post-job validator both look
    for `*.events*`). Internally we invoke pepper with the
    corresponding `.lhef[.gz]` extension and create the `.events[.gz]`
    name as a symlink.

    Parameters
    ----------
    process     : Pepper process shortcut, e.g. "ppjj", "ppz1j", "pptt".
    nevents     : Approximate number of unweighted events wanted.
    seed        : Random seed (typically runArgs.randomSeed).
    ecm         : Centre-of-mass energy in GeV (typically runArgs.ecmEnergy).
    output      : Output filename. Must end in one of .events[.gz],
                  .lhef[.gz], .hepmc3[.gz], .hdf5, or .debug.
    batch_size  : Pepper batch size. Defaults to min(nevents, 5000).
    n_batches   : Number of batches. Defaults so that we usually
                  overshoot the requested nevents after accounting for
                  unweighting efficiency.
    extra_args  : List of additional CLI args to pass through.

    Returns
    -------
    The absolute path of the event file in $PWD.
    """
    exe  = _resolve_pepper_executable()
    data = _resolve_data_path(exe)

    stem, user_ext = _split_ext(output)
    pepper_ext     = _EXT_MAP[user_ext]
    pepper_output  = stem + pepper_ext

    if batch_size is None:
        batch_size = min(nevents, 5000)
    if n_batches is None:
        # Unweighting efficiency is process-dependent; pad by 2x so
        # we usually overshoot rather than undershoot.
        n_batches = max(1, (2 * nevents + batch_size - 1) // batch_size)

    cmd = [exe,
           "--process",          str(process),
           "--collision-energy", str(ecm),
           "--seed",             str(seed),
           "--batch-size",       str(batch_size),
           "--n-batches",        str(n_batches),
           "--output",           pepper_output]
    if extra_args:
        cmd += list(extra_args)

    env = os.environ.copy()
    env["PEPPER_DATA_PATH"] = data
    # Sensible OpenMP defaults so Kokkos doesn't warn at startup.
    env.setdefault("OMP_PROC_BIND", "spread")
    env.setdefault("OMP_PLACES",    "threads")

    log.info("PEPPER_DATA_PATH = %s", data)
    log.info("Pepper command:    %s", " ".join(cmd))

    rc = subprocess.call(cmd, env=env)
    if rc != 0:
        raise RuntimeError("pepper exited with rc=%d" % rc)
    if not os.path.isfile(pepper_output):
        raise RuntimeError("pepper succeeded but %s is missing" % pepper_output)

    # If user requested a name different from what pepper produces
    # (typical: user asked for .events.gz; pepper made .lhef.gz),
    # link the user-facing name to the actual file.
    if output != pepper_output:
        if os.path.lexists(output):
            os.remove(output)
        os.symlink(os.path.basename(pepper_output), output)
        log.info("Linked %s -> %s", output, pepper_output)

    return os.path.abspath(output)
