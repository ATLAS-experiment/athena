#!/bin/sh
#
# art-description: Reco_tf runs on 2022 13.6 TeV collision data for all streams. Report issues to https://its.cern.ch/jira/projects/ATLASRECTS/
# art-athena-mt: 8
# art-type: grid
# art-include: main/Athena
# art-include: 24.0/Athena

#Based on https://twiki.cern.ch/twiki/bin/view/Atlas/Quick2022Run3Reprocessing
#In CA all the monitoring flags are true by default, so don't set them.
#Don't touch ZDC flag, which is off by default.

export ATHENA_CORE_NUMBER=8
INPUTFILE=$(python -c "from AthenaConfiguration.TestDefaults import defaultTestFiles; print(defaultTestFiles.RAW_RUN3_DATA22[0])")
CONDTAG=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN3_DATA22)")
GEOTAG=$(python -c "from AthenaConfiguration.TestDefaults import defaultGeometryTags; print(defaultGeometryTags.RUN3)")

Reco_tf.py --multithreaded --maxEvents 300 \
--inputBSFile="${INPUTFILE}" --conditionsTag="${CONDTAG}" --geometryVersion="${GEOTAG}" \
--outputESDFile myESD.pool.root --outputAODFile myAOD.pool.root --outputHISTFile myHist.root

#Remember retval of transform as art result
RES=$?
echo "art-result: $RES Reco"
