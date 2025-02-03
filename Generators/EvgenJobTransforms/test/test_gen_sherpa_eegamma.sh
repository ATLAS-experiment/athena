#!/bin/bash
# art-description: Generation test Sherpa3 eegamma
# art-type: build
# art-include: main/AthGeneration
# art-include: 22.0/Athena
# art-output: *.root
# art-output: log.generate

## Any arguments are considered overrides, and will be added at the end
export TRF_ECHO=True;
Gen_tf.py --ecmEnergy=13600 --jobConfig=950750 --maxEvents=10 \
    --outputEVNTFile=test_sherpa_eegamma.EVNT.pool.root \

echo "art-result: $? generate"



