#!/bin/bash
NTHREADS=${1}
NEVENTS=${2}


python -m MuonPatternRecognitionAlgs.MuonHoughTransformAlgConfig  \
       --noSTGC \
       --noMM \
       --nEvents ${NEVENTS} \
       --threads ${NTHREADS}
