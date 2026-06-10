#!/bin/bash

export IN_FILE=$(python -c "from MuonGeoModelTestR4.testGeoModel import MuonPhaseIITestDefaults; print(MuonPhaseIITestDefaults.HITS_PG_R4[0])")

python -m MuonPatternRecognitionTest.MuonHoughTransformTesterConfig \
        --inputFile ${IN_FILE} \
        --defaultGeoFile RUN4 \
        --nEvents 2 \
        --noMonitorPlots
