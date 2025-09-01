#!/bin/bash
#
# art-description: Run 4 digitization of a single muon sample without pile-up
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-memory: 3999
# art-include: main/Athena
# art-output: RUN4_muons.RDO.pool.root
# art-output: RDOAnalysis.root

if [ -z ${ATLAS_REFERENCE_DATA+x} ]; then
  ATLAS_REFERENCE_DATA="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art"
fi

Events=1000
HSHitsFile="${ATLAS_REFERENCE_DATA}/PhaseIIUpgrade/HITS/ATLAS-P2-RUN4-04-00-00/mc21_14TeV.900498.PG_single_muonpm_Pt100_etaFlatnp0_43.simul.HITS.e8481_s4494/HITS.43777402._000098.pool.root.1"
DigiOutFileName="RUN4_muons.RDO.pool.root"

conditions=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN4_MC)")

Digi_tf.py \
--CA \
--conditionsTag default:${conditions} \
--digiSeedOffset1 170 --digiSeedOffset2 170 \
--geometryVersion default:ATLAS-P2-RUN4-04-00-00 \
--inputHITSFile ${HSHitsFile} \
--jobNumber 568 \
--maxEvents ${Events} \
--outputRDOFile ${DigiOutFileName} \
--postInclude 'PyJobTransforms.UseFrontier' \
--preInclude 'HITtoRDO:Campaigns.PhaseIINoPileUp' \
--skipEvents 0

rc=$?
status=$rc
echo "art-result: $rc digiCA"

rc2=-9999
if [ $rc -eq 0 ]; then
  RunRDOAnalysis.py -i "$DigiOutFileName"
  rc2=$?
  status=$rc2
fi
echo "art-result: $rc2 analysis"

if command -v art.py >/dev/null 2>&1; then
  rc3=-9999
  if [ $rc -eq 0 ]; then
    art.py compare grid --entries 10 "${1}" "${2}" --mode=semi-detailed --file="$DigiOutFileName"
    rc3=$?
    status=$rc3
  fi
  echo "art-result: $rc3 regression"
fi

exit $status
