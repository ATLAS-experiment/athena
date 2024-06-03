#!/bin/sh
#
# art-description: Reco_tf on  IDCosmic stream, without trigger.
# art-athena-mt: 8
# art-type: grid
# art-include: main/Athena
# art-include: 24.0/Athena

export ATHENA_CORE_NUMBER=8

# Setting based on a run after Run-3 data taking with best condition knowledge from PROC
Reco_tf.py  --CA --multithreaded --inputBSFile=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/RecJobTransformTests/data24_cos/data24_cos.00475431.physics_IDCosmic.merge.RAW/data24_cos.00475431.physics_IDCosmic.merge.RAW._lb0002._SFO-ALL._0001.1  --maxEvents=300 --conditionsTag="CONDBR2-BLKPA-2024-03" --geometryVersion="ATLAS-R3S-2021-03-02-00" --outputESDFile=myESD.pool.root --outputAODFile=myAOD.pool.root

RES=$?
echo "art-result: $RES Reco"
