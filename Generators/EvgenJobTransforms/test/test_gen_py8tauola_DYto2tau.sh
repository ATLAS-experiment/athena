#!/bin/bash
# art-description: Generation test Pythia8Tauolapp Z->tautau 
# art-include: main/AthGeneration
# art-include: main--HepMC2/Athena
# art-include: 22.0/Athena
# art-type: build
# art-output: *.root
# art-output: log.generate
## Any arguments are considered overrides, and will be added at the end
export TRF_ECHO=True;
Gen_tf.py --ecmEnergy=13600 --jobConfig=421112 --maxEvents=100 \
    --outputEVNTFile=test_TauolaPP_evtgen.EVNT.pool.root \

echo "art-result: $? generate"




