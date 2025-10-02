#!/bin/bash
NTHREADS=${1}
NEVENTS=${2}


python -m MuonPatternRecognitionTest.MuonHoughTransformTesterConfig  \
       --noSTGC \
       --noMM \
       --nEvents ${NEVENTS} \
       --threads ${NTHREADS}
