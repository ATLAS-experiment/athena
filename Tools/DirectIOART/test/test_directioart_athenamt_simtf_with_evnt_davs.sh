#!/bin/bash

# art-description: DirectIOART AthenaMT Sim_tf.py inputFile:EVNT protocol=DAVS
# art-type: grid
# art-output: *.pool.root
# art-include: master/Athena
# art-athena-mt: 2

set -e

Sim_tf.py \
  --multithreaded="True" \
  --inputEVNTFile=davs://lcg-lrz-http.grid.lrz.de:443/pnfs/lrz-muenchen.de/data/atlas/dq2/atlasdatadisk/rucio/mc21_13p6TeV/d3/3f/EVNT.29070483._000001.pool.root.1 \
  --maxEvents=8 \
  --postInclude "default:PyJobTransforms/UseFrontier.py" \
  --preInclude "EVNTtoHITS:Campaigns/MC21SimulationMultiBeamSpot.py,SimulationJobOptions/preInclude.ExtraParticles.py,SimulationJobOptions/preInclude.G4ExtraProcesses.py" \
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

echo "art-result: $? DirectIOART_AthenaMT_SimTF_inputEVNT_protocol_DAVS"
