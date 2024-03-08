#!/bin/bash

# art-description: DirectIOART AthenaMP Reco_tf.py inputFile:AOD protocol=HTTPS
# art-type: grid
# art-output: *.pool.root
# art-include: main/Athena
# art-athena-mt: 2

set -e

Derivation_tf.py --CA \
  --multiprocess True \
  --athenaMPMergeTargetSize 'DAOD_*:0' \
  --inputAODFile https://lcg-lrz-http.grid.lrz.de:443/pnfs/lrz-muenchen.de/data/atlas/dq2/atlasdatadisk/rucio/data18_13TeV/4e/57/data18_13TeV.00349263.physics_Main.merge.AOD.f937_m1972._lb0149._0001.1 \
  --outputDAODFile art.pool.root \
  --formats TEST1 \
  --maxEvents 100

echo "art-result: $? DirectIOART_AthenaMP_RecoTF_inputAOD_protocol_HTTPS"
