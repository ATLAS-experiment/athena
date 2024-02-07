#!/bin/sh
#
# art-description: Run cosmics simulation outside ISF, generating events on-the-fly, using 2015 geometry and conditions
# art-include: 24.0/Athena
# art-include: main/Athena
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-output: *.HITS.pool.root
# art-output: *.TR.pool.root
# art-output: log.*
# art-output: Config*.pkl

AtlasG4_tf.py \
    --CA \
    --outputHITSFile 'test.HITS.pool.root' \
    --maxEvents '1500' \
    --randomSeed '1234' \
    --DataRunNumber '222525' \
    --CosmicFilterVolume 'Calo' \
    --CosmicFilterVolume2 'NONE' \
    --geometryVersion 'ATLAS-R2-2015-03-01-00' \
    --conditionsTag 'OFLCOND-RUN12-SDR-19' \
    --physicsList 'FTFP_BERT' \
    --CosmicPtSlice 'NONE' \
    --outputEVNT_TRFile 'test.TR.pool.root' \
    --beamType 'cosmics' \
    --postInclude 'PyJobTransforms.TransformUtils.UseFrontier,CosmicGenerator.CosmicGeneratorConfig.postIncludeCosmicGenerator' \
    --postExec 'with open("ConfigSimCA.pkl", "wb") as f: cfg.store(f)' \
    --truthStrategy 'MC12' \
    --imf False

rc=$?
status=$rc
mv log.AtlasG4Tf log.AtlasG4Tf_CA
echo  "art-result: $rc simCA"

rc2=-9999
if [ $rc -eq 0 ]
then
    ArtPackage=$1
    ArtJobName=$2
    art.py compare grid --entries 10 ${ArtPackage} ${ArtJobName} --mode=semi-detailed --file=test.HITS.pool.root --file=test.TR.pool.root
    rc2=$?
    status=$rc2
fi
echo  "art-result: $rc2 regression"

exit $status
