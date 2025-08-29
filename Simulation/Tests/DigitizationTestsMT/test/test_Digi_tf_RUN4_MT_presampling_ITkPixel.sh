#!/bin/bash
#
# art-description: Run 4 pile-up pre-mixing MT tests
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-athena-mt: 8
# art-include: main/Athena
# art-output: RUN4_presampling_MT.RDO.pool.root

export ATHENA_CORE_NUMBER=8

if [ -z ${ATLAS_REFERENCE_DATA+x} ]; then
  ATLAS_REFERENCE_DATA="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art"
fi

Events=50
HSHitsFile="${ATLAS_REFERENCE_DATA}/PhaseIIUpgrade/HITS/ATLAS-P2-RUN4-04-00-00/mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.simul.HITS.e8481_s4494/HITS.43777451._000083.pool.root.1"
HighPtMinbiasHitsFiles="${ATLAS_REFERENCE_DATA}/PhaseIIUpgrade/HITS/ATLAS-P2-RUN4-04-00-00/mc21_14TeV.800831.Py8EG_minbias_inelastic_highjetphotonlepton.merge.HITS.e8481_s4494_s4492/*"
LowPtMinbiasHitsFiles="${ATLAS_REFERENCE_DATA}/PhaseIIUpgrade/HITS/ATLAS-P2-RUN4-04-00-00/mc21_14TeV.900311.Epos_minbias_inelastic_lowjetphoton.merge.HITS.e8481_s4494_s4492/*"
DigiOutFileName="RUN4_presampling_MT.RDO.pool.root"

conditions=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN4_MC)")

Digi_tf.py \
--CA \
--detectors ITkPixel \
--multithreaded \
--PileUpPresampling True \
--conditionsTag default:${conditions} \
--digiSeedOffset1 170 --digiSeedOffset2 170 \
--digiSteeringConf 'StandardSignalOnlyTruth' \
--geometryVersion default:ATLAS-P2-RUN4-04-00-00 \
--inputHITSFile ${HSHitsFile} \
--inputHighPtMinbiasHitsFile ${HighPtMinbiasHitsFiles} \
--inputLowPtMinbiasHitsFile ${LowPtMinbiasHitsFiles} \
--jobNumber 568 \
--maxEvents ${Events} \
--outputRDOFile ${DigiOutFileName} \
--preInclude 'HITtoRDO:Campaigns.PhaseIIPileUp1' \
--postInclude 'PyJobTransforms.UseFrontier' \
--skipEvents 0

rc=$?
status=$rc
echo "art-result: $rc digiCA"

if command -v art.py >/dev/null 2>&1; then
  rc2=-9999
  if [ $rc -eq 0 ]; then
    art.py compare grid --entries 10 "${1}" "${2}" --mode=semi-detailed --order-trees --diff-root --file="$DigiOutFileName"
    rc2=$?
    status=$rc2
  fi
  echo "art-result: $rc2 regression"
fi

exit $status
