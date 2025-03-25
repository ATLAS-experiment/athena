#!/bin/sh
#
# art-description: Reco_tf runs on MC23d 13.6 TeV ttbar with HITS input (overlay+trigger+reco). Report issues to https://its.cern.ch/jira/projects/ATLASRECTS/
# art-output: log.*
# art-athena-mt: 8
# art-type: grid
# art-include: main/Athena
# art-include: 24.0/Athena

export ATHENA_CORE_NUMBER=8
INPUTFILE=$(python -c "from AthenaConfiguration.TestDefaults import defaultTestFiles; print(defaultTestFiles.HITS_RUN3_2023[0])")
CONDTAG=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN3_MC)")
GEOTAG=$(python -c "from AthenaConfiguration.TestDefaults import defaultGeometryTags; print(defaultGeometryTags.RUN3)")
BKGFILE=$(python -c "from AthenaConfiguration.TestDefaults import defaultTestFiles; print(defaultTestFiles.RDO_BKG_RUN3_2023[0])")

#TODO update input files from mc23d campaign when available

Reco_tf.py --CA "default:True" "RDOtoRDOTrigger:False" \
	   --multithreaded --maxEvents=300 \
	   --asetup "RDOtoRDOTrigger:Athena,23.0.32.7" \
	   --steering "doOverlay" "doRDO_TRIG" "doTRIGtoALL" \
	   --inputHITSFile="${INPUTFILE}" --conditionsTag="${CONDTAG}" --geometryVersion="${GEOTAG}" \
	   --inputRDO_BKGFile="${BKGFILE}" \
	   --outputRDOFile=myRDO.pool.root --outputAODFile=myAOD.pool.root --outputESDFile=myESD.pool.root \
	   --preInclude 'Campaigns.MC23d'

RES=$?
echo "art-result: $RES Reco"
