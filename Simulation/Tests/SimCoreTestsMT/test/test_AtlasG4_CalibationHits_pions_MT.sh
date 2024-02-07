#!/bin/sh
#
# art-description: Run MT simulation outside ISF, reading single pion events, writing HITS including CaloCalibrationHits, using MC16 geometry and conditions
# art-include: 24.0/Athena
# art-include: main/Athena
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-athena-mt: 8
# art-output: log.*
# art-output: test.HITS.pool.root

export ATHENA_CORE_NUMBER=8

AtlasG4_tf.py \
    --CA \
    --multithreaded \
    --inputEVNTFile '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/ISF_Validation/pi_E50_eta0-60.evgen.pool.root' \
    --outputHITSFile 'test.HITS.pool.root' \
    --maxEvents '10' \
    --skipEvents '0' \
    --randomSeed '10' \
    --geometryVersion 'ATLAS-R2-2016-01-00-01' \
    --conditionsTag 'OFLCOND-MC16-SDR-14' \
    --DataRunNumber '284500' \
    --physicsList 'FTFP_BERT_ATL' \
    --postInclude 'PyJobTransforms.UseFrontier' \
    --preInclude 'SimuJobTransforms.CalHits,SimuJobTransforms.ParticleID' \
    --postExec 'AtlasG4Tf:with open("ConfigSimCA.pkl", "wb") as f: cfg.store(f)' \
    --imf False

rc=$?
status=$rc
echo  "art-result: $rc simCA"
mv log.AtlasG4Tf log.AtlasG4Tf_CA

rc2=-9999
if [ $rc -eq 0 ]
then
    ArtPackage=$1
    ArtJobName=$2
    art.py compare grid --entries 10 ${ArtPackage} ${ArtJobName} --mode=semi-detailed --order-trees --file=test.HITS.pool.root
    rc2=$?
    if [ $status -eq 0 ]
    then
        status=$rc2
    fi
fi
echo  "art-result: $rc2 regression"
exit $status
