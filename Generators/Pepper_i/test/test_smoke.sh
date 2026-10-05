#!/bin/bash
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# test/test_smoke.sh — minimal end-to-end smoke tests for Pepper_i.
#
#   shower : Pepper + Pythia8 + EvtGen in one job  ->  EVNT
#   lhe    : Pepper only                           ->  TXT tarball
#
# REQUIREMENTS:
#   * lxplus-gpu (or any worker with a CUDA device); CPU-only nodes
#     will deadlock during Kokkos init.
#   * AthGeneration release that ships Pepper_i (or a sparse checkout
#     where Pepper_i is on JOBOPTSEARCHPATH).
#   * ATLAS environment via setupATLAS.
#
# USAGE:
#   bash test_smoke.sh [shower|lhe|all]      (default: all)

set -euo pipefail

MODE=${1:-all}
NEVENTS=10
WORKDIR=${WORKDIR:-/tmp/${USER}-pepper-smoke}
mkdir -p "$WORKDIR"

fail() { echo "FAIL: $*"; exit 1; }

# ---- Pepper + Pythia8 + EvtGen -> EVNT -------------------------------------
run_shower() {
    local run="$WORKDIR/shower" jo="$WORKDIR/shower/999100"
    mkdir -p "$jo" && cd "$run"
    cat > "$jo/mc.PepperPy8EG_A14NNPDF23LO_dijet.py" <<'JOEOF'
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
JOEOF

    Gen_tf.py \
        --ecmEnergy=13600. \
        --maxEvents=$NEVENTS \
        --randomSeed=1234 \
        --jobConfig="$jo" \
        --outputEVNTFile=pepper_smoke.EVNT.root \
        2>&1 | tee gen.log

    [ -f pepper_smoke.EVNT.root ] || fail "pepper_smoke.EVNT.root was not produced"
    grep -q "trf exit code 0" gen.log || fail "transform did not exit cleanly"
    grep -q "Event counting test passed ($NEVENTS events)" gen.log \
        || fail "event count check did not pass for $NEVENTS events"
    echo "PASS: Pepper_i shower smoke test"
}

# ---- Pepper only -> TXT tarball --------------------------------------------
run_lhe() {
    local run="$WORKDIR/lhe" jo="$WORKDIR/lhe/999101"
    mkdir -p "$jo" && cd "$run"
    cat > "$jo/mc.Pepper_dijet_LHE.py" <<'JOEOF'
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
evgenConfig.description    = "Pepper_i smoke test: ppjj, LHE only"
evgenConfig.keywords       = ["SM", "QCD", "jets"]
evgenConfig.contact        = ["atlas-generator-software@cern.ch"]
evgenConfig.generators     = ["Pepper"]
evgenConfig.nEventsPerJob  = 10

PEPPER_PROCESS = "ppjj"

include("Pepper_i/Pepper_Common.py")
JOEOF

    Gen_tf.py \
        --ecmEnergy=13600. \
        --maxEvents=$NEVENTS \
        --randomSeed=1234 \
        --jobConfig="$jo" \
        --outputTXTFile=pepper_smoke.TXT.tar.gz \
        2>&1 | tee gen.log

    [ -f pepper_smoke.TXT.tar.gz ] || fail "pepper_smoke.TXT.tar.gz was not produced"
    grep -q "trf exit code 0" gen.log || fail "transform did not exit cleanly"
    tar tzf pepper_smoke.TXT.tar.gz | grep -qx "pepper_smoke.TXT.events" \
        || fail "tarball does not hold pepper_smoke.TXT.events"
    local n
    n=$(tar xzOf pepper_smoke.TXT.tar.gz | grep -c '^ *<event[ >]' || true)
    [ "$n" -ge "$NEVENTS" ] || fail "only $n events in the tarball, expected >= $NEVENTS"
    grep -q "Number of produced LHE events" log.generate \
        || fail "skeleton did not report the number of LHE events"
    echo "PASS: Pepper_i LHE-only smoke test ($n events)"
}

case "$MODE" in
    shower) run_shower ;;
    lhe)    run_lhe ;;
    all)    run_shower; run_lhe ;;
    *)      echo "usage: $0 [shower|lhe|all]"; exit 2 ;;
esac
