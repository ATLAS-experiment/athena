#!/usr/bin/bash
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

NTHREADS=${1}
NEVENTS=${2}
OPT=${3}

inputFile=$(python -c "from MuonGeoModelTestR4.testGeoModel import MuonPhaseIITestDefaults; print(' '.join(MuonPhaseIITestDefaults.DATA_BS))")

# Run the job
export TRF_ECHO=1;
python -m MuonPatternRecognitionTest.MuonRecoChainTesterConfig ${OPT} \
    --noSTGC \
    --nEvents ${NEVENTS} \
    --threads ${NTHREADS} \
    --inputFile ${inputFile} > log.MuonR4Reco 2>&1;

ecode=$?
echo ${ecode} > __exitcode;
echo "leaving with code ${ecode}: successful run" >> log.MuonR4Reco;
