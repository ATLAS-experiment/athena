#!/bin/sh

# art-description: MC+data Simulation+Overlay chain with MT support, HI 2023, config test
# art-type: grid
# art-architecture:  "#x86_64-intel"
# art-include: main/Athena
# art-include: 24.0/Athena

# art-memory: 3999
# art-output: dataOverlay.HITS.pool.root
# art-output: dataOverlay.RDO.pool.root
# art-output: dataOverlay.AOD.pool.root
# art-output: log.*
# art-output: prmon.summary.*
# art-output: prmon.full.*
# art-output: runargs.*
# art-output: *.pkl

events=5

if [ -z ${ATLAS_REFERENCE_DATA+x} ]; then
  ATLAS_REFERENCE_DATA="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art"
fi

EVNT_File="${ATLAS_REFERENCE_DATA}/OverlayTests/DataOverlayEVNT/mc16_5TeV.603398.PhPy8EG_AZNLO_Zmumu_v2.merge.EVNT.e8552_e7400/EVNT.37836290._000357.pool.root.1"
RDO_BKG_File="${ATLAS_REFERENCE_DATA}/OverlayTests/MinBiasOverlay/processed/24.0/v2/data23_hi.00463124.physics_MinBiasOverlay.daq.RDO_BKG._lb0305._SFO-16._0001.pool.root"
HITS_File="dataOverlay.HITS.pool.root"
RDO_File="dataOverlay.RDO.pool.root"
AOD_File="dataOverlay.AOD.pool.root"

export ATHENA_CORE_NUMBER=1

FastChain_tf.py \
   --multithreaded \
   --simulator FullG4MT_QS \
   --randomSeed 123 \
   --inputEVNTFile "${EVNT_File}" \
   --inputRDO_BKGFile "${RDO_BKG_File}" \
   --outputHITSFile "${HITS_File}" \
   --outputRDOFile "${RDO_File}" \
   --maxEvents ${events} \
   --skipEvents 0 \
   --digiSeedOffset1 511 \
   --digiSeedOffset2 727 \
   --preInclude "Campaigns.DataOverlay2023" \
   --postInclude "OverlayConfiguration.DataOverlayConditions.DataOverlay2023Cfg" \
   --conditionsTag "default:CONDBR2-BLKPA-2023-07" \
   --geometryVersion "default:ATLAS-R3S-2021-03-02-00" \
   --postExec 'with open("Config.pkl", "wb") as f: cfg.store(f)' \
   --imf False

rc1=$?
status=$rc1
echo "art-result: $rc1 fast chain"

rc2=-9999
if [ $rc1 -eq 0 ]; then
   Reco_tf.py \
      --multithreaded \
      --inputRDOFile "${RDO_File}" \
      --outputAODFile "${AOD_File}" \
      --maxEvents ${events} \
      --skipEvents 0 \
      --preInclude "Campaigns.DataOverlay2023" \
      --postInclude "OverlayConfiguration.DataOverlayConditions.DataOverlay2023Cfg" \
      --conditionsTag "default:CONDBR2-BLKPA-2023-07" \
      --geometryVersion "default:ATLAS-R3S-2021-03-02-00" \
      --imf False

   rc2=$?
   status=$rc2
fi
echo "art-result: $rc2 reco"

if command -v art.py >/dev/null 2>&1; then
    rc3=-9999
    if [ $rc1 -eq 0 ]; then
        ArtPackage=$1
        ArtJobName=$2
        art.py compare grid --entries 10 "${ArtPackage}" "${ArtJobName}" --mode=semi-detailed --order-trees --file=dataOverlay.HITS.pool.root --file=dataOverlay.RDO.pool.root
        rc3=$?
        status=$rc3
    fi
    echo "art-result: $rc3 regression"
fi

exit $status
