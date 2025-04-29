#!/bin/bash
# art-description: Generation test Sherpa Z0/gamma->ee 
# art-type: build
# art-include: main/AthGeneration
# art-include: 22.0/Athena
# art-output: *.root
# art-output: log.generate

## Any arguments are considered overrides, and will be added at the end
## originally 421003 was used, which should still be OK
export TRF_ECHO=True;
Gen_tf.py --ecmEnergy=13600 --jobConfig=950749 --maxEvents=10 \
    --outputEVNTFile=test_sherpa_z0.EVNT.pool.root \

echo "art-result: $? generate"



