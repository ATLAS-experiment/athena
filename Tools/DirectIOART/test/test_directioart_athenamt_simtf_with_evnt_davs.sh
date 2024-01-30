#!/bin/bash

# art-description: DirectIOART AthenaMT Sim_tf.py inputFile:EVNT protocol=DAVS
# art-type: grid
# art-output: *.pool.root
# art-include: main/Athena
# art-athena-mt: 2

set -e

Sim_tf.py \
<<<<<<< HEAD
    --CA \
    --multithreaded="True" \
    --conditionsTag 'default:OFLCOND-MC23-SDR-RUN3-01' \
    --simulator 'FullG4MT' \
    --postInclude 'PyJobTransforms.UseFrontier' \
    --preInclude 'EVNTtoHITS:Campaigns.MC23aSimulationMultipleIoV' \
    --geometryVersion 'default:ATLAS-R3S-2021-03-02-00' \
    --inputEVNTFile=davs://lcg-lrz-http.grid.lrz.de:443/pnfs/lrz-muenchen.de/data/atlas/dq2/atlasdatadisk/rucio/mc21_13p6TeV/d3/3f/EVNT.29070483._000001.pool.root.1 \
    --outputHITSFile "test.HITS.pool.root" \
    --maxEvents 8 \
    --imf False
=======
  --multithreaded="True" \
  --inputEVNTFile=davs://lcg-lrz-http.grid.lrz.de:443/pnfs/lrz-muenchen.de/data/atlas/dq2/atlasdatadisk/rucio/mc21_13p6TeV/d3/3f/EVNT.29070483._000001.pool.root.1 \
  --maxEvents=8 \
  --postInclude "default:PyJobTransforms/UseFrontier.py" \
  --preInclude "EVNTtoHITS:Campaigns/MC21SimulationMultipleIoV.py,SimulationJobOptions/preInclude.ExtraParticles.py,SimulationJobOptions/preInclude.G4ExtraProcesses.py" \
  --randomSeed=4056 \
  --DBRelease="all:300.0.4" \
  --conditionsTag "default:OFLCOND-MC21-SDR-RUN3-05" \
  --geometryVersion="default:ATLAS-R3S-2021-03-00-00_VALIDATION" \
  --runNumber=801165 \
  --AMITag=s3873 \
  --jobNumber=4056 \
  --firstEvent=4055001 \
  --outputHITSFile=hits.pool.root \
  --simulator=FullG4MT_QS
>>>>>>> ceee5aad679 (reverting accidental changes...)

echo "art-result: $? DirectIOART_AthenaMT_SimTF_inputEVNT_protocol_DAVS"
