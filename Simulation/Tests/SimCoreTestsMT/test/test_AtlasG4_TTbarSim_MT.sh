#!/bin/sh
#
# art-description: Run MT simulation outside ISF, reading ttbar events, writing HITS, using MC16 geometry and conditions
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
    --inputEVNTFile '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/ttbar_muplusjets-pythia6-7000.evgen.pool.root' \
    --outputHITSFile 'test.HITS.pool.root' \
    --maxEvents '20' \
    --skipEvents '0' \
    --randomSeed '10' \
    --geometryVersion 'ATLAS-R2-2016-01-00-01' \
    --conditionsTag 'OFLCOND-MC16-SDR-14' \
    --DataRunNumber '284500' \
    --physicsList 'FTFP_BERT_ATL' \
    --postInclude 'PyJobTransforms.UseFrontier' \
    --imf False

rc=$?
echo  "art-result: $rc simulation"
status=$rc

rc2=-9999
if [ $status -eq 0 ]
then
    ArtPackage=$1
    ArtJobName=$2
    art.py compare grid --entries 10 ${ArtPackage} ${ArtJobName} --mode=semi-detailed --order-trees
    rc2=$?
    status=$rc2
fi
echo  "art-result: $rc2 regression"
exit $status
