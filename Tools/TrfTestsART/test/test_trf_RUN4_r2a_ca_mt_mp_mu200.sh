#!/bin/bash
#
# art-description: Reco_tf.py Run 4, mu=200, HITStoRDO in MP mode,RAWtoALL in MT mode
# art-type: grid
# art-include: main/Athena
# art-athena-mt: 8

if [ -z ${ATLAS_REFERENCE_DATA+x} ]; then
  ATLAS_REFERENCE_DATA="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art"
fi

HSHitsFile="${ATLAS_REFERENCE_DATA}/PhaseIIUpgrade/HITS/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.simul.HITS.e8481_s4149/HITS.33605501._000106.pool.root.1"
HighPtMinbiasHitsFiles="${ATLAS_REFERENCE_DATA}/PhaseIIUpgrade/HITS/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.800831.Py8EG_minbias_inelastic_highjetphotonlepton.merge.HITS.e8481_s4149_s4150/*"
LowPtMinbiasHitsFiles="${ATLAS_REFERENCE_DATA}/PhaseIIUpgrade/HITS/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.900311.Epos_minbias_inelastic_lowjetphoton.merge.HITS.e8481_s4149_s4150/*"

conditions=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN4_MC)")

Reco_tf.py \
  --conditionsTag ${conditions} \
  --geometryVersion ATLAS-P2-RUN4-03-00-00 \
  --digiSteeringConf "StandardSignalOnlyTruth" \
  --preInclude "all:Campaigns.PhaseIIPileUp200" \
  --postInclude "all:PyJobTransforms.UseFrontier" \
  --inputHITSFile "${HSHitsFile}" \
  --inputHighPtMinbiasHitsFile ${HighPtMinbiasHitsFiles} \
  --inputLowPtMinbiasHitsFile ${LowPtMinbiasHitsFiles} \
  --outputAODFile RUN4.AOD.pool.root \
  --jobNumber 1 \
  --maxEvents 50

rc1=$?
echo "art-result: ${rc1} Reco_tf_RUN4_r2a_mt"

# Check for FPEs in the logiles
test_trf_check_fpe.sh
fpeStat=$?

echo "art-result: ${fpeStat} FPEs in logfiles"
