#!/bin/sh

# art-description: MC+data Overlay with MT support, HI 2023, config test
# art-type: grid
# art-architecture:  "#x86_64-intel"
# art-include: main/Athena
# art-include: 24.0/Athena

# art-output: dataOverlay.RDO.pool.root
# art-output: dataOverlayBS.RDO.pool.root
# art-output: log.*
# art-output: prmon.summary.*
# art-output: prmon.full.*
# art-output: runargs.*
# art-output: *.pkl

events=2

if [ -z ${ATLAS_REFERENCE_DATA+x} ]; then
  ATLAS_REFERENCE_DATA="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art"
fi

Overlay_tf.py \
--detectors TRT \
--inputHITSFile "${ATLAS_REFERENCE_DATA}/OverlayTests/DataOverlaySimulation/24.0/v1/mc23_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.simul.HITS.pool.root" \
--inputBS_SKIMFile "${ATLAS_REFERENCE_DATA}/OverlayTests/MinBiasOverlay/data23_hi.00463124.physics_MinBiasOverlay.daq.RAW._lb0305._SFO-16._0001.data" \
--outputRDOFile dataOverlayBS.RDO.pool.root \
--maxEvents $events \
--conditionsTag "default:CONDBR2-BLKPA-2023-07" \
--geometryVersion "default:ATLAS-R3S-2021-03-02-00" \
--preInclude "Campaigns.DataOverlay2023" \
--postExec "with open('ConfigOverlayBS.pkl', 'wb') as f: cfg.store(f)" \
--imf False \
--athenaopts="--threads=1"

rc1=$?
status=$rc1
echo "art-result: $rc1 overlay BS"
mv log.Overlay log.OverlayBS
mv prmon.summary.Overlay.json prmon.summary.OverlayBS.json
mv prmon.full.Overlay prmon.full.OverlayBS
mv runargs.Overlay.py runargs.OverlayBS.py
mv OverlayTestConfig.txt OverlayBSTestConfig.txt


Overlay_tf.py \
--detectors TRT \
--inputHITSFile "${ATLAS_REFERENCE_DATA}/OverlayTests/DataOverlaySimulation/24.0/v1/mc23_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.simul.HITS.pool.root" \
--inputRDO_BKGFile "${ATLAS_REFERENCE_DATA}/OverlayTests/MinBiasOverlay/processed/24.0/v2/data23_hi.00463124.physics_MinBiasOverlay.daq.RDO_BKG._lb0305._SFO-16._0001.pool.root" \
--outputRDOFile dataOverlay.RDO.pool.root \
--maxEvents $events \
--conditionsTag "default:CONDBR2-BLKPA-2023-07" \
--geometryVersion "default:ATLAS-R3S-2021-03-02-00" \
--preInclude "Campaigns.DataOverlay2023" \
--postExec "with open('ConfigOverlay.pkl', 'wb') as f: cfg.store(f)" \
--imf False \
--athenaopts="--threads=1"

rc2=$?
if [ $status -eq 0 ]; then
    status=$rc2
fi
echo "art-result: $rc2 overlay"


rc3=-9999
if [ $rc1 -eq 0 ] && [ $rc2 -eq 0 ]; then
    acmd.py diff-root dataOverlayBS.RDO.pool.root dataOverlay.RDO.pool.root \
        --mode=detailed --error-mode resilient --order-trees --exact-branches \
        --ignore-leaves index_ref \
            xAOD::EventAuxInfo_v3_EventInfoAuxDyn.actualInteractionsPerCrossing \
            xAOD::EventAuxInfo_v3_EventInfoAuxDyn.averageInteractionsPerCrossing \
            xAOD::EventAuxInfo_v3_EventInfoAux.detectorMask0 \
            xAOD::EventAuxInfo_v3_EventInfoAux.detectorMask1 \
            xAOD::EventAuxInfo_v3_EventInfoAux.detectorMask2 \
            xAOD::EventAuxInfo_v3_EventInfoAux.detectorMask3
    rc3=$?
    if [ $status -eq 0 ]; then
        status=$rc3
    fi
fi
echo "art-result: $rc3 BS vs RDO"


if command -v art.py >/dev/null 2>&1; then
    rc4=-9999
    if [ $rc2 -eq 0 ]; then
        ArtPackage=$1
        ArtJobName=$2
        art.py compare grid --entries 10 "${ArtPackage}" "${ArtJobName}" --mode=semi-detailed --order-trees
        rc4=$?
        status=$rc4
    fi
    echo "art-result: $rc4 regression"
fi

exit $status
