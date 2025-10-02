#!/bin/sh

# art-description: MC+data Overlay RDO preparation, HI 2023
# art-type: grid
# art-architecture:  "#x86_64-intel"
# art-include: main/Athena
# art-include: 24.0/Athena

# art-memory: 3999
# art-output: RDO_BKG.pool.root
# art-output: log.*
# art-output: prmon.summary.*
# art-output: prmon.full.*
# art-output: runargs.*
# art-output: *.pkl

if [ -z ${ATLAS_REFERENCE_DATA+x} ]; then
  ATLAS_REFERENCE_DATA="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art"
fi

events=-1

Overlay_tf.py \
   --inputBSFile "${ATLAS_REFERENCE_DATA}/OverlayTests/MinBiasOverlay/data23_hi.00463124.physics_MinBiasOverlay.daq.RAW._lb0305._SFO-16._0001.data" \
   --outputRDO_BKGFile "RDO_BKG.pool.root" \
   --maxEvents ${events} \
   --conditionsTag "default:CONDBR2-BLKPA-2023-07" \
   --geometryVersion "default:ATLAS-R3S-2021-03-02-00" \
   --postExec "with open('Config.pkl', 'wb') as f: cfg.store(f)" \
   --imf False

rc=$?
status=$rc
echo "art-result: $rc BStoRDO"

if command -v art.py >/dev/null 2>&1; then
    rc2=-9999
    if [ $rc -eq 0 ]
    then
        ArtPackage=$1
        ArtJobName=$2
        art.py compare grid --entries 10 "${ArtPackage}" "${ArtJobName}" --mode=semi-detailed --order-trees
        rc2=$?
        status=$rc2
    fi
    echo "art-result: $rc2 regression"
fi

exit $status
