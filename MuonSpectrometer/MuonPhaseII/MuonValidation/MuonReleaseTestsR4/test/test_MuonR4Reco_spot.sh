#!/usr/bin/bash
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

NTHREADS=${1}
NEVENTS=${2}

inputFile=$(python -c "from AthenaConfiguration.TestDefaults import defaultTestFiles; print(defaultTestFiles.RAW_RUN3_DATA24[0])")

# Run the job
python -m MuonPatternRecognitionTest.MuonHoughTransformTesterConfig \
    --nEvents ${NEVENTS} \
    --threads ${NTHREADS} \
    --noMonitorPlots \
    --inputFile ${inputFile} &> out.log 

