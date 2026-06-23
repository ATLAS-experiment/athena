#!/bin/bash
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# test/test_smoke.sh — minimal end-to-end smoke test for Pepper_i.
#
# Mirrors the working invocation that produced lcg_pepper.EVNT.root.
#
# REQUIREMENTS:
#   * lxplus-gpu (or any worker with a CUDA device); CPU-only nodes
#     will deadlock during Kokkos init.
#   * AthGeneration release that ships Pepper_i (or a sparse checkout
#     where Pepper_i is on JOBOPTSEARCHPATH).
#   * ATLAS environment via setupATLAS.
#
# USAGE:
#   bash test_smoke.sh

set -euo pipefail

WORKDIR=${WORKDIR:-/tmp/${USER}-pepper-smoke}
mkdir -p "$WORKDIR" && cd "$WORKDIR"

# ---- DSID JO ---------------------------------------------------------------
JO="$WORKDIR/999100"
mkdir -p "$JO"
cat > "$JO/mc.PepperPy8EG_A14NNPDF23LO_dijet.py" <<'JOEOF'
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
evgenConfig.description    = "Pepper_i smoke test: ppjj + Pythia8 A14 + EvtGen"
evgenConfig.keywords       = ["SM", "QCD", "jets"]
evgenConfig.contact        = ["atlas-generator-software@cern.ch"]
evgenConfig.generators     = ["Pepper", "Pythia8", "EvtGen"]
evgenConfig.nEventsPerJob  = 10

PEPPER_PROCESS = "ppjj"

include("Pepper_i/Pepper_Common.py")
include("Pythia8_i/Pythia8_A14_NNPDF23LO_EvtGen_Common.py")
include("Pythia8_i/Pythia8_LHEF.py")

genSeq.Pythia8.Commands += [ "Beams:LHEF = " + PEPPER_OUTPUT ]
JOEOF

# ---- Run -------------------------------------------------------------------
Gen_tf.py \
    --ecmEnergy=13600. \
    --maxEvents=10 \
    --randomSeed=1234 \
    --jobConfig="$JO" \
    --outputEVNTFile=pepper_smoke.EVNT.root \
    2>&1 | tee gen.log

# ---- Validate --------------------------------------------------------------
if [ ! -f pepper_smoke.EVNT.root ]; then
    echo "FAIL: pepper_smoke.EVNT.root was not produced"
    exit 1
fi

if ! grep -q "trf exit code 0" gen.log; then
    echo "FAIL: transform did not exit cleanly"
    exit 1
fi

if ! grep -q "Event counting test passed (10 events)" gen.log; then
    echo "FAIL: event count check did not pass for 10 events"
    exit 1
fi

echo "PASS: Pepper_i smoke test"
