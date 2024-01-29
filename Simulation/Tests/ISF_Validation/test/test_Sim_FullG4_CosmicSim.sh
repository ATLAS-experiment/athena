#!/bin/sh
#
# art-description: Run cosmics simulation using ISF with the FullG4 simulator, generating events on-the-fly, using 2015 geometry and conditions
# art-include: 23.0/Athena
# art-include: main/Athena
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-output: *.HITS.pool.root
# art-output: *.TR.pool.root
# art-output: log.*
# art-output: Config*.pkl

Sim_tf.py \
--CA \
--conditionsTag 'OFLCOND-RUN12-SDR-19' \
--physicsList 'QGSP_BERT' \
--truthStrategy 'MC15aPlus' \
--simulator 'CosmicsG4' \
--outputHITSFile 'test.CA.HITS.pool.root' \
--outputEVNT_TRFile 'test.CA.TR.pool.root' \
--maxEvents '1500' \
--randomSeed '1234' \
--DataRunNumber '10' \
--CosmicFilterVolume 'Calo' \
--CosmicFilterVolume2 'NONE' \
--geometryVersion 'ATLAS-R2-2015-03-01-00' \
--CosmicPtSlice 'NONE' \
--beamType 'cosmics' \
--postInclude 'PyJobTransforms.TransformUtils.UseFrontier,CosmicGenerator.CosmicGeneratorConfig.postIncludeCosmicGenerator' \
--postExec 'with open("ConfigSimCA.pkl", "wb") as f: cfg.store(f)' \
--imf False

rc=$?
status=$rc
mv log.EVNTtoHITS log.EVNTtoHITS_CA
echo  "art-result: $rc simCA"

Sim_tf.py \
--conditionsTag 'OFLCOND-RUN12-SDR-19' \
--physicsList 'QGSP_BERT' \
--truthStrategy 'MC15aPlus' \
--simulator 'CosmicsG4' \
--outputEVNT_TRFile 'test.CA.TR.pool.root' \
--outputHITSFile 'test.CA.HITS.pool.root' \
--maxEvents '1500' \
--randomSeed '1234' \
--DataRunNumber '10' \
--CosmicFilterVolume 'Calo' \
--CosmicFilterVolume2 'NONE' \
--preInclude 'SimulationJobOptions/preInclude.Cosmics.py' \
--geometryVersion 'ATLAS-R2-2015-03-01-00' \
--CosmicPtSlice 'NONE' \
--beamType 'cosmics' \
--imf False \
--athenaopts '"--config-only=ConfigSimCG.pkl"'

Sim_tf.py \
--conditionsTag 'OFLCOND-RUN12-SDR-19' \
--physicsList 'QGSP_BERT' \
--truthStrategy 'MC15aPlus' \
--simulator 'CosmicsG4' \
--outputEVNT_TRFile 'test.TR.pool.root' \
--outputHITSFile 'test.HITS.pool.root' \
--maxEvents '1500' \
--randomSeed '1234' \
--DataRunNumber '10' \
--CosmicFilterVolume 'Calo' \
--CosmicFilterVolume2 'NONE' \
--preInclude 'SimulationJobOptions/preInclude.Cosmics.py' \
--geometryVersion 'ATLAS-R2-2015-03-01-00' \
--CosmicPtSlice 'NONE' \
--beamType 'cosmics' \
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
    acmd.py diff-root test.TR.pool.root test.CA.TR.pool.root --error-mode resilient --mode=semi-detailed --order-trees --ignore-leaves RecoTimingObj_p1_EVNTtoHITS_timings index_ref
    rc3=$?
    status=$rc3
fi
echo  "art-result: $rc3 TR_OLDvsCA"

rc4=-9999
if [ $rc -eq 0 ] && [ $rc2 -eq 0 ]
then
    acmd.py diff-root test.HITS.pool.root test.CA.HITS.pool.root --error-mode resilient --mode=semi-detailed --order-trees --ignore-leaves RecoTimingObj_p1_EVNTtoHITS_timings index_ref
    rc4=$?
    if [ $status -eq 0 ]
    then
        status=$rc4
    fi
fi
echo  "art-result: $rc4 HITS_OLDvsCA"


rc5=-9999
if [ $rc2 -eq 0 ]
then
    ArtPackage=$1
    ArtJobName=$2
    art.py compare grid --entries 10 ${ArtPackage} ${ArtJobName} --mode=semi-detailed --file=test.HITS.pool.root --file=test.TR.pool.root
    rc5=$?
    if [ $status -eq 0 ]
        status=$rc5
    fi
fi
echo  "art-result: $rc5 regression"

exit $status
