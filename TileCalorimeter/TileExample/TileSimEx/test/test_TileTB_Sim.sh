#!/bin/sh
#
# art-description: Tile Test Beam Simulation example
# art-type: build
# art-include: main/Athena

TestBeam_tf.py \
    --DataRunNumber '1' \
    --outputHITSFile 'test.HITS.pool.root' \
    --conditionsTag 'OFLCOND-MC16-SDR-RUN2-12' \
    --maxEvents '10' \
    --Eta '0.35' \
    --testBeamConfig 'tbtile' \
    --postInclude 'PyJobTransforms.TransformUtils.UseFrontier' \
    --imf False

echo  "art-result: $? simulation"
