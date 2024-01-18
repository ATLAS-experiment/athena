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
    --outputHITSFile 'test.CA.HITS.pool.root' \
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

AtlasG4_tf.py \
    --multithreaded \
    --inputEVNTFile '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/ISF_Validation/pi_E50_eta0-60.evgen.pool.root' \
    --outputHITSFile 'test.CA.HITS.pool.root' \
    --maxEvents '10' \
    --skipEvents '0' \
    --randomSeed '10' \
    --geometryVersion 'ATLAS-R2-2016-01-00-01' \
    --conditionsTag 'OFLCOND-MC16-SDR-14' \
    --DataRunNumber '284500' \
    --physicsList 'FTFP_BERT_ATL' \
    --postInclude 'PyJobTransforms/UseFrontier.py' \
    --preExec 'AtlasG4Tf:simFlags.ReleaseGeoModel=False;' \
    --preInclude 'SimulationJobOptions/preInclude.CalHits.py,SimulationJobOptions/preInclude.ParticleID.py' \
    --imf False \
    --athenaopts '"--config-only=ConfigSimCG.pkl"'

AtlasG4_tf.py \
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
    --postInclude 'PyJobTransforms/UseFrontier.py' \
    --preExec 'AtlasG4Tf:simFlags.ReleaseGeoModel=False;' \
    --preInclude 'SimulationJobOptions/preInclude.CalHits.py,SimulationJobOptions/preInclude.ParticleID.py' \
    --imf False

rc2=$?
echo  "art-result: $rc2 simOLD"
mv log.AtlasG4Tf log.AtlasG4Tf_CG
if [ $status -eq 0 ]
then
    status=$rc2
fi

rc3=-9999
if [[ $rc -eq 0 ]] && [[ $rc2 -eq 0 ]]
then
    acmd.py diff-root test.HITS.pool.root test.CA.HITS.pool.root --error-mode resilient --mode=semi-detailed --order-trees
    rc3=$?
    if [ $status -eq 0 ]
    then
        status=$rc3
    fi
fi
echo  "art-result: $rc3 OLDvsCA"


rc4=-9999
if [ $rc -eq 0 ]
then
    ArtPackage=$1
    ArtJobName=$2
    art.py compare grid --entries 10 ${ArtPackage} ${ArtJobName} --mode=semi-detailed --order-trees
    rc4=$?
    if [ $status -eq 0 ]
    then
        status=$rc4
    fi
fi
echo  "art-result: $rc4 regression"
exit $status
