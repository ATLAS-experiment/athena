#!/bin/bash
# art-description: Generation test MG+Py8 TT xAODMET or xAODLepton Filter
# art-include: main/AthGeneration
# art-include: main--HepMC2/Athena
# art-include: main--dev3LCG/Athena
# art-include: main--dev4LCG/Athena
# art-type: build
# art-output: *.root
# art-output: log.generate

## Any arguments are considered overrides, and will be added at the end
export TRF_ECHO=True;
rm *;
Gen_tf.py --ecmEnergy=13600 --jobConfig=421345 --maxEvents=10 \
    --outputEVNTFile=test_mgpythia8_TT_xFilters.EVNT.pool.root \

echo "art-result: $? generate"


