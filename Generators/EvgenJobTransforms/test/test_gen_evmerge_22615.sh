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


EVNTMerge_tf.py --inputEVNTFile="test_minbias.EVNT.pool.root" --maxEvents="10000" --skipEvents="0" \ 
    --outputEVNT_MRGFile="merge_minbias.EVNT.pool.root" --AMITag="e8455" \
    --asetup "EVNTMerge:AthGeneration,22.6.15,gcc11,centos7"
    
echo "art-result: $? generate"





