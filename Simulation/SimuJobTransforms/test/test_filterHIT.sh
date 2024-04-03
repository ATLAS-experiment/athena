#!/bin/sh
#
# art-description: Create a minbias HITS file then run FilterHit_tf.py on it
# art-type: build
# art-include: 24.0/Athena
# art-include: main/Athena

# MC15 setup
# ATLAS-R2-2015-03-01-00 and OFLCOND-RUN12-SDR-30
export TRF_ECHO=1
AtlasG4_tf.py \
    --CA \
    --conditionsTag 'default:OFLCOND-MC23-SDR-RUN3-01' \
    --postInclude 'default:PyJobTransforms.UseFrontier' \
    --preInclude 'AtlasG4Tf:Campaigns.MC23SimulationSingleIoV' \
    --geometryVersion 'default:ATLAS-R3S-2021-03-02-00' \
    --inputEVNTFile '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/minbias_Inelastic-pythia8-7000.evgen.pool.root' \
    --outputHITSFile 'minbias_Inelastic-pythia8-7000_ATLAS-R3S-2021-03-02-00.HITS.pool.root' \
    --maxEvents 5 \
    --skipEvents 0 \

echo  "art-result: $? simulation"

FilterHit_tf.py \
    --CA \
    --inputHITSFile 'minbias_Inelastic-pythia8-7000_ATLAS-R3S-2021-03-02-00.HITS.pool.root' \
    --outputHITS_FILTFile 'minbias_Inelastic-pythia8-7000_ATLAS-R3S-2021-03-02-00.filtered.HITS.pool.root'\
    --maxEvents 5 \
    --skipEvents 0 \
    --geometryVersion 'ATLAS-R3S-2021-03-02-00'

echo  "art-result: $? filter"
