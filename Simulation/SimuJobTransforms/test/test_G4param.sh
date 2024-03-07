#!/bin/sh
#
# art-description: MC15-style simulation using frozen showers
# art-type: build
# art-include: 21.0/Athena
# art-include: 21.0/AthSimulation
# art-include: 21.3/Athena
# art-include: 21.9/Athena
# art-include: main/Athena
# art-include: main/AthSimulation

# MC15 setup
# ATLAS-R2-2015-03-01-00 and OFLCOND-RUN12-SDR-30
export TRF_ECHO=1
AtlasG4_tf.py \
    --CA \
    --conditionsTag 'default:OFLCOND-MC23-SDR-RUN3-01' \
    --postInclude 'default:PyJobTransforms.UseFrontier' \
    --preInclude 'AtlasG4Tf:Campaigns.MC23aSimulationMultipleIoV' \
    --geometryVersion 'default:ATLAS-R3S-2021-03-02-00' \
    --inputEVNTFile '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/mu_E200_eta0-25.evgen.pool.root' \
    --outputHITSFile 'mu_E200_eta0-25.HITS.pool.root' \
    --maxEvents 5 \
    --skipEvents 0 \
    --jobNumber 857 \
    --imf False

echo  "art-result: $? simulation"
