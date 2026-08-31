#!/bin/bash
#
# art-description: Reco_tf.py Run 4, mu=200, OFFC raw channel building with 32 samples
# art-type: grid
# art-include: main/Athena
# art-output: *.root
# art-output: log.*
#
# Runs the OFFC in the configuration it is written for: 32 ROD samples with 24
# preceding. Tuning parameters are left at their defaults.

if [ -z ${ATLAS_REFERENCE_DATA+x} ]; then
  ATLAS_REFERENCE_DATA="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art"
fi

RUN4HITS="${ATLAS_REFERENCE_DATA}/PhaseIIUpgrade/HITS/ATLAS-P2-RUN4-05-00-00"
HSHitsFile="${RUN4HITS}/mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.simul.HITS.e8481_s4676/HITS.51318242._004216.pool.root.1"
HighPtMinbiasHitsFiles="${RUN4HITS}/mc21_14TeV.800831.Py8EG_minbias_inelastic_highjetphotonlepton.merge.HITS.e8481_s4676_s4677/*"
LowPtMinbiasHitsFiles="${RUN4HITS}/mc21_14TeV.900311.Epos_minbias_inelastic_lowjetphoton.merge.HITS.e8481_s4676_s4677/*"

conditions=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN4_MC)")

RDOFile="RUN4.OFFC.mu200.RDO.pool.root"

Reco_tf.py \
  --conditionsTag ${conditions} \
  --digiSteeringConf "StandardSignalOnlyTruth" \
  --preInclude "all:Campaigns.MC23PhaseIIPileUp200" \
  --preExec "HITtoRDO:flags.LAr.ROD.OFFCRawChannelBuilding=True; flags.LAr.ROD.nSamples=32; flags.LAr.ROD.nPreceedingSamples=24;" \
  --postInclude "all:PyJobTransforms.UseFrontier" \
  --inputHITSFile "${HSHitsFile}" \
  --inputHighPtMinbiasHitsFile ${HighPtMinbiasHitsFiles} \
  --inputLowPtMinbiasHitsFile ${LowPtMinbiasHitsFiles} \
  --outputRDOFile ${RDOFile} \
  --jobNumber 1 \
  --maxEvents 25

rc1=$?
status=${rc1}
echo "art-result: ${rc1} OFFC_HITtoRDO"

rc2=-9999
if [ ${rc1} -eq 0 ]; then
  RunRDOAnalysis.py -i "${RDOFile}"
  rc2=$?
  status=${rc2}
fi
echo "art-result: ${rc2} RDOAnalysis"

if command -v art.py >/dev/null 2>&1; then
  rc3=-9999
  if [ ${rc1} -eq 0 ]; then
    art.py compare grid --entries 10 "${1}" "${2}" --mode=semi-detailed --file="${RDOFile}"
    rc3=$?
    status=${rc3}
  fi
  echo "art-result: ${rc3} regression"
fi

exit ${status}
