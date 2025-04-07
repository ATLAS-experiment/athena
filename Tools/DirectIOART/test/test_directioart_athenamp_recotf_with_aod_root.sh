#!/bin/bash

# art-description: DirectIOART AthenaMP Reco_tf.py inputFile:AOD protocol=ROOT
# art-type: grid
# art-output: *.pool.root
# art-include: main/Athena
# art-athena-mt: 2

set -e

Derivation_tf.py \
  --multiprocess True \
  --athenaMPMergeTargetSize 'DAOD_*:0' \
  --inputAODFile root://lcg-lrz-rootd.grid.lrz.de:1094/pnfs/lrz-muenchen.de/data/atlas/dq2/atlasdatadisk/rucio/mc15_13TeV/ed/68/AOD.05536542._000001.pool.root.1 \
  --outputDAODFile art.pool.root \
  --formats TEST1 \
  --maxEvents 100

echo "art-result: $? DirectIOART_AthenaMP_RecoTF_inputAOD_protocol_ROOT"
