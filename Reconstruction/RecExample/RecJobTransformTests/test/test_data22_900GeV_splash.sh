#!/bin/sh
#
# art-description: Reco_tf runs on 2022 900 GeV splash events with all streams
# art-athena-mt: 8
# art-type: grid
# art-include: main/Athena
# art-include: 24.0/Athena

# TODO update following ATLASRECTS-8054
export ATHENA_CORE_NUMBER=8
#Monitoring is disabled because it tries to use the trigger information, which is disabled.
conditionsTag=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN3_DATA22)")
Reco_tf.py --multithreaded \
	   --inputBSFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/RecJobTransformTests/data22_900GeV/data22_900GeV.00423110.physics_Main.daq.RAW/data22_900GeV.00423110.physics_Main.daq.RAW._lb0274._SFO-14._0001.data \
	   --maxEvents=300 \
	   --conditionsTag=$conditionsTag \
	   --geometryVersion="ATLAS-R3S-2021-03-01-00"\
	   --outputESDFile myESD.pool.root \
	   --outputAODFile myAOD.pool.root \
	   --outputHISTFile myHist.root

RES=$?
echo "art-result: $RES Reco"

