#!/usr/bin/bash
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

NTHREADS=${1}
NEVENTS=${2}

inputFile=$(python -c "from AthenaConfiguration.TestDefaults import defaultTestFiles; print(defaultTestFiles.RAW_RUN3_DATA24[0])")

# Run the job
export TRF_ECHO=1;
python -m MuonPatternRecognitionTest.MuonRecoChainTesterConfig \
    --nEvents ${NEVENTS} \
    --threads ${NTHREADS} \
    --inputFile ${inputFile} > log.MuonR4Reco 2>&1;

ecode=$?
echo ${ecode} > __exitcode;
echo "leaving with code ${ecode}: successful run" >> log.MuonR4Reco;
