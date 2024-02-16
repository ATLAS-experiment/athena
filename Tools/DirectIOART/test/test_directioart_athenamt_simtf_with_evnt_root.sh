#!/bin/bash

# art-description: DirectIOART AthenaMT Sim_tf.py inputFile:EVNT protocol=ROOT
# art-type: grid
# art-output: *.pool.root
# art-include: main/Athena
# art-athena-mt: 2

set -e

Sim_tf.py \
    --CA \
    --multithreaded="True" \
    --conditionsTag 'default:OFLCOND-MC23-SDR-RUN3-01' \
    --simulator 'FullG4MT' \
    --postInclude 'PyJobTransforms.UseFrontier' \
    --preInclude 'EVNTtoHITS:Campaigns.MC23aSimulationMultipleIoV' \
    --geometryVersion 'default:ATLAS-R3S-2021-03-02-00' \
    --inputEVNTFile=root://lcg-lrz-rootd.grid.lrz.de:1094/pnfs/lrz-muenchen.de/data/atlas/dq2/atlasdatadisk/rucio/mc21_13p6TeV/d3/3f/EVNT.29070483._000001.pool.root.1 \
    --outputHITSFile "test.HITS.pool.root" \
    --maxEvents 8 \
    --imf False

echo "art-result: $? DirectIOART_AthenaMT_SimTF_inputEVNT_protocol_ROOT"
