#!/bin/bash
# art-description: Generation test Sherpa3 W->mu nu
# art-type: build
# art-include: main/AthGeneration
# art-include: 22.0/Athena
# art-output: *.root
# art-output: log.generate

## Any arguments are considered overrides, and will be added at the end
export TRF_ECHO=True;
Gen_tf.py --ecmEnergy=13600 --jobConfig=950748 --maxEvents=10 \
    --outputEVNTFile=test_sherpa_wmunu.EVNT.pool.root \

echo "art-result: $? generate"



