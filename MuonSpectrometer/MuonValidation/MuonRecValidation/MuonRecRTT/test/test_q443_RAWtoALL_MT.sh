#!/bin/sh
#
# art-description: run the RAWtoESD transform of plain q443 with different number of threads and compare the outputs
#
# art-type: grid
# art-include: main/Athena
# art-include: 24.0/Athena
# art-athena-mt: 8
# art-output: OUT_ESD.root
# art-output: OUT_ESD_1thread.root
# art-output: OUT_ESD_5thread.root
# art-output: OUT_ESD_8thread.root
# art-output: diff_1_vs_serial.txt
# art-output: diff_5_vs_1.txt
# art-output: diff_8_vs_1.txt
# art-output: log.RAWtoALL_serial
# art-output: log.RAWtoALL_1thread
# art-output: log.RAWtoALL_5thread
# art-output: log.RAWtoALL_8thread
# art-output: log.HITtoRDO_serial
# art-output: log.HITtoRDO_1thread
# art-output: log.HITtoRDO_5thread
# art-output: log.HITtoRDO_8thread
# art-output: log.RDOtoRDOTrigger_serial
# art-output: log.RDOtoRDOTrigger_1thread
# art-output: log.RDOtoRDOTrigger_5thread
# art-output: log.RDOtoRDOTrigger_8thread
# art-output: NSWPRDValAlg.reco.ntuple.root
# art-output: NSWPRDValAlg.reco.dcube.root

# Run each Reco_tf in a seperate directory

mkdir Serial
cd Serial

#####################################################################
Reco_tf.py --CA 'all:True' 'RDOtoRDOTrigger:False' \
           --AMI q443 \
           --conditionsTag 'default:OFLCOND-MC16-SDR-RUN2-12' 'RDOtoRDOTrigger:OFLCOND-MC16-SDR-RUN2-08-02' \
           --postInclude "RAWtoALL:MuonPRDTest.HitValAlgReco.HitValAlgRecoCfg" \
           --imf False \
           --outputESDFile OUT_ESD.root
exit_code=$?
echo  "art-result: ${exit_code} Reco_tf.py"
if [ ${exit_code} -ne 0 ]
then
    exit ${exit_code}
fi
cd ..
mv Serial/log.HITtoRDO log.HITtoRDO_serial
mv Serial/log.RDOtoRDOTrigger log.RDOtoRDOTrigger_serial
mv Serial/log.RAWtoALL log.RAWtoALL_serial
mv Serial/OUT_ESD.root ./
mv Serial/NSWPRDValAlg.reco.ntuple.root ./
#####################################################################

export ATHENA_CORE_NUMBER=1
mkdir 1thread
cd 1thread

#####################################################################
# now run reconstruction with AthenaMT with 1 thread
Reco_tf.py --CA 'all:True' 'RDOtoRDOTrigger:False' \
           --AMI q443 \
           --conditionsTag 'default:OFLCOND-MC16-SDR-RUN2-11' 'RDOtoRDOTrigger:OFLCOND-MC16-SDR-RUN2-08-02' \
           --multithreaded \
           --imf False \
           --outputESDFile OUT_ESD_1thread.root
exit_code=$?
echo  "art-result: ${exit_code} Reco_tf_1thread.py"
if [ ${exit_code} -ne 0 ]
then
    exit ${exit_code}
fi
cd ..
mv 1thread/log.HITtoRDO log.HITtoRDO_1thread
mv 1thread/log.RDOtoRDOTrigger log.RDOtoRDOTrigger_1thread
mv 1thread/log.RAWtoALL log.RAWtoALL_1thread
mv 1thread/OUT_ESD_1thread.root ./
#####################################################################

export ATHENA_CORE_NUMBER=5
mkdir 5thread
cd 5thread

#####################################################################
# now run reconstruction with AthenaMT with 5 threads
Reco_tf.py --CA 'all:True' 'RDOtoRDOTrigger:False' \
           --AMI q443 \
           --conditionsTag 'default:OFLCOND-MC16-SDR-RUN2-11' 'RDOtoRDOTrigger:OFLCOND-MC16-SDR-RUN2-08-02' \
           --imf False \
           --multithreaded \
           --outputESDFile OUT_ESD_5thread.root
exit_code=$?
echo  "art-result: ${exit_code} Reco_tf_5thread.py"
if [ ${exit_code} -ne 0 ]
then
    exit ${exit_code}
fi
cd ..
mv 5thread/log.HITtoRDO log.HITtoRDO_5thread
mv 5thread/log.RDOtoRDOTrigger log.RDOtoRDOTrigger_5thread
mv 5thread/log.RAWtoALL log.RAWtoALL_5thread
mv 5thread/OUT_ESD_5thread.root ./
#####################################################################

export ATHENA_CORE_NUMBER=8
mkdir 8thread
cd 8thread

#####################################################################
# now run reconstruction with AthenaMT with 8 threads
Reco_tf.py --CA 'all:True' 'RDOtoRDOTrigger:False' \
           --AMI q443 \
           --conditionsTag 'default:OFLCOND-MC16-SDR-RUN2-11' 'RDOtoRDOTrigger:OFLCOND-MC16-SDR-RUN2-08-02' \
           --imf False \
           --multithreaded \
           --outputESDFile OUT_ESD_8thread.root
exit_code=$?
echo  "art-result: ${exit_code} Reco_tf_8thread.py"
if [ ${exit_code} -ne 0 ]
then
    exit ${exit_code}
fi
cd ..
mv 8thread/log.HITtoRDO log.HITtoRDO_8thread
mv 8thread/log.RDOtoRDOTrigger log.RDOtoRDOTrigger_8thread
mv 8thread/log.RAWtoALL log.RAWtoALL_8thread
mv 8thread/OUT_ESD_8thread.root ./
#####################################################################

#####################################################################
# create histograms for dcube
python $Athena_DIR/bin/createDCubeRecoHistograms_withSel.py
exit_code=$?
echo  "art-result: ${exit_code} DCubeRecoHist"
if [ ${exit_code} -ne 0 ]
then
    exit ${exit_code}
fi
#####################################################################

#####################################################################
# now run diff-root to compare the ESDs made with serial and 1thread
acmd.py diff-root  --nan-equal \
                   --ignore-leaves InDet::PixelClusterContainer_p3_PixelClusters \
                                  HLT::HLTResult_p1_HLTResult_HLT.m_navigationResult  \
                                  xAOD::BTaggingAuxContainer_v1_BTagging_AntiKt4EMTopoAuxDyn \
                                  xAOD::TrigDecisionAuxInfo_v1_xTrigDecisionAux \
                                  xAOD::TrigPassBitsAuxContainer_v1_HLT_xAOD__TrigPassBitsContainer_passbitsAux \
                                  xAOD::TrigNavigationAuxInfo_v1_TrigNavigationAux \
                                  InDet::SCT_ClusterContainer_p3_SCT_Clusters \
                                  xAOD::BTaggingAuxContainer_v1_HLT_BTaggingAuxDyn \
                                  xAOD::JetAuxContainer_v1_AntiKt4TruthJetsAuxDyn \
                                  xAOD::BTaggingAuxContainer_v1_HLT_BTaggingAuxDyn \
                                  xAOD::JetAuxContainer_v1_AntiKt4EMTopoJetsAuxDyn \
                                  xAOD::JetAuxContainer_v1_AntiKt4EMPFlowJetsAuxDyn \
                                  RecoTimingObj_p1_HITStoRDO_timings \
                                  RecoTimingObj_p1_RAWtoESD_mems \
                                  RecoTimingObj_p1_RAWtoESD_timings \
                                  index_ref \
                  --order-trees OUT_ESD_5thread.root OUT_ESD.root &> diff_1_vs_serial.txt
exit_code=$?
echo  "art-result: ${exit_code} diff-root"
if [ ${exit_code} -ne 0 ]
then
    exit ${exit_code}
fi
#####################################################################
# now run diff-root to compare the ESDs made with 5threads and 1thread
acmd.py diff-root  --nan-equal \
                   --ignore-leaves InDet::PixelClusterContainer_p3_PixelClusters \
                                  HLT::HLTResult_p1_HLTResult_HLT.m_navigationResult  \
                                  xAOD::BTaggingAuxContainer_v1_BTagging_AntiKt4EMTopoAuxDyn \
                                  xAOD::TrigDecisionAuxInfo_v1_xTrigDecisionAux \
                                  xAOD::TrigPassBitsAuxContainer_v1_HLT_xAOD__TrigPassBitsContainer_passbitsAux \
                                  xAOD::TrigNavigationAuxInfo_v1_TrigNavigationAux \
                                  InDet::SCT_ClusterContainer_p3_SCT_Clusters \
                                  xAOD::BTaggingAuxContainer_v1_HLT_BTaggingAuxDyn \
                                  xAOD::JetAuxContainer_v1_AntiKt4TruthJetsAuxDyn \
                                  xAOD::BTaggingAuxContainer_v1_HLT_BTaggingAuxDyn \
                                  xAOD::JetAuxContainer_v1_AntiKt4EMTopoJetsAuxDyn \
                                  xAOD::JetAuxContainer_v1_AntiKt4EMPFlowJetsAuxDyn \
                                  RecoTimingObj_p1_HITStoRDO_timings \
                                  RecoTimingObj_p1_RAWtoESD_mems \
                                  RecoTimingObj_p1_RAWtoESD_timings \
                                  index_ref \
                    --order-trees OUT_ESD_5thread.root OUT_ESD_1thread.root &> diff_5_vs_1.txt
exit_code=$?
echo  "art-result: ${exit_code} diff-root_5thread"
if [ ${exit_code} -ne 0 ]
then
    exit ${exit_code}
fi
#####################################################################
# now run diff-root to compare the ESDs made with 8threads and 1thread
acmd.py diff-root  --nan-equal \
                   --ignore-leaves InDet::PixelClusterContainer_p3_PixelClusters \
                                  HLT::HLTResult_p1_HLTResult_HLT.m_navigationResult  \
                                  xAOD::BTaggingAuxContainer_v1_BTagging_AntiKt4EMTopoAuxDyn \
                                  xAOD::TrigDecisionAuxInfo_v1_xTrigDecisionAux \
                                  xAOD::TrigPassBitsAuxContainer_v1_HLT_xAOD__TrigPassBitsContainer_passbitsAux \
                                  xAOD::TrigNavigationAuxInfo_v1_TrigNavigationAux \
                                  InDet::SCT_ClusterContainer_p3_SCT_Clusters \
                                  xAOD::BTaggingAuxContainer_v1_HLT_BTaggingAuxDyn \
                                  xAOD::JetAuxContainer_v1_AntiKt4TruthJetsAuxDyn \
                                  xAOD::BTaggingAuxContainer_v1_HLT_BTaggingAuxDyn \
                                  xAOD::JetAuxContainer_v1_AntiKt4EMTopoJetsAuxDyn \
                                  xAOD::JetAuxContainer_v1_AntiKt4EMPFlowJetsAuxDyn \
                                  RecoTimingObj_p1_HITStoRDO_timings \
                                  RecoTimingObj_p1_RAWtoESD_mems \
                                  RecoTimingObj_p1_RAWtoESD_timings \
                                  index_ref \
                   --order-trees OUT_ESD_8thread.root OUT_ESD_1thread.root &> diff_8_vs_1.txt
exit_code=$?
echo  "art-result: ${exit_code} diff-root_8thread"
if [ ${exit_code} -ne 0 ]
then
    exit ${exit_code}
fi
#####################################################################

echo "art-result: $?"
