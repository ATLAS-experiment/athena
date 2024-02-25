#!/bin/sh
#
# art-description: Reco_tf.py runs short test (25 events) on 2022 13.6 TeV data twice, both times with 8 threads, and then does diff-root.
# art-type: grid
# art-include: main/Athena
# art-include: 24.0/Athena
# art-athena-mt: 8
# art-output: runOne
# art-output: runTwo

preExecString="flags.Reco.EnableTrigger=False;flags.DQ.doMonitoring=False"
conditionsTagString="CONDBR2-BLKPA-2022-09"
geometryVersionString="ATLAS-R3S-2021-03-01-00"

mkdir runOne; cd runOne
Reco_tf.py --CA --athenaopts="--threads=8"  --preExec "${preExecString}" --conditionsTag="${conditionsTagString}" --geometryVersion="${geometryVersionString}" --inputBSFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/RecJobTransformTests/data22_13p6TeV/data22_13p6TeV.00430536.physics_Main.daq.RAW/data22_13p6TeV.00430536.physics_Main.daq.RAW._lb1015._SFO-20._0001.data  --outputAODFile=myAOD.pool.root --outputESDFile=myESD.pool.root  --maxEvents=25 | tee athenarunOne.log
rc1=${PIPESTATUS[0]}
xAODDigest.py myAOD.pool.root | tee digestOne.log
echo "art-result: $rc1 runOne"

cd ../
mkdir runTwo; cd runTwo
Reco_tf.py --CA --athenaopts="--threads=8" --preExec "${preExecString}" --conditionsTag="${conditionsTagString}" --geometryVersion="${geometryVersionString}" --inputBSFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/RecJobTransformTests/data22_13p6TeV/data22_13p6TeV.00430536.physics_Main.daq.RAW/data22_13p6TeV.00430536.physics_Main.daq.RAW._lb1015._SFO-20._0001.data  --outputAODFile=myAOD.pool.root --outputESDFile=myESD.pool.root  --maxEvents=25 | tee athenarunTwo.log
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
