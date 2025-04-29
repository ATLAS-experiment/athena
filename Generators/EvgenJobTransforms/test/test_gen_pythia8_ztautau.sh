#!/bin/bash
# art-description: Generation test Pythia8 Z->tautau 
# art-type: build
# art-include: main/AthGeneration
# art-include: main--HepMC2/Athena
# art-include: 22.0/Athena
# art-output: *.root
# art-output: log.generate

## Any arguments are considered overrides, and will be added at the end
export TRF_ECHO=True;
Gen_tf.py --ecmEnergy=13600 --jobConfig=421100 --maxEvents=10 \
    --outputEVNTFile=test_Ztautau.EVNT.pool.root \

echo "art-result: $? generate"


