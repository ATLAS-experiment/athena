#!/bin/sh
#
# art-description: MC21-style simulation using FullG4 and RUN4 geometry, single muon
# art-include: 24.0/Athena
# art-include: main/Athena
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-memory: 3999
# art-output: test_muons.HITS.pool.root

Input="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/EVNT/mc21_14TeV.900498.PG_single_muonpm_Pt100_etaFlatnp0_43.evgen.EVNT.e8481/EVNT.30802154._000010.pool.root.1"
Output="test_muons.HITS.pool.root"

geometry=$(python -c "from AthenaConfiguration.TestDefaults import defaultGeometryTags; print(defaultGeometryTags.RUN4)")
conditions=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN4_MC)")

# RUN4 setup
Sim_tf.py \
--CA \
--conditionsTag "default:${conditions}" \
--simulator 'FullG4MT' \
--postInclude 'default:PyJobTransforms.UseFrontier' \
--preInclude 'EVNTtoHITS:Campaigns.PhaseIISimulation' \
--geometryVersion "default:${geometry}" \
--inputEVNTFile "$Input" \
--outputHITSFile "$Output" \
--maxEvents 1000 \
--imf False

rc=$?
status=$rc
echo "art-result: $rc simCA"

rc2=-9999
if [ $rc -eq 0 ]; then
  art.py compare grid --entries 10 "${1}" "${2}" --mode=semi-detailed --file="$Output"
  rc2=$?
  status=$rc2
fi
echo "art-result: $rc2 regression"

exit $status
