#!/bin/sh
#
# art-description: Tests detector response to single pions, generated on-the-fly, using RUN3 geometry and conditions
# art-include: 24.0/Athena
# art-include: main/Athena
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-output: *.HITS.pool.root
# art-output: log.*
# art-output: Config*.pkl

AtlasG4_tf.py \
    --CA \
    --preExec 'flags.Sim.GenerationConfiguration="ParticleGun.ParticleGunConfig.ParticleGun_SinglePionCfg"' \
    --outputHITSFile 'test.HITS.pool.root' \
    --maxEvents '150' \
    --randomSeed '10' \
    --geometryVersion 'ATLAS-R3S-2021-03-02-00' \
    --conditionsTag 'OFLCOND-MC23-SDR-RUN3-08' \
    --preInclude 'AtlasG4Tf:Campaigns.MC23SimulationSingleIoV' \
    --runNumber '999999' \
    --postInclude 'PyJobTransforms.UseFrontier' \
    --postExec 'with open("ConfigSimCA.pkl", "wb") as f: cfg.store(f)' \
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
    art.py compare grid --entries 10 ${ArtPackage} ${ArtJobName} --mode=semi-detailed --file=test.HITS.pool.root
    rc2=$?
    status=$rc2
fi
echo  "art-result: $rc2 regression"

exit $status
