#!/bin/sh
#
# art-description: Reco_tf runs on 2022 13.6 TeV collision data, where Run 435229 includes AFP detector. Details at https://twiki.cern.ch/twiki/bin/view/AtlasProtected/SpecialRunsIn2022#done_47_SM_HI_combined_LHCf_ZDC. Report issues to https://its.cern.ch/jira/projects/ATLASRECTS/
# art-athena-mt: 8
# art-type: grid
# art-include: main/Athena
# art-include: 24.0/Athena

export ATHENA_CORE_NUMBER=8
INPUTFILE=$(python -c "from AthenaConfiguration.TestDefaults import defaultTestFiles; print(defaultTestFiles.RAW_RUN3_DATA22_AFP[0])")
CONDTAG=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN3_DATA22)")
GEOTAG=$(python -c "from AthenaConfiguration.TestDefaults import defaultGeometryTags; print(defaultGeometryTags.RUN3)")

Reco_tf.py --multithreaded --maxEvents=10 \
--inputBSFile="${INPUTFILE}" --conditionsTag="${CONDTAG}" --geometryVersion="${GEOTAG}" \
--outputESDFile myESD.pool.root --outputAODFile myAOD.pool.root --outputHISTFile myHist.root

#Remember retval of transform as art result
RES=$?
xAODDigest.py myAOD.pool.root digest.txt
echo "art-result: $RES Reco"
