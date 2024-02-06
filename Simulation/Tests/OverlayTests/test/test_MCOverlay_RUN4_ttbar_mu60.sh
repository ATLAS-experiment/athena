#!/bin/sh

# art-description: MC+MC Overlay without reco for Run 4, ttbar, mu=60
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-include: main/Athena

# art-output: *.root
# art-output: log.*
# art-output: mem.summary.*
# art-output: mem.full.*
# art-output: runargs.*

if [ -z ${ATLAS_REFERENCE_DATA+x} ]; then
  ATLAS_REFERENCE_DATA="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art"
fi

events=25
HITS_File="${ATLAS_REFERENCE_DATA}/PhaseIIUpgrade/HITS/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.simul.HITS.e8481_s4149/HITS.33605501._000106.pool.root.1"
RDO_BKG_File="${ATLAS_REFERENCE_DATA}/PhaseIIUpgrade/RDO_BKG/ATLAS-P2-RUN4-03-00-00/RUN4_presampling.mu60.25events.RDO.pool.root"
OverlayOutFile="RUN4_ttbar.mu60.overlay.RDO.pool.root"

Overlay_tf.py \
--CA \
--conditionsTag OFLCOND-MC15c-SDR-14-05 \
--digiSeedOffset1 170 --digiSeedOffset2 170 \
--geometryVersion ATLAS-P2-RUN4-03-00-00 \
--inputHITSFile ${HITS_File} \
--inputRDO_BKGFile ${RDO_BKG_File} \
--maxEvents ${events} \
--outputRDOFile ${OverlayOutFile} \
--preInclude 'HITtoRDO:Campaigns.PhaseIIPileUp60' \
--postInclude 'PyJobTransforms.UseFrontier' \
--skipEvents 0

rc=$?
status=$rc
echo "art-result: $rc overlay"

rc2=-9999
if [ $rc -eq 0 ]; then
  RunRDOAnalysis.py -i "$OverlayOutFile"
  rc2=$?
  status=$rc2
fi
echo "art-result: $rc2 analysis"

if command -v art.py >/dev/null 2>&1; then
  rc3=-9999
  if [ $rc -eq 0 ]; then
    art.py compare grid --entries 10 "${1}" "${2}" --mode=semi-detailed --file="$OverlayOutFile"
    rc3=$?
    status=$rc3
  fi
  echo "art-result: $rc3 regression"
fi

exit $status
