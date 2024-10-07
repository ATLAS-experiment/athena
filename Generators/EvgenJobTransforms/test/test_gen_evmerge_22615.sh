#!/bin/bash
# art-description: MC Generators test of generationg minbias and merging it in an old release
# art-type: build
# art-include: main/AthGeneration
# art-include: main--HepMC2/Athena
# art-output: *.root
# art-output: log.generate
# art-output: log.EVNTMerge

## Any arguments are considered overrides, and will be added at the end
export TRF_ECHO=True;
Gen_tf.py --ecmEnergy=13600 --jobConfig=421113 --maxEvents=10000 \
    --outputEVNTFile=test_minbias.EVNT.pool.root \


source ${ATLAS_LOCAL_ROOT_BASE}/user/atlasLocalSetup.sh -c x86_64-centos7-gcc11-opt
asetup --platform=x86_64-centos7-gcc11-opt AthGeneration,22.6.15
EVNTMerge_tf.py --inputEVNTFile="test_minbias.EVNT.pool.root" --maxEvents="10000" --skipEvents="0" \ 
    --outputEVNT_MRGFile="merge_minbias.EVNT.pool.root" --AMITag="e8455" \

echo "art-result: $? generate"





