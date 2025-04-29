#!/bin/bash
#
# art-description: Run mc23a pile-up presampling
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-athena-mt: 8
# art-include: 24.0/Athena
# art-include: main/Athena
# art-output: mc23a_presampling_SP.RDO.pool.root
# art-output: mc23a_presampling_MP_fork_evt0.RDO.pool.root
# art-output: mc23a_presampling_MP_fork_evt1.RDO.pool.root

if [ -z ${ATLAS_REFERENCE_DATA+x} ]; then
  ATLAS_REFERENCE_DATA="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art"
fi

export ATHENA_CORE_NUMBER=8

Events=25
DigiOutFileNameSP="mc23a_presampling_SP.RDO.pool.root"
DigiOutFileNameMP0="mc23a_presampling_MP_fork_evt0.RDO.pool.root"
DigiOutFileNameMP1="mc23a_presampling_MP_fork_evt1.RDO.pool.root"

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
    --detectors Truth \
    --PileUpPresampling True \
    --conditionsTag default:OFLCOND-MC23-SDR-RUN3-08 \
    --digiSeedOffset1 170 --digiSeedOffset2 170 \
    --digiSteeringConf 'StandardSignalOnlyTruth' \
    --geometryVersion default:ATLAS-R3S-2021-03-02-00 \
    --inputHITSFile ${HSHitsFile} \
    --inputHighPtMinbiasHitsFile ${HighPtMinbiasHitsFiles1} \
    --inputLowPtMinbiasHitsFile ${LowPtMinbiasHitsFiles1} \
    --jobNumber 568 \
    --maxEvents ${Events} \
    --outputRDOFile ${DigiOutFileNameSP} \
    --preInclude 'HITtoRDO:Campaigns.MC23a' \
    --postExec 'HITtoRDO:cfg.getService("PileUpEventLoopMgr").AllowSerialAndMPToDiffer=False' \
    --postInclude 'all:PyJobTransforms.UseFrontier' \
    --skipEvents 0

rc=$?
status=$rc
echo "art-result: $rc Digi_tf.py SP"

Digi_tf.py \
    --CA \
    --multiprocess --athenaMPEventsBeforeFork 0 \
    --detectors Truth \
    --PileUpPresampling True \
    --conditionsTag default:OFLCOND-MC23-SDR-RUN3-08 \
    --digiSeedOffset1 170 --digiSeedOffset2 170 \
    --digiSteeringConf 'StandardSignalOnlyTruth' \
    --geometryVersion default:ATLAS-R3S-2021-03-02-00 \
    --inputHITSFile ${HSHitsFile} \
    --inputHighPtMinbiasHitsFile ${HighPtMinbiasHitsFiles1} \
    --inputLowPtMinbiasHitsFile ${LowPtMinbiasHitsFiles1} \
    --jobNumber 568 \
    --maxEvents ${Events} \
    --outputRDOFile ${DigiOutFileNameMP0} \
    --preInclude 'HITtoRDO:Campaigns.MC23a' \
    --postExec 'HITtoRDO:cfg.getService("PileUpEventLoopMgr").AllowSerialAndMPToDiffer=False' \
    --postInclude 'all:PyJobTransforms.UseFrontier' \
    --skipEvents 0

rc2=$?
if [[ $status -eq 0 ]]; then
  status=$rc2
fi
echo "art-result: $rc2 Digi_tf.py MP fork after 0"

Digi_tf.py \
    --CA \
    --multiprocess --athenaMPEventsBeforeFork 1 \
    --detectors Truth \
    --PileUpPresampling True \
    --conditionsTag default:OFLCOND-MC23-SDR-RUN3-08 \
    --digiSeedOffset1 170 --digiSeedOffset2 170 \
    --digiSteeringConf 'StandardSignalOnlyTruth' \
    --geometryVersion default:ATLAS-R3S-2021-03-02-00 \
    --inputHITSFile ${HSHitsFile} \
    --inputHighPtMinbiasHitsFile ${HighPtMinbiasHitsFiles1} \
    --inputLowPtMinbiasHitsFile ${LowPtMinbiasHitsFiles1} \
    --jobNumber 568 \
    --maxEvents ${Events} \
    --outputRDOFile ${DigiOutFileNameMP1} \
    --preInclude 'HITtoRDO:Campaigns.MC23a' \
    --postExec 'HITtoRDO:cfg.getService("PileUpEventLoopMgr").AllowSerialAndMPToDiffer=False' \
    --postInclude 'all:PyJobTransforms.UseFrontier' \
    --skipEvents 0

rc3=$?
if [[ $status -eq 0 ]]; then
  status=$rc3
fi
echo "art-result: $rc3 Digi_tf.py MP fork after 1"

rc4=-9999
if [[ $rc -eq 0 ]] && [[ $rc2 -eq 0 ]]
then
    acmd.py diff-root ${DigiOutFileNameSP} ${DigiOutFileNameMP0} \
        --nan-equal --mode=semi-detailed --error-mode resilient --order-trees \
        --ignore-leaves RecoTimingObj_p1_Bkg_HITStoRDO_timings index_ref
    rc4=$?
    if [[ $status -eq 0 ]]; then
        status=$rc4
    fi
fi
echo "art-result: $rc4 SP vs MP fork after 0"

rc5=-9999
if [[ $rc -eq 0 ]] && [[ $rc3 -eq 0 ]]
then
    acmd.py diff-root ${DigiOutFileNameSP} ${DigiOutFileNameMP1} \
        --nan-equal --mode=semi-detailed --error-mode resilient --order-trees \
        --ignore-leaves RecoTimingObj_p1_Bkg_HITStoRDO_timings index_ref
    rc5=$?
    if [[ $status -eq 0 ]]; then
        status=$rc5
    fi
fi
echo "art-result: $rc5 SP vs MP fork after 1"

exit $status
