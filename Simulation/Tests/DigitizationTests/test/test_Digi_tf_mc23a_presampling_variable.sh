#!/bin/bash
#
# art-description: Run a digitization example to compare configuration between ConfGetter and the new ComponentAccumulator approach.
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-memory: 4096
# art-include: 24.0/Athena
# art-include: main/Athena
# art-output: mc23a_presampling.VarBS.CG.RDO.pool.root
# art-output: log.*
# art-output: legacy.*
# art-output: DigiPUConfig*

if [ -z ${ATLAS_REFERENCE_DATA+x} ]; then
  ATLAS_REFERENCE_DATA="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art"
fi

Events=50
DigiOutFileName="mc23a_presampling.VarBS.CG.RDO.pool.root"
HSHitsFile="${ATLAS_REFERENCE_DATA}/CampaignInputs/mc23/HITS/mc23_13p6TeV.900149.PG_single_nu_Pt50.simul.HITS.e8514_e8528_s4112/10000events.HITS.pool.root"
HighPtMinbiasHitsFiles1="${ATLAS_REFERENCE_DATA}/CampaignInputs/mc23/HITS/mc23_13p6TeV.800831.Py8EG_minbias_inelastic_highjetphotonlepton.merge.HITS.e8514_e8528_s4116_s4120/*"
HighPtMinbiasHitsFiles2="${ATLAS_REFERENCE_DATA}/CampaignInputs/mc23/HITS/mc23_13p6TeV.800831.Py8EG_minbias_inelastic_highjetphotonlepton.merge.HITS.e8514_e8528_s4117_s4120/*"
HighPtMinbiasHitsFiles3="${ATLAS_REFERENCE_DATA}/CampaignInputs/mc23/HITS/mc23_13p6TeV.800831.Py8EG_minbias_inelastic_highjetphotonlepton.merge.HITS.e8514_e8528_s4118_s4120/*"
HighPtMinbiasHitsFiles4="${ATLAS_REFERENCE_DATA}/CampaignInputs/mc23/HITS/mc23_13p6TeV.800831.Py8EG_minbias_inelastic_highjetphotonlepton.merge.HITS.e8514_e8528_s4119_s4120/*"
LowPtMinbiasHitsFiles1="${ATLAS_REFERENCE_DATA}/CampaignInputs/mc23/HITS/mc23_13p6TeV.900311.Epos_minbias_inelastic_lowjetphoton.merge.HITS.e8514_e8528_s4116_s4120/*"
LowPtMinbiasHitsFiles2="${ATLAS_REFERENCE_DATA}/CampaignInputs/mc23/HITS/mc23_13p6TeV.900311.Epos_minbias_inelastic_lowjetphoton.merge.HITS.e8514_e8528_s4117_s4120/*"
LowPtMinbiasHitsFiles3="${ATLAS_REFERENCE_DATA}/CampaignInputs/mc23/HITS/mc23_13p6TeV.900311.Epos_minbias_inelastic_lowjetphoton.merge.HITS.e8514_e8528_s4118_s4120/*"
LowPtMinbiasHitsFiles4="${ATLAS_REFERENCE_DATA}/CampaignInputs/mc23/HITS/mc23_13p6TeV.900311.Epos_minbias_inelastic_lowjetphoton.merge.HITS.e8514_e8528_s4119_s4120/*"

Digi_tf.py \
    --CA \
    --splitConfig 'HITtoRDO:Campaigns.BeamspotSplitMC23a' \
    --PileUpPresampling True \
    --conditionsTag default:OFLCOND-MC23-SDR-RUN3-01 \
    --digiSeedOffset1 170 --digiSeedOffset2 170 \
    --digiSteeringConf 'StandardSignalOnlyTruth' \
    --geometryVersion default:ATLAS-R3S-2021-03-02-00 \
    --inputHITSFile ${HSHitsFile} \
    --inputHighPtMinbiasHitsFile ${HighPtMinbiasHitsFiles1} \
    --inputHighPtMinbiasHitsFile ${HighPtMinbiasHitsFiles2} \
    --inputHighPtMinbiasHitsFile ${HighPtMinbiasHitsFiles3} \
    --inputHighPtMinbiasHitsFile ${HighPtMinbiasHitsFiles4} \
    --inputLowPtMinbiasHitsFile ${LowPtMinbiasHitsFiles1} \
    --inputLowPtMinbiasHitsFile ${LowPtMinbiasHitsFiles2} \
    --inputLowPtMinbiasHitsFile ${LowPtMinbiasHitsFiles3} \
    --inputLowPtMinbiasHitsFile ${LowPtMinbiasHitsFiles4} \
    --jobNumber 568 \
    --maxEvents ${Events} \
    --outputRDOFile ${DigiOutFileName} \
    --postInclude 'PyJobTransforms.UseFrontier' 'HITtoRDO:DigitizationConfig.DigitizationSteering.DigitizationTestingPostInclude' \
    --preInclude 'HITtoRDO:Campaigns.MC23a' \
    --skipEvents 0

rc=$?
status=$rc
echo "art-result: $rc digiCA"

# get reference directory
source DigitizationCheckReferenceLocation.sh
echo "Reference set being used: ${DigitizationTestsVersion}"

rc4=-9999
if [[ $rc -eq 0 ]]
then
    # Do reference comparisons
    art.py compare ref --mode=semi-detailed --no-diff-meta "$DigiOutFileName" "${ATLAS_REFERENCE_DATA}/DigitizationTests/ReferenceFiles/$DigitizationTestsVersion/$CMTCONFIG/$DigiOutFileName"
    rc4=$?
    status=$rc4
fi
echo "art-result: $rc4 OLDvsFixedRef"

rc6=-9999
if [[ $rc -eq 0 ]]
then
    art.py compare grid --entries 10 "$1" "$2" --mode=semi-detailed --file="$DigiOutFileName"
    rc6=$?
    status=$rc6
fi
echo "art-result: $rc6 regression"

exit $status
