#!/bin/sh
#
# art-description: Run MT and ST simulation with concurrent Geant4-Gaudi event loop. Experimental development for Run 4
# art-include: main/Athena

# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-athena-mt: 8
# art-output: log.*
# art-output: test.MT.HITS.pool.root
# art-output: test.ST.HITS.pool.root

export ATHENA_CORE_NUMBER=8

# Only a limited subset of detectors are supported as they require update in the sensitive detector code
AtlasG4_tf.py \
    --CA \
    --multithreaded \
    --detectors 'Tile,BCM,Pixel,SCT,TRT' \
    --useG4Workers True \
    --inputEVNTFile '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/ttbar_muplusjets-pythia6-7000.evgen.pool.root' \
    --outputHITSFile 'test.MT.HITS.pool.root' \
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
echo  "art-result: $rc MTsim"
status=$rc

unset ATHENA_CORE_NUMBER
# Only a limited subset of detectors are supported as they require update in the sensitive detector code
AtlasG4_tf.py \
    --CA \
    --detectors 'Tile,BCM,Pixel,SCT,TRT' \
    --useG4Workers True \
    --inputEVNTFile '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/ttbar_muplusjets-pythia6-7000.evgen.pool.root' \
    --outputHITSFile 'test.ST.HITS.pool.root' \
    --maxEvents '20' \
    --skipEvents '0' \
    --randomSeed '10' \
    --geometryVersion 'default:ATLAS-R3S-2021-03-00-00' \
    --conditionsTag 'default:OFLCOND-MC21-SDR-RUN3-07' \
    --DataRunNumber '284500' \
    --physicsList 'FTFP_BERT_ATL' \
    --postInclude 'PyJobTransforms.UseFrontier' \
    --imf False
rc2=$?
echo  "art-result: $rc2 STsim"
if [ $status -eq 0 ]
then
    status=$rc2
fi

rc3=-9999
if [ $status -eq 0 ]
then
    acmd.py diff-root test.MT.HITS.pool.root test.ST.HITS.pool.root --error-mode resilient --mode=semi-detailed --order-trees
    rc3=$?
    status=$rc3
fi
echo  "art-result: $rc3 comparision"
exit $status
