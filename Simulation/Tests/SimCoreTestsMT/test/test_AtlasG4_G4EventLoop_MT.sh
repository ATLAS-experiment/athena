#!/bin/sh
#
# art-description: Run MT simulation with concurrent Geant4-Gaudi event loop. Experimental development for Run 4
# art-include: main/Athena

# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-athena-mt: 8
# art-output: log.*
# art-output: test.HITS.pool.root

export ATHENA_CORE_NUMBER=8

# Only a limited subset of detectors are supported as they require update in the sensitive detector code
AtlasG4_tf.py \
    --CA \
    --multithreaded \
    --detectors 'Tile,BCM,Pixel,SCT,TRT' \
    --useG4Workers True \
    --inputEVNTFile '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/ttbar_muplusjets-pythia6-7000.evgen.pool.root' \
    --outputHITSFile 'test.HITS.pool.root' \
    --maxEvents '20' \
    --skipEvents '0' \
    --randomSeed '10' \
    --geometryVersion 'default:ATLAS-R3S-2021-03-00-00' \
    --conditionsTag 'default:OFLCOND-MC21-SDR-RUN3-07' \
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
    art.py compare grid --entries 10 ${ArtPackage} ${ArtJobName} --mode=semi-detailed --order-trees --diff-root
    rc2=$?
    status=$rc2
fi
echo  "art-result: $rc2 regression"
exit $status
