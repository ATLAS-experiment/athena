# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# Generators/Pepper_i/share/common/Pepper_Common.py
#
# JO fragment that runs Pepper and produces a parton-level event file.
# Include this from a DSID JO file after setting at minimum:
#
#     PEPPER_PROCESS = "ppjj"   # any built-in Pepper process shortcut
#
# Optional overrides:
#
#     PEPPER_NEVENTS    - default: evgenConfig.nEventsPerJob
#     PEPPER_SEED       - default: runArgs.randomSeed
#     PEPPER_ECM        - default: runArgs.ecmEnergy
#     PEPPER_OUTPUT     - default: "pepper.events.gz"
#     PEPPER_BATCH_SIZE - default: min(nevents, 5000)
#     PEPPER_N_BATCHES  - default: 2 * nevents / batch_size
#     PEPPER_EXTRA_ARGS - list of additional CLI args, default: []
#
# After this fragment runs, an event file is on disk at the path
# `PEPPER_OUTPUT`. To shower it with Pythia8, include the standard
# Pythia8 fragments after this one in the DSID JO and point Pythia8
# at the file. For example:
#
#     PEPPER_PROCESS = "ppjj"
#     include("Pepper_i/Pepper_Common.py")
#     include("Pythia8_i/Pythia8_A14_NNPDF23LO_EvtGen_Common.py")
#     include("Pythia8_i/Pythia8_LHEF.py")
#     genSeq.Pythia8.Commands += [ "Beams:LHEF = " + PEPPER_OUTPUT ]
#
# This fragment requires the Pepper_i package and a GPU-equipped
# worker node. See Generators/Pepper_i/doc/README.md.

from Pepper_i.PepperConfig import run_pepper

_process    = globals().get("PEPPER_PROCESS")
_nevents    = globals().get("PEPPER_NEVENTS",    evgenConfig.nEventsPerJob)
_seed       = globals().get("PEPPER_SEED",       runArgs.randomSeed)
_ecm        = globals().get("PEPPER_ECM",        runArgs.ecmEnergy)
_output     = globals().get("PEPPER_OUTPUT",     "pepper.events.gz")
_batch_size = globals().get("PEPPER_BATCH_SIZE", None)
_n_batches  = globals().get("PEPPER_N_BATCHES",  None)
_extra      = globals().get("PEPPER_EXTRA_ARGS", None)

if _process is None:
    raise RuntimeError(
        "Pepper_Common.py: please set PEPPER_PROCESS in the JO "
        "(e.g. PEPPER_PROCESS = 'ppjj').")

# Run Pepper. PEPPER_OUTPUT is exported back into the JO scope so
# downstream fragments can pick it up.
PEPPER_OUTPUT = run_pepper(
    process     = _process,
    nevents     = _nevents,
    seed        = _seed,
    ecm         = _ecm,
    output      = _output,
    batch_size  = _batch_size,
    n_batches   = _n_batches,
    extra_args  = _extra,
)
