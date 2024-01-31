#!/bin/sh
#
# art-description: Run cosmics simulation using ISF with the FullG4 simulator, using TrackRecords as input, using 2015 geometry and conditions
# art-include: 23.0/Athena
# art-include: main/Athena
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-include: master/Athena
# art-output: test.*.HITS.pool.root
# art-output: log.*
# art-output: Config*.pkl

Sim_tf.py \
    --CA \
    --inputEVNT_TRFile '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/ISF_Validation/Cosmics.TR.pool.root' \
    --outputHITSFile 'test.CA.HITS.pool.root' \
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

Sim_tf.py \
    --inputEVNT_TRFile '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/ISF_Validation/Cosmics.TR.pool.root' \
    --outputHITSFile 'test.CA.HITS.pool.root' \
    --simulator 'FullG4MT' \
    --maxEvents '-1' \
    --randomSeed '1234' \
    --geometryVersion 'ATLAS-R2-2015-03-01-00_VALIDATION' \
    --conditionsTag 'OFLCOND-RUN12-SDR-19' \
    --physicsList 'FTFP_BERT_ATL' \
    --DataRunNumber '10' \
    --firstEvent '1' \
    --preInclude 'SimulationJobOptions/preInclude.Cosmics.py' \
    --beamType 'cosmics' \
    --postInclude 'PyJobTransforms/UseFrontier.py' \
    --truthStrategy 'MC15aPlus' \
    --imf False \
    --athenaopts '"--config-only=ConfigSimCG.pkl"'

Sim_tf.py \
    --inputEVNT_TRFile '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/ISF_Validation/Cosmics.TR.pool.root' \
    --outputHITSFile 'test.HITS.pool.root' \
    --simulator 'FullG4MT' \
    --maxEvents '-1' \
    --randomSeed '1234' \
    --geometryVersion 'ATLAS-R2-2015-03-01-00_VALIDATION' \
    --conditionsTag 'OFLCOND-RUN12-SDR-19' \
    --physicsList 'FTFP_BERT_ATL' \
    --DataRunNumber '10' \
    --firstEvent '1' \
    --preInclude 'SimulationJobOptions/preInclude.Cosmics.py' \
    --beamType 'cosmics' \
    --postInclude 'PyJobTransforms/UseFrontier.py' \
    --truthStrategy 'MC15aPlus' \
    --imf False

rc2=$?
if [ $status -eq 0 ]
then
    status=$rc2
fi
mv log.TRtoHITS log.TRtoHITS_OLD
echo  "art-result: $rc2 simOLD"

rc3=-9999
if [ $status -eq 0 ]
then
    acmd.py diff-root test.HITS.pool.root test.CA.HITS.pool.root --error-mode resilient --mode=semi-detailed
    rc3=$?
    status=$rc3
fi
echo  "art-result: $rc3 OLDvsCA"

rc4=-9999
if [ $rc2 -eq 0 ]
then
    ArtPackage=$1
    ArtJobName=$2
    art.py compare grid --entries 10 ${ArtPackage} ${ArtJobName} --mode=semi-detailed --file=test.HITS.pool.root
    rc4=$?
    if [ $status -eq 0 ]
    then
        status=$rc4
    fi
fi
echo  "art-result: $rc4 regression"

exit $status
