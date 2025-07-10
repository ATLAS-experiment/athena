#!/bin/sh

# art-description: MC+data Overlay with MT support, HI 2023, running with 8 threads, full chain
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-athena-mt: 8
# art-include: main/Athena
# art-include: 24.0/Athena

# art-output: dataOverlay.RDO.pool.root
# art-output: dataOverlay.AOD.pool.root
# art-output: log.*
# art-output: prmon.summary.*
# art-output: prmon.full.*
# art-output: runargs.*
# art-output: *.pkl

export ATHENA_CORE_NUMBER=8

events=25

if [ -z ${ATLAS_REFERENCE_DATA+x} ]; then
  ATLAS_REFERENCE_DATA="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art"
fi

FastChain_tf.py \
--multithreaded \
--simulator FullG4MT_QS \
--randomSeed 123 \
--runNumber 603398 \
--inputEVNTFile "${ATLAS_REFERENCE_DATA}/OverlayTests/DataOverlayEVNT/mc16_5TeV.603398.PhPy8EG_AZNLO_Zmumu_v2.merge.EVNT.e8552_e7400/EVNT.37836290._000357.pool.root.1" \
--inputRDO_BKGFile "${ATLAS_REFERENCE_DATA}/OverlayTests/MinBiasOverlay/processed/24.0/v2/data23_hi.00463124.physics_MinBiasOverlay.daq.RDO_BKG._lb0305._SFO-16._0001.pool.root" \
--outputRDOFile dataOverlay.RDO.pool.root \
--outputAODFile dataOverlay.AOD.pool.root \
--maxEvents ${events} \
--conditionsTag "default:CONDBR2-BLKPA-2023-07" \
--geometryVersion "default:ATLAS-R3S-2021-03-02-00" \
--preInclude "Campaigns.DataOverlay2023" \
--postInclude "OverlayConfiguration.DataOverlayConditions.DataOverlay2023Cfg" \
--postExec "with open('ConfigOverlay.pkl', 'wb') as f: cfg.store(f)" \
--imf False

rc=$?
status=$rc
echo "art-result: $rc overlay"

if command -v art.py >/dev/null 2>&1; then
    rc2=-9999
    if [ $rc -eq 0 ]
    then
        ArtPackage=$1
        ArtJobName=$2
        art.py compare grid --entries 10 "${ArtPackage}" "${ArtJobName}" --mode=semi-detailed --order-trees --diff-root
        rc2=$?
        status=$rc2
    fi
    echo  "art-result: $rc2 regression"
fi

exit $status
