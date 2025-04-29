#!/bin/bash
# art-description: Generation test MG+Py8 tW with MadSTR - AGENE-2244 
# art-include: main/AthGeneration
# art-include: main--dev4LCG/Athena
# art-include: 22.0/Athena
# art-type: build
# art-output: *.root
# art-output: log.generate

## Any arguments are considered overrides, and will be added at the end
export TRF_ECHO=True;
rm *;
Gen_tf.py --ecmEnergy=13600 --jobConfig=421498 --maxEvents=10 \
    --outputEVNTFile=test_mgpythia8_tW_MADSTR.EVNT.pool.root \

echo "art-result: $? generate"


