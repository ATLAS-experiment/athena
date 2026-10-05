#!/bin/sh
#
# art-description: Reco_tf runs on 2015 13 TeV collision data with all streams. Report issues to https://its.cern.ch/jira/projects/ATLASRECTS/
# art-athena-mt: 8
# art-type: grid
# art-include: main/Athena
# art-include: 24.0/Athena

#This configuration is documented here: https://twiki.cern.ch/twiki/bin/view/Atlas/2021FullRun2Reprocessing#Configuration
#Note that in the CA there is no useMuForTRTErrorScaling flag - this correctly auto configures
#Also note that it is believed the bunch structure value no longer needs to be changed to avoid crashes and so is not included.

export ATHENA_CORE_NUMBER=8
INPUTFILE=$(python -c "from AthenaConfiguration.TestDefaults import defaultTestFiles; print(defaultTestFiles.RAW_RUN2_DATA15[0])")
CONDTAG=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN2_DATA)")
GEOTAG=$(python -c "from AthenaConfiguration.TestDefaults import defaultGeometryTags; print(defaultGeometryTags.RUN2)")

Reco_tf.py --multithreaded --maxEvents 300 --autoConfiguration everything \
--inputBSFile="${INPUTFILE}" --conditionsTag="${CONDTAG}" --geometryVersion="${GEOTAG}" \
--outputESDFile myESD.pool.root --outputAODFile myAOD.pool.root --outputHISTFile myHist.root

RES=$?
echo "art-result: $RES Reco"
