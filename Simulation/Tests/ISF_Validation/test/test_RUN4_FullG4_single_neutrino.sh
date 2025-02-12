#!/bin/sh
#
# art-description: MC21-style simulation using FullG4 and RUN4 geometry, single neutrino
# art-include: 24.0/Athena
# art-include: main/Athena
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-memory: 2999
# art-output: test_neutrino.HITS.pool.root

Input="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/EVNT/mc21_14TeV.900149.PG_single_nu_Pt50.evgen.EVNT.e8481/EVNT.30810279._000071.pool.root.1"
Output="test_neutrino.HITS.pool.root"

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
