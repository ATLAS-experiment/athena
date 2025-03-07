#!/bin/bash
# art-description: Generation test Powheg LHE-only single top s-channel
# art-type: build
# art-include: main/AthGeneration
# art-include: main--HepMC2/Athena
# art-include: 22.0/Athena
# art-output: *.root
# art-output: log.generate

## Any arguments are considered overrides, and will be added at the end
export TRF_ECHO=True;
rm *;
Gen_tf.py --ecmEnergy=13600 --jobConfig=601348 --maxEvents=10 \
    --outputEVNTFile=test_powheg_t.root \

echo "art-result: $? generate"


