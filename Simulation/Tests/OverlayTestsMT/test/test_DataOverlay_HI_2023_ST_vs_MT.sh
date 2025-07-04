#!/bin/sh

# art-description: MC+data Overlay with MT support, HI 2023, running with 8 threads
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-athena-mt: 8
# art-include: main/Athena
# art-include: 24.0/Athena
# art-memory: 3999

# art-output: MC_plus_data.MT.RDO.pool.root
# art-output: MC_plus_data.ST.RDO.pool.root
# art-output: log.*
# art-output: prmon.summary.*
# art-output: prmon.full.*
# art-output: runargs.*
# art-output: *.pkl

export ATHENA_CORE_NUMBER=8

if [ -z ${ATLAS_REFERENCE_DATA+x} ]; then
  ATLAS_REFERENCE_DATA="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art"
fi

Overlay_tf.py \
--multithreaded \
--inputHITSFile "${ATLAS_REFERENCE_DATA}/OverlayTests/DataOverlaySimulation/24.0/v1/mc23_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.simul.HITS.pool.root" \
--inputRDO_BKGFile "${ATLAS_REFERENCE_DATA}/OverlayTests/MinBiasOverlay/processed/24.0/v2/data23_hi.00463124.physics_MinBiasOverlay.daq.RDO_BKG._lb0305._SFO-16._0001.pool.root" \
--outputRDOFile MC_plus_data.MT.RDO.pool.root \
--maxEvents 10 \
--conditionsTag "default:CONDBR2-BLKPA-2023-07" \
--geometryVersion "default:ATLAS-R3S-2021-03-02-00" \
--preInclude "Campaigns.DataOverlay2023" \
--postInclude "OverlayConfiguration.DataOverlayConditions.DataOverlay2023Cfg" \
--postExec "with open('ConfigOverlay.pkl', 'wb') as f: cfg.store(f)" \
--imf False

rc=$?
status=$rc
echo "art-result: $rc overlayMT"
mv log.Overlay log.OverlayMT

rc2=-9999
if [ $rc -eq 0 ]
then
    Overlay_tf.py \
    --inputHITSFile "${ATLAS_REFERENCE_DATA}/OverlayTests/DataOverlaySimulation/24.0/v1/mc23_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.simul.HITS.pool.root" \
    --inputRDO_BKGFile "${ATLAS_REFERENCE_DATA}/OverlayTests/MinBiasOverlay/processed/24.0/v2/data23_hi.00463124.physics_MinBiasOverlay.daq.RDO_BKG._lb0305._SFO-16._0001.pool.root" \
    --outputRDOFile MC_plus_data.ST.RDO.pool.root \
    --maxEvents 10 \
    --conditionsTag "default:CONDBR2-BLKPA-2023-07" \
    --geometryVersion "default:ATLAS-R3S-2021-03-02-00" \
    --preInclude "Campaigns.DataOverlay2023" \
    --postInclude "OverlayConfiguration.DataOverlayConditions.DataOverlay2023Cfg" \
    --postExec "with open('ConfigOverlay.pkl', 'wb') as f: cfg.store(f)" \
    --imf False
    rc2=$?
    status=$rc2
fi
echo  "art-result: $rc2 overlayST"

rc3=-9999
if [ $rc2 -eq 0 ]
then
    acmd.py diff-root MC_plus_data.ST.RDO.pool.root MC_plus_data.MT.RDO.pool.root --error-mode resilient --mode=semi-detailed --order-trees
    rc3=$?
    status=$rc3
fi
echo  "art-result: $rc3 comparison"

if command -v art.py >/dev/null 2>&1; then
    rc4=-9999
    if [ $rc2 -eq 0 ]
    then
        ArtPackage=$1
        ArtJobName=$2
        art.py compare grid --entries 10 "${ArtPackage}" "${ArtJobName}" --mode=semi-detailed --order-trees --diff-root
        rc4=$?
        status=$rc4
    fi
    echo  "art-result: $rc4 regression"
fi

exit $status
