#!/bin/sh
#
# art-description: Run cosmics simulation using ISF with the FullG4 simulator, using TrackRecords as input, using 2015 geometry and conditions
# art-include: 24.0/Athena
# art-include: main/Athena
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-include: master/Athena
# art-output: test.HITS.pool.root
# art-output: log.*
# art-output: Config*.pkl

Sim_tf.py \
    --CA \
    --inputEVNT_TRFile '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/ISF_Validation/Cosmics.TR.pool.root' \
    --outputHITSFile 'test.HITS.pool.root' \
    --simulator 'FullG4MT' \
    --maxEvents '-1' \
    --randomSeed '1234' \
    --geometryVersion 'ATLAS-R2-2015-03-01-00' \
    --conditionsTag 'OFLCOND-RUN12-SDR-19' \
    --physicsList 'FTFP_BERT_ATL' \
    --DataRunNumber '10' \
    --firstEvent '1' \
    --beamType 'cosmics' \
    --postInclude 'PyJobTransforms.UseFrontier' \
    --postExec 'with open("ConfigSimCA.pkl", "wb") as f: cfg.store(f)' \
    --truthStrategy 'MC15aPlus' \
    --imf False

rc=$?
status=$rc
mv log.TRtoHITS log.TRtoHITS_CA
echo  "art-result: $rc simCA"

rc2=-9999
if [ $rc -eq 0 ]
then
    ArtPackage=$1
    ArtJobName=$2
    art.py compare grid --entries 10 ${ArtPackage} ${ArtJobName} --mode=semi-detailed --file=test.HITS.pool.root
    rc2=$?
    if [ $status -eq 0 ]
    then
        status=$rc2
    fi
fi
echo  "art-result: $rc2 regression"

exit $status
