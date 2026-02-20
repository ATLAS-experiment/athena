#!/bin/bash
#
# art-description: Run a digitization example of mc23d presampling with sim&digi parameters written directly to in-file metadata
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-memory: 4096
# art-athena-mt: 8
# art-include: 24.0/Athena
# art-include: main/Athena
# art-output: mc23d_presampling.VarBS.RDO.pool.root
# art-output: log.*
# art-output: DigiPUConfig*

if [ -z ${ATLAS_REFERENCE_DATA+x} ]; then
  ATLAS_REFERENCE_DATA="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art"
fi

Events=100
DigiOutFileName="mc23d_presampling.VarBS.RDO.pool.root"
HSHitsFile="${ATLAS_REFERENCE_DATA}/CampaignInputs/mc23/HITS/mc23_13p6TeV.900149.PG_single_nu_Pt50.simul.HITS.e8514_e8528_s4153/10000events.HITS.pool.root"
HighPtMinbiasHitsFiles1="${ATLAS_REFERENCE_DATA}/CampaignInputs/mc23/HITS/mc23_13p6TeV.800831.Py8EG_minbias_inelastic_highjetphotonlepton.merge.HITS.e8514_e8528_s4154_s4120/*"
HighPtMinbiasHitsFiles2="${ATLAS_REFERENCE_DATA}/CampaignInputs/mc23/HITS/mc23_13p6TeV.800831.Py8EG_minbias_inelastic_highjetphotonlepton.merge.HITS.e8514_e8528_s4155_s4120/*"
HighPtMinbiasHitsFiles3="${ATLAS_REFERENCE_DATA}/CampaignInputs/mc23/HITS/mc23_13p6TeV.800831.Py8EG_minbias_inelastic_highjetphotonlepton.merge.HITS.e8514_e8528_s4156_s4120/*"
HighPtMinbiasHitsFiles4="${ATLAS_REFERENCE_DATA}/CampaignInputs/mc23/HITS/mc23_13p6TeV.800831.Py8EG_minbias_inelastic_highjetphotonlepton.merge.HITS.e8514_e8528_s4157_s4120/*"
LowPtMinbiasHitsFiles1="${ATLAS_REFERENCE_DATA}/CampaignInputs/mc23/HITS/mc23_13p6TeV.900311.Epos_minbias_inelastic_lowjetphoton.merge.HITS.e8514_e8528_s4154_s4120/*"
LowPtMinbiasHitsFiles2="${ATLAS_REFERENCE_DATA}/CampaignInputs/mc23/HITS/mc23_13p6TeV.900311.Epos_minbias_inelastic_lowjetphoton.merge.HITS.e8514_e8528_s4155_s4120/*"
LowPtMinbiasHitsFiles3="${ATLAS_REFERENCE_DATA}/CampaignInputs/mc23/HITS/mc23_13p6TeV.900311.Epos_minbias_inelastic_lowjetphoton.merge.HITS.e8514_e8528_s4156_s4120/*"
LowPtMinbiasHitsFiles4="${ATLAS_REFERENCE_DATA}/CampaignInputs/mc23/HITS/mc23_13p6TeV.900311.Epos_minbias_inelastic_lowjetphoton.merge.HITS.e8514_e8528_s4157_s4120/*"
geometry=$(python -c "from AthenaConfiguration.TestDefaults import defaultGeometryTags; print(defaultGeometryTags.RUN3)")
conditions=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN3_MC)")
Digi_tf.py \
     --inputHITSFile ${HSHitsFile} \
     --outputRDOFile ${DigiOutFileName} \
     --inputHighPtMinbiasHitsFile ${HighPtMinbiasHitsFiles1} \
     --inputHighPtMinbiasHitsFile ${HighPtMinbiasHitsFiles2} \
     --inputHighPtMinbiasHitsFile ${HighPtMinbiasHitsFiles3} \
     --inputHighPtMinbiasHitsFile ${HighPtMinbiasHitsFiles4} \
     --inputLowPtMinbiasHitsFile ${LowPtMinbiasHitsFiles1} \
     --inputLowPtMinbiasHitsFile ${LowPtMinbiasHitsFiles2} \
     --inputLowPtMinbiasHitsFile ${LowPtMinbiasHitsFiles3} \
     --inputLowPtMinbiasHitsFile ${LowPtMinbiasHitsFiles4} \
     --multiprocess \
     --PileUpPresampling True \
     --conditionsTag "default:${conditions}" \
     --geometryVersion "default:${geometry}" \
     --digiSeedOffset1 170 --digiSeedOffset2 170 \
     --digiSteeringConf 'StandardSignalOnlyTruth' \
     --postInclude 'all:PyJobTransforms.UseFrontier' \
     --preInclude 'HITtoRDO:Campaigns.MC23d' \
     --preExec 'HITtoRDO:flags.IOVDb.WriteParametersAsMetaData=True' \
     --splitConfig "HITtoRDO:Campaigns.BeamspotSplitMC23d" \
     --skipEvents 0 \
     --jobNumber 568 \
     --maxEvents ${Events} \
     --AMITag d1907 \
     --skipEvents 0

rc=$?
status=$rc
echo "art-result: $rc digiCA"

# Initially we are just interested in whether the job still runs

exit $status
