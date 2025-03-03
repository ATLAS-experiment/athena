#!/bin/sh
#
# art-description: Run test-beam simulation outside ISF, generating events on-the-fly, writing HITS
# art-include: 24.0/Athena
# art-include: main/Athena
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-output: test*HITS.pool.root
# art-output: log.*
# art-output: Config*.pkl

conditions=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN3_MC)")

TestBeam_tf.py \
    --CA \
    --DataRunNumber '1' \
    --outputHITSFile 'test.HITS.pool.root' \
    --conditionsTag "default:${conditions}" \
    --maxEvents '10' \
    --Eta '0.35' \
    --testBeamConfig 'tbtile' \
    --postInclude 'PyJobTransforms.TransformUtils.UseFrontier' \
    --postExec 'with open("ConfigSimCA.pkl", "wb") as f: cfg.store(f)' \
    --imf False
rc=$?
mv log.TestBeamTf log.TestBeamTf.CA
echo  "art-result: $rc simCA"
status=$rc

rc2=-9999
if [ $rc -eq 0 ]
then
    ArtPackage=$1
    ArtJobName=$2
    art.py compare grid --entries 10 ${ArtPackage} ${ArtJobName} --mode=semi-detailed --file=test.HITS.pool.root
    rc2=$?
    status=$rc2
fi
echo  "art-result: $rc2 regression"

exit $status
