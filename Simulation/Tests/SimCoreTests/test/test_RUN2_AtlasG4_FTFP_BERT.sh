#!/bin/sh
#
# art-description: Run simulation outside ISF, reading di-jet events, using FTFP_BERT physics list, writing HITS, using Best Knowledge RUN2 geometry and conditions
# art-include: 24.0/Athena
# art-include: 24.0/AthSimulation
# art-include: main/Athena
# art-include: main/AthSimulation
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-output: *.HITS.pool.root
# art-output: log.*
# art-output: Config*.pkl

AtlasG4_tf.py \
    --CA \
    --conditionsTag 'OFLCOND-MC23-SDR-RUN3-08' \
    --physicsList 'FTFP_BERT' \
    --preInclude 'AtlasG4Tf:Campaigns.MC23SimulationNoIoV' \
    --postInclude 'default:PyJobTransforms.UseFrontier' \
    --DataRunNumber '284500' \
    --geometryVersion 'default:ATLAS-R2-2016-01-02-01' \
    --inputEVNTFile '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/J2_jetjet-pythia6-7000.evgen.pool.root' \
    --outputHITSFile "test.HITS.pool.root" \
    --maxEvents '10' \
    --skipEvents '0' \
    --randomSeed '10' \
    --postExec 'with open("ConfigSimCA.pkl", "wb") as f: cfg.store(f)' \
    --imf False

rc=$?
mv log.AtlasG4Tf log.AtlasG4Tf_CA
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
