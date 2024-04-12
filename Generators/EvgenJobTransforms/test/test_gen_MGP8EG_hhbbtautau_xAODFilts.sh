#!/bin/bash
# art-description: MGP8EG hh->bb->tautau xParCh xXtoVV xMulEMuTau 
# art-include: main/AthGeneration
# art-include: main--HepMC2/Athena
# art-include: main--dev3LCG/Athena
# art-include: main--dev4LCG/Athena
# art-type: build
# art-output: *.root
# art-output: log.generate

## Any arguments are considered overrides, and will be added at the end
export TRF_ECHO=True;
Gen_tf.py --ecmEnergy=13600 --jobConfig=421495 --maxEvents=10 \
    --outputEVNTFile=test_mgp8eg_xAODFilt.EVNT.pool.root \

echo "art-result: $? generate"

