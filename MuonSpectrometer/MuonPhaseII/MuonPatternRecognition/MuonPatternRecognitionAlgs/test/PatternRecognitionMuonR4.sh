#!/bin/bash
NTHREADS=${1}
NEVENTS=${2}
OPT=${3}


python -m MuonPatternRecognitionTest.MuonHoughTransformTesterConfig ${OPT} \
       --noSTGC \
       --noMM \
       --nEvents ${NEVENTS} \
       --threads ${NTHREADS}
