#!/bin/bash
#
# art-description: Reco_tf.py Run 4, mu=1, HITStoRDO,RAWtoALL in MT mode, comparing COOL and CREST
# art-type: grid
# art-include: main/Athena
# art-athena-mt: 8

if [ -z ${ATLAS_REFERENCE_DATA+x} ]; then
  ATLAS_REFERENCE_DATA="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art"
fi

HSHitsFile="${ATLAS_REFERENCE_DATA}/PhaseIIUpgrade/HITS/ATLAS-P2-RUN4-04-00-00/mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.simul.HITS.e8481_s4494/HITS.43777451._000083.pool.root.1"
MinBiasHighHitsFiles="${ATLAS_REFERENCE_DATA}/PhaseIIUpgrade/HITS/ATLAS-P2-RUN4-04-00-00/mc21_14TeV.800831.Py8EG_minbias_inelastic_highjetphotonlepton.merge.HITS.e8481_s4494_s4493/*"
MinBiasLowHitsFiles="${ATLAS_REFERENCE_DATA}/PhaseIIUpgrade/HITS/ATLAS-P2-RUN4-04-00-00/mc21_14TeV.900311.Epos_minbias_inelastic_lowjetphoton.merge.HITS.e8481_s4494_s4493/*"


conditions="OFLCOND-MC21-SDR-RUN4-03"
conditions_crest="CREST-MC21-SDR-RUN4-03"

Reco_tf.py \
  --conditionsTag "${conditions}" \
  --geometryVersion ATLAS-P2-RUN4-04-00-00 \
  --digiSteeringConf "StandardSignalOnlyTruth" \
  --preInclude "all:Campaigns.MC23PhaseIIPileUp1" \
  --postInclude "all:PyJobTransforms.UseFrontier.py" \
  --inputHITSFile "${HSHitsFile}" \
  --inputHighPtMinbiasHitsFile ${MinBiasHighHitsFiles} \
  --inputLowPtMinbiasHitsFile ${MinBiasLowHitsFiles} \
  --outputAODFile RUN4_COOL.AOD.pool.root \
  --imf="False" \
  --jobNumber 1 \
  --maxEvents 50

rc1=$?
echo "art-result: ${rc1} Reco_tf_RUN4_r2a_mt (COOL)"

# Check for FPEs in the logiles
test_trf_check_fpe.sh
fpeStat=$?

echo "art-result: ${fpeStat} FPEs in logfiles"

Reco_tf.py \
  --conditionsTag "${conditions_crest}" \
  --geometryVersion ATLAS-P2-RUN4-04-00-00 \
  --digiSteeringConf "StandardSignalOnlyTruth" \
  --preInclude "all:Campaigns.MC23PhaseIIPileUp1,PyJobTransforms.UseCREST" \
  --inputHITSFile "${HSHitsFile}" \
  --inputHighPtMinbiasHitsFile ${MinBiasHighHitsFiles} \
  --inputLowPtMinbiasHitsFile ${MinBiasLowHitsFiles} \
  --outputAODFile RUN4_CREST.AOD.pool.root \
  --imf="False" \
  --jobNumber 1 \
  --maxEvents 50

rc2=$?
echo "art-result: ${rc2} Reco_tf_RUN4_r2a_mt (CREST)"

acmd.py diff-root --order-trees --nan-equal  RUN4_COOL.AOD.pool.root RUN4_CREST.AOD.pool.root &> diff_cool_vs_crest.txt
rootdiffStat=$?
echo  "art-result: ${rootdiffStat} diff-root COOL vs CREST"

