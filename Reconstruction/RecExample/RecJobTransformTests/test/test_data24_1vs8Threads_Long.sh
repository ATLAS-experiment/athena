#!/bin/sh
#
# art-description: Reco_tf.py runs long test on 1627 events (size of input file) on 2024 13.6 TeV data twice, with 1 and 8 threads, and then does diff-root. Report issues to https://its.cern.ch/jira/projects/ATLASRECTS/
# art-type: grid
# art-include: main/Athena
# art-include: 24.0/Athena
# art-athena-mt: 8
# art-output: runOne
# art-output: runTwo
# art-runon: Monday

preExecString="flags.Reco.EnableTrigger=False;flags.DQ.doMonitoring=False"
INPUTFILE=$(python -c "from AthenaConfiguration.TestDefaults import defaultTestFiles; print(defaultTestFiles.RAW_RUN3_DATA24[0])")
CONDTAG=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN3_DATA)")
GEOTAG=$(python -c "from AthenaConfiguration.TestDefaults import defaultGeometryTags; print(defaultGeometryTags.RUN3)")

mkdir runOne; cd runOne
Reco_tf.py --CA --athenaopts="--threads=1"  --preExec "${preExecString}" \
--conditionsTag="${CONDTAG}" --geometryVersion="${GEOTAG}" --inputBSFile="${INPUTFILE}" \
--outputAODFile=myAOD.pool.root --maxEvents=1000 | tee athenarunOne.log
rc1=${PIPESTATUS[0]}
xAODDigest.py myAOD.pool.root | tee digestOne.log
echo "art-result: $rc1 runOne"

cd ../
mkdir runTwo; cd runTwo
Reco_tf.py --CA --athenaopts="--threads=8" --preExec "${preExecString}" \
--conditionsTag="${CONDTAG}" --geometryVersion="${GEOTAG}" --inputBSFile="${INPUTFILE}" \
--outputAODFile=myAOD.pool.root --maxEvents=1000 | tee athenarunTwo.log
rc2=${PIPESTATUS[0]}
xAODDigest.py myAOD.pool.root | tee digestTwo.log
echo "art-result: $rc2 runTwo"

if [[ $rc1 -eq 0 ]] && [[ $rc2 -eq  0 ]]
then
 echo "Compare two directories"
 art.py compare ref --entries 10 --mode=semi-detailed --order-trees --diff-root . ../runOne/ | tee diffEightThreads.log
 rcDiff=${PIPESTATUS[0]}
 collateDigest.sh digestTwo.log ../runOne/digestOne.log digestDiffOneTwo.log
 echo "art-result: $rcDiff Diff-EightThreads-TwoRuns"
fi
