#!/bin/bash
# art-description: PhP8EG bb->4l 
# art-include: main/AthGeneration
# art-include: main--HepMC2/Athena
# art-include: main--dev3LCG/Athena
# art-include: main--dev4LCG/Athena
# art-type: build
# art-output: *.root
# art-output: log.generate

## Any arguments are considered overrides, and will be added at the end
export TRF_ECHO=True;
Gen_tf.py --ecmEnergy=13600 --jobConfig=602495 --maxEvents=10 \
    --outputEVNTFile=test_php8eg_bb4l.EVNT.pool.root \

echo "art-result: $? generate"

