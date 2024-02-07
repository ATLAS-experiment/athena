#!/bin/sh
#
# art-description: Run simulation outside ISF, using 2011 geometry and conditions, reading single muon events, writing HITS
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
    --conditionsTag 'OFLCOND-RUN12-SDR-19' \
    --postInclude 'PyJobTransforms.UseFrontier' \
    --preInclude 'AtlasG4Tf:Campaigns.MC23SimulationNoIoV' \
    --DataRunNumber '180164' \
    --geometryVersion 'ATLAS-R1-2011-02-00-00' \
    --inputEVNTFile '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/mu_E200_eta0-25.evgen.pool.root' \
    --outputHITSFile 'test.HITS.pool.root' \
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
