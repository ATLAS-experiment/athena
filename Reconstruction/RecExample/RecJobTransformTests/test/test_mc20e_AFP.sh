#!/bin/sh
#
# art-description: Run mc20e MC setup, with APF, without pileup. Report issues to https://its.cern.ch/jira/projects/ATLASRECTS/
# art-output: log.*
# art-type: grid
# art-athena-mt: 8
# art-include: main/Athena
# art-include: 24.0/Athena

export ATHENA_CORE_NUMBER=8
INPUTFILE=$(python -c "from AthenaConfiguration.TestDefaults import defaultTestFiles; print(defaultTestFiles.HITS_RUN2_MC20_AFP[0])")
CONDTAG=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN2_MC)")
GEOTAG=$(python -c "from AthenaConfiguration.TestDefaults import defaultGeometryTags; print(defaultGeometryTags.RUN2)")

Reco_tf.py --CA 'all:True' 'RDOtoRDOTrigger:False' --multithreaded --maxEvents=300 \
--inputHITSFile="${INPUTFILE}" --conditionsTag="${CONDTAG}" --geometryVersion="${GEOTAG}" \
--preInclude 'HITtoRDO:Campaigns.MC20e' 'RDOtoRDOTrigger:Campaigns/MC20e.py' 'RAWtoALL:Campaigns.MC20e' \
--outputRDOFile=myRDO.pool.root --outputAODFile=myAOD.pool.root --outputESDFile=myESD.pool.root \
--steering 'doRDO_TRIG' --triggerConfig 'RDOtoRDOTrigger=MCRECO:DBF:TRIGGERDBMC:2233,87,314' --asetup "RDOtoRDOTrigger:Athena,21.0,latest"

RES=$?
echo "art-result: $RES Reco"
