#!/bin/bash

export IN_FILE=$(python -c "from MuonGeoModelTestR4.testGeoModel import MuonPhaseIITestDefaults; print(MuonPhaseIITestDefaults.HITS_PG_R3[0])")

python -m MuonPatternRecognitionTest.MuonHoughTransformTesterConfig \
        --inputFile ${IN_FILE} \
        --defaultGeoFile RUN3 \
        --nEvents 2 \
        --noMdt \
        --noRpc \
        --noTgc \
        --noMonitorPlots
