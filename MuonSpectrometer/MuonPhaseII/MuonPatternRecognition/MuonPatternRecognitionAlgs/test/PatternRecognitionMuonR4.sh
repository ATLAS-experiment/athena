#!/bin/bash
NTHREADS=${1}
NEVENTS=${2}


python -m MuonPatternRecognitionAlgs.MuonHoughTransformAlgConfig  \
       --nEvents ${NEVENTS} \
       --threads ${NTHREADS}
