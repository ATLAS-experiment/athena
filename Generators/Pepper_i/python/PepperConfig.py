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
   For convenience `.events[.gz]` is accepted too: pepper is invoked
   with the equivalent `.lhef[.gz]` form, then symlinked back to the
   requested name.

3. The binary deadlocks in Kokkos initialisation on CPU-only nodes,
   waiting for a CUDA device. Jobs MUST run on a GPU-equipped worker.

4. Pepper's `--batch-size`/`--n-batches` options count *trial* phase-space
   points, of which only the fraction surviving unweighting is written
   out (about 15% for ppjj at 13.6 TeV). The wrapper therefore steers
   the run with `--n-events`, which is the minimum number of non-zero
   (i.e. written) events, and checks the yield afterwards.

5. "Pepper" is an LHE generator for the Gen_tf.py skeletons
   (GeneratorConfig.GenConfigHelpers.lhefGenerators). They therefore
   expect runArgs.inputGeneratorFile to name an uncompressed
   `<name>.events` file, which they link to `events.lhe` for the
   shower and count for the job metadata. arrange_output() provides
   that, in the same way as MadGraphControl.arrange_output().
"""

import gzip
import os
import re
import shutil
import subprocess
import tarfile

from AthenaCommon.Logging import logging

log = logging.getLogger("PepperConfig")


# Map of user-facing output extensions to the pepper-CLI extension
# we actually invoke pepper with. pepper itself doesn't recognise
# .events[.gz], so we transparently swap to .lhef[.gz] and symlink
# afterwards.
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

# Pepper-CLI extensions that are LHE files, i.e. whose events we can
# count and hand to the transform.
_LHE_EXTS = (".lhef.gz", ".lhef")

# Name under which the events are handed to the transform when the job
# has no TXT output (cf. 'tmp_LHE_events' in MadGraphControl).
_TMP_LHE_NAME = "pepper_LHE_events"

# Number of phase-space points evaluated per batch. This only tunes
# throughput (and the granularity of the overshoot, since pepper stops
# at the first batch boundary after reaching --n-events); it does not
# determine how many events are produced.
DEFAULT_BATCH_SIZE = 1000

_EVENT_TAG = re.compile(r"^\s*<event[\s>]")


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


def _open_events(path, mode="rt"):
    """Open a (possibly gzipped) event file."""
    if path.endswith(".gz"):
        return gzip.open(path, mode)
    return open(path, mode)


def count_lhe_events(path):
    """Return the number of <event> blocks in a (possibly gzipped) LHE file."""
    n = 0
    with _open_events(path) as f:
        for line in f:
            if _EVENT_TAG.match(line):
                n += 1
    return n


def run_pepper(process,
               nevents,
               seed,
               ecm,
               output="pepper.lhef",
               batch_size=None,
               extra_args=None):
    """Invoke `pepper`, produce an event file, and return its path.

    The default output is an uncompressed LHE file. Use
    arrange_output() afterwards to hand it to the Gen_tf.py skeleton.

    Parameters
    ----------
    process     : Pepper process shortcut, e.g. "ppjj", "ppz1j", "pptt".
    nevents     : Minimum number of events to be written (passed to
                  pepper as --n-events). Pepper stops at the first batch
                  boundary after reaching it, so the file usually holds
                  slightly more.
    seed        : Random seed (typically runArgs.randomSeed).
    ecm         : Centre-of-mass energy in GeV (typically runArgs.ecmEnergy).
    output      : Output filename. Must end in one of .events[.gz],
                  .lhef[.gz], .hepmc3[.gz], .hdf5, or .debug.
    batch_size  : Phase-space points per batch. Throughput tuning only;
                  defaults to DEFAULT_BATCH_SIZE.
    extra_args  : List of additional CLI args to pass through.

    Returns
    -------
    The absolute path of the event file in $PWD.
    """
    nevents = int(nevents)
    if nevents < 1:
        raise ValueError("run_pepper: nevents must be >= 1, got %d" % nevents)

    exe  = _resolve_pepper_executable()
    data = _resolve_data_path(exe)

    stem, user_ext = _split_ext(output)
    pepper_ext     = _EXT_MAP[user_ext]
    pepper_output  = stem + pepper_ext

    if batch_size is None:
        batch_size = DEFAULT_BATCH_SIZE

    cmd = [exe,
           "--process",          str(process),
           "--collision-energy", str(ecm),
           "--seed",             str(seed),
           "--batch-size",       str(batch_size),
           "--n-events",         str(nevents),
           "--output",           pepper_output]
    if extra_args:
        cmd += list(extra_args)

    env = os.environ.copy()
    env["PEPPER_DATA_PATH"] = data
    # Sensible OpenMP defaults so Kokkos doesn't warn at startup.
    env.setdefault("OMP_PROC_BIND", "spread")
    env.setdefault("OMP_PLACES",    "threads")

    # Make sure the yield check below can only ever see this run's events.
    if os.path.lexists(pepper_output):
        os.remove(pepper_output)

    log.info("PEPPER_DATA_PATH = %s", data)
    log.info("Pepper command:    %s", " ".join(cmd))

    rc = subprocess.call(cmd, env=env)
    if rc != 0:
        raise RuntimeError("pepper exited with rc=%d" % rc)
    if not os.path.isfile(pepper_output):
        raise RuntimeError("pepper succeeded but %s is missing" % pepper_output)

    if pepper_ext in _LHE_EXTS:
        nwritten = count_lhe_events(pepper_output)
        log.info("Pepper wrote %d events to %s (%d requested)",
                 nwritten, pepper_output, nevents)
        if nwritten < nevents:
            raise RuntimeError(
                "pepper wrote only %d events to %s, but %d were requested"
                % (nwritten, pepper_output, nevents))
    else:
        log.info("Output format %s: number of written events not checked",
                 pepper_ext)

    # If user requested a name different from what pepper produces
    # (typical: user asked for .events.gz; pepper made .lhef.gz),
    # link the user-facing name to the actual file.
    if output != pepper_output:
        if os.path.lexists(output):
            os.remove(output)
        os.symlink(os.path.basename(pepper_output), output)
        log.info("Linked %s -> %s", output, pepper_output)

    return os.path.abspath(output)


def arrange_output(events_file, runArgs):
    """Hand an LHE file from run_pepper() over to the Gen_tf.py skeleton.

    Mirrors MadGraphControl.arrange_output():

    * the events are made available uncompressed as `<name>.events`;
    * if the job has a TXT output (--outputTXTFile=<name>.tar.gz), that
      file is packed into the tarball;
    * runArgs.inputGeneratorFile is set so that the skeleton finds
      `<name>.events`, links it to `events.lhe` (which Pythia8_LHEF.py
      reads) and counts its events for the job metadata.

    Non-LHE outputs (.hepmc3, .hdf5, .debug) are left alone.

    Parameters
    ----------
    events_file : LHE file from run_pepper() (.lhef[.gz] or .events[.gz]).
    runArgs     : The transform's runArgs.

    Returns
    -------
    The value given to runArgs.inputGeneratorFile, or None if nothing
    was arranged.
    """
    _, ext = _split_ext(events_file)
    if _EXT_MAP[ext] not in _LHE_EXTS:
        if getattr(runArgs, "outputTXTFile", None):
            raise RuntimeError(
                "A TXT output was requested but %s is not an LHE file; "
                "use an .lhef[.gz] or .events[.gz] output" % events_file)
        log.info("%s is not an LHE file: not handed to the transform",
                 events_file)
        return None

    output_txt = getattr(runArgs, "outputTXTFile", None)
    output_ds  = output_txt if output_txt else _TMP_LHE_NAME
    lhe        = output_ds.split(".tar.gz")[0] + ".events"

    # Uncompressed <name>.events: a link if pepper's file is already
    # plain text, an unpacked copy otherwise.
    if os.path.realpath(lhe) != os.path.realpath(events_file):
        if os.path.lexists(lhe):
            os.remove(lhe)
        if events_file.endswith(".gz"):
            with gzip.open(events_file, "rb") as src, open(lhe, "wb") as dst:
                shutil.copyfileobj(src, dst)
        else:
            os.symlink(os.path.realpath(events_file), lhe)

    if output_txt:
        with tarfile.open(output_txt, "w:gz", dereference=True) as tar:
            tar.add(lhe, arcname=os.path.basename(lhe))
        log.info("Packed %s into %s", lhe, output_txt)
        # As in MadGraphControl: the skeleton derives the name of the
        # .events file from this.
        input_file = output_txt.split(".TXT")[0]
    else:
        input_file = output_ds

    log.info("Setting runArgs.inputGeneratorFile to %s (events in %s)",
             input_file, lhe)
    runArgs.inputGeneratorFile = input_file
    return input_file
