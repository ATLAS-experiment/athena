# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# Generators/Pepper_i/share/common/Pepper_Common.py
#
# JO fragment that runs Pepper and hands the parton-level events to
# the transform. Include this from a DSID JO file after setting at
# minimum:
#
#     PEPPER_PROCESS = "ppjj"   # any built-in Pepper process shortcut
#
# Optional overrides:
#
#     PEPPER_NEVENTS       - minimum number of events to be written.
#                            default: PEPPER_SAFETY_FACTOR times the
#                            number of events the job has to deliver
#                            (runArgs.maxEvents if given, otherwise
#                            evgenConfig.nEventsPerJob), rounded up
#     PEPPER_SAFETY_FACTOR - default: 1.1 when the events are showered
#                            in this job, so that the shower does not
#                            run out of them; 1.0 for LHE-only jobs
#     PEPPER_SEED          - default: runArgs.randomSeed
#     PEPPER_ECM           - default: runArgs.ecmEnergy
#     PEPPER_OUTPUT        - default: "pepper.lhef"
#     PEPPER_BATCH_SIZE    - phase-space points per batch (throughput
#                            tuning only), default: 1000
#     PEPPER_EXTRA_ARGS    - list of additional CLI args, default: []
#
# After this fragment has run, the events are available to the
# transform in the same way as for MadGraphControl: the skeleton links
# them to `events.lhe`, and they are packed into the TXT output if the
# job was given --outputTXTFile. `PEPPER_OUTPUT` holds the path of the
# file Pepper wrote.
#
# Pepper + Pythia8 in one job (run with --outputEVNTFile):
#
#     evgenConfig.generators = ["Pepper", "Pythia8", "EvtGen"]
#     PEPPER_PROCESS = "ppjj"
#     include("Pepper_i/Pepper_Common.py")
#     include("Pythia8_i/Pythia8_A14_NNPDF23LO_EvtGen_Common.py")
#     include("Pythia8_i/Pythia8_LHEF.py")
#
# LHE only (run with --outputTXTFile and no --outputEVNTFile):
#
#     evgenConfig.generators = ["Pepper"]
#     PEPPER_PROCESS = "ppjj"
#     include("Pepper_i/Pepper_Common.py")
#
# This fragment requires the Pepper_i package and a GPU-equipped
# worker node. See Generators/Pepper_i/doc/README.md.

import math

from Pepper_i.PepperConfig import run_pepper, arrange_output

if "PEPPER_N_BATCHES" in globals():
    raise RuntimeError(
        "Pepper_Common.py: PEPPER_N_BATCHES is no longer supported. It "
        "counted trial phase-space points rather than written events; "
        "set PEPPER_NEVENTS (minimum number of written events) instead.")

# LHE-only job: a TXT output and no EVNT output.
_lhe_only = (hasattr(runArgs, "outputTXTFile")
             and not hasattr(runArgs, "outputEVNTFile")
             and not hasattr(runArgs, "outputEVNT_PreFile"))

_process    = globals().get("PEPPER_PROCESS")
_safety     = globals().get("PEPPER_SAFETY_FACTOR", 1.0 if _lhe_only else 1.1)
_seed       = globals().get("PEPPER_SEED",       runArgs.randomSeed)
_ecm        = globals().get("PEPPER_ECM",        runArgs.ecmEnergy)
_output     = globals().get("PEPPER_OUTPUT",     "pepper.lhef")
_batch_size = globals().get("PEPPER_BATCH_SIZE", None)
_extra      = globals().get("PEPPER_EXTRA_ARGS", None)

if _process is None:
    raise RuntimeError(
        "Pepper_Common.py: please set PEPPER_PROCESS in the JO "
        "(e.g. PEPPER_PROCESS = 'ppjj').")

# Number of events this job has to deliver: --maxEvents if given on
# the command line, otherwise evgenConfig.nEventsPerJob.
_target = getattr(runArgs, "maxEvents", -1)
if not _target or _target < 1:
    _target = evgenConfig.nEventsPerJob

# Small tolerance so that e.g. 1.1 * 10000 does not round up to 11001.
_nevents = globals().get("PEPPER_NEVENTS",
                         int(math.ceil(_target * _safety - 1e-9)))

# Run Pepper. PEPPER_OUTPUT is exported back into the JO scope.
PEPPER_OUTPUT = run_pepper(
    process     = _process,
    nevents     = _nevents,
    seed        = _seed,
    ecm         = _ecm,
    output      = _output,
    batch_size  = _batch_size,
    extra_args  = _extra,
)

# Hand the events to the transform (events.lhe link, TXT output).
arrange_output(PEPPER_OUTPUT, runArgs)
