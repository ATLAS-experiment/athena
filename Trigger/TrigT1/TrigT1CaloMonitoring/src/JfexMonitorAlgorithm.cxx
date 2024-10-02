/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "JfexMonitorAlgorithm.h"
#include "TMath.h"
#include "JfexMapForwardEmptyBins.h"

#include "CxxUtils/checker_macros.h"
ATLAS_NO_CHECK_FILE_THREAD_SAFETY;

JfexMonitorAlgorithm::JfexMonitorAlgorithm( const std::string& name, ISvcLocator* pSvcLocator )
  : AthMonitorAlgorithm(name,pSvcLocator)
{
}

StatusCode JfexMonitorAlgorithm::initialize() {

    ATH_MSG_DEBUG("JfexMonitorAlgorith::initialize");
    ATH_MSG_DEBUG("Package Name "<< m_Grouphist);

    ATH_MSG_DEBUG("m_jFexLRJetContainerKey"<< m_jFexLRJetContainerKey);
    ATH_MSG_DEBUG("m_jFexSRJetContainerKey"<< m_jFexSRJetContainerKey);
    ATH_MSG_DEBUG("m_jFexTauContainerKey"  << m_jFexTauContainerKey);
    ATH_MSG_DEBUG("m_jFexFwdElContainerKey"<< m_jFexFwdElContainerKey);
    ATH_MSG_DEBUG("m_jFexMETContainerKey"  << m_jFexMETContainerKey);
    ATH_MSG_DEBUG("m_jFexSumEtContainerKey"<< m_jFexSumEtContainerKey);

    ATH_CHECK( m_jFexLRJetContainerKey.initialize() );
    ATH_CHECK( m_jFexSRJetContainerKey.initialize() );
    ATH_CHECK( m_jFexTauContainerKey.initialize()   );
    ATH_CHECK( m_jFexFwdElContainerKey.initialize() );
    ATH_CHECK( m_jFexMETContainerKey.initialize()   );
    ATH_CHECK( m_jFexSumEtContainerKey.initialize() );

    // TOBs may come from trigger bytestream - renounce from scheduler
    renounce(m_jFexLRJetContainerKey );
    renounce(m_jFexSRJetContainerKey );
    renounce(m_jFexTauContainerKey   );
    renounce(m_jFexFwdElContainerKey );
    renounce(m_jFexMETContainerKey   );
    renounce(m_jFexSumEtContainerKey );

    return AthMonitorAlgorithm::initialize();
}

StatusCode JfexMonitorAlgorithm::fillHistograms( const EventContext& ctx ) const {

    ATH_MSG_DEBUG("JfexMonitorAlgorithm::fillHistograms");

    bool jJ_isInValid   = false;
    bool jLJ_isInValid  = false;
    bool jTau_isInValid = false;
    bool jEM_isInValid  = false;
    bool jXE_isInValid  = false;
    bool jTE_isInValid  = false;

    SG::ReadHandle<xAOD::jFexSRJetRoIContainer> jFexSRJetContainer{m_jFexSRJetContainerKey, ctx};
    if(!jFexSRJetContainer.isValid()) {
        ATH_MSG_WARNING("No jFex SR Jet container found in storegate "<< m_jFexSRJetContainerKey<<". Will be skipped!");
        jJ_isInValid = true;
    }
    SG::ReadHandle<xAOD::jFexLRJetRoIContainer> jFexLRJetContainer{m_jFexLRJetContainerKey, ctx};
    if(!jFexLRJetContainer.isValid()) {
        ATH_MSG_WARNING("No jFex LR Jet container found in storegate "<< m_jFexLRJetContainerKey<<". Will be skipped!");
        jLJ_isInValid = true;
    }
    SG::ReadHandle<xAOD::jFexTauRoIContainer> jFexTauContainer{m_jFexTauContainerKey, ctx};
    if(!jFexTauContainer.isValid()) {
        ATH_MSG_WARNING("No jFex Tau container found in storegate "<< m_jFexTauContainerKey<<". Will be skipped!");
        jTau_isInValid = true;
    }
    SG::ReadHandle<xAOD::jFexFwdElRoIContainer> jFexEMContainer{m_jFexFwdElContainerKey, ctx};
    if(!jFexEMContainer.isValid()) {
        ATH_MSG_WARNING("No jFex EM container found in storegate "<< m_jFexFwdElContainerKey<<". Will be skipped!");
        jEM_isInValid =  true;
    }
    SG::ReadHandle<xAOD::jFexMETRoIContainer> jFexMETContainer{m_jFexMETContainerKey, ctx};
    if(!jFexMETContainer.isValid()) {
        ATH_MSG_WARNING("No jFex MET container found in storegate "<< m_jFexMETContainerKey<<". Will be skipped!");
        jXE_isInValid = true;
    }
    SG::ReadHandle<xAOD::jFexSumETRoIContainer> jFexSumETContainer{m_jFexSumEtContainerKey, ctx};
    if(!jFexSumETContainer.isValid()) {
        ATH_MSG_WARNING("No jFex MET container found in storegate "<< m_jFexSumEtContainerKey<<". Will be skipped!");
        jTE_isInValid = true;
    }

    // variables for histograms
    // jJ
    auto jFexSRJetModule  = Monitored::Scalar<int>  ("jJ_jFexNumber",0);
    auto jFexSRJetFPGA    = Monitored::Scalar<int>  ("jJ_fpgaNumber",0);
    auto jFexSRJetEt      = Monitored::Scalar<int>  ("jJ_Et",0);
    auto jFexSRJeteta     = Monitored::Scalar<float>("jJ_Eta",0.0);
    auto jFexSRJetphi     = Monitored::Scalar<float>("jJ_Phi",0.0);
    auto jFexSRJeteta_glo = Monitored::Scalar<float>("jJ_GlobalEta",0.0);
    auto jFexSRJetphi_glo = Monitored::Scalar<float>("jJ_GlobalPhi",0.0);

    // jLJ
    auto jFexLRJetModule  = Monitored::Scalar<int>  ("jLJ_jFexNumber",0);
    auto jFexLRJetFPGA    = Monitored::Scalar<int>  ("jLJ_fpgaNumber",0);
    auto jFexLRJetEt      = Monitored::Scalar<int>  ("jLJ_Et",0);
    auto jFexLRJeteta     = Monitored::Scalar<float>("jLJ_Eta",0.0);
    auto jFexLRJetphi     = Monitored::Scalar<float>("jLJ_Phi",0.0);
    auto jFexLRJeteta_glo = Monitored::Scalar<float>("jLJ_GlobalEta",0.0);
    auto jFexLRJetphi_glo = Monitored::Scalar<float>("jLJ_GlobalPhi",0.0);


    // jTau
    auto jFexTauModule  = Monitored::Scalar<int>  ("jTau_jFexNumber",0);
    auto jFexTauFPGA    = Monitored::Scalar<int>  ("jTau_fpgaNumber",0);
    auto jFexTauEt      = Monitored::Scalar<int>  ("jTau_Et",0);
    auto jFexTauIso     = Monitored::Scalar<int>  ("jTau_Iso",0);
    auto jFexTaueta     = Monitored::Scalar<float>("jTau_Eta",0.0);
    auto jFexTauphi     = Monitored::Scalar<float>("jTau_Phi",0.0);
    auto jFexTaueta_glo = Monitored::Scalar<float>("jTau_GlobalEta",0.0);
    auto jFexTauphi_glo = Monitored::Scalar<float>("jTau_GlobalPhi",0.0);

    // jEM
    auto jFexEMModule  = Monitored::Scalar<int>  ("jEM_jFexNumber",0);
    auto jFexEMFPGA    = Monitored::Scalar<int>  ("jEM_fpgaNumber",0);
    auto jFexEMEt      = Monitored::Scalar<int>  ("jEM_Et",0);
    auto jFexEMeta     = Monitored::Scalar<float>("jEM_Eta",0.0);
    auto jFexEMphi     = Monitored::Scalar<float>("jEM_Phi",0.0);
    auto jFexEMeta_glo = Monitored::Scalar<float>("jEM_GlobalEta",0.0);
    auto jFexEMphi_glo = Monitored::Scalar<float>("jEM_GlobalPhi",0.0);
    auto jFexEMIso     = Monitored::Scalar<int>  ("jEM_Iso",0);
    auto jFexEMf1      = Monitored::Scalar<int>  ("jEM_f1",0);
    auto jFexEMf2      = Monitored::Scalar<int>  ("jEM_f2",0);

    // jXE
    auto jFexMETX   = Monitored::Scalar<int>  ("jXE_X",0);
    auto jFexMETY   = Monitored::Scalar<int>  ("jXE_Y",0);
    auto jFexMET    = Monitored::Scalar<float>("jXE_MET",0.0);
    auto jFexMETphi = Monitored::Scalar<float>("jXE_phi",0.0);

    // jTE
    auto jFexSumEt_low   = Monitored::Scalar<int>  ("jTE_low",0);
    auto jFexSumEt_high  = Monitored::Scalar<int>  ("jTE_high",0);
    auto jFexSumEt_total = Monitored::Scalar<float>("jTE_SumEt",0.0);

    auto weight = Monitored::Scalar<float>("weight",1);

    // write -1 into bins in maps that are always empty
    {
        std::scoped_lock lock(m_mutex);
        if (m_firstEvent) {
            weight = -1;
            // empty bins due to irregular structure of FCAL
            for (auto& [eta, phi] : jFEXMapEmptyBinCenters) {
                jFexSRJeteta = eta;
                jFexSRJetphi = phi;
                jFexEMeta = eta;
                jFexEMphi = phi;
                fill(m_Groupmaps,jFexSRJeteta,jFexSRJetphi,jFexEMeta,jFexEMphi,weight);
                fill(m_GroupmapsHighPt,jFexSRJeteta,jFexSRJetphi,jFexEMeta,jFexEMphi,weight);
            }

            // central region without jEM
            for (int ieta=-23; ieta<23; ieta++){
                jFexEMeta = 0.1 * ieta + 0.05;
                for (int iphi=-32; iphi<33; iphi++){
                    jFexEMphi = M_PI/32 * iphi + M_PI/64;
                    fill(m_Groupmaps,jFexEMeta,jFexEMphi,weight);
                }
            }
            m_firstEvent = false;
            weight = 1;
        }
    }

    if(!jJ_isInValid) {
        for(const xAOD::jFexSRJetRoI* jFexSRJetRoI : *jFexSRJetContainer) {
            if(jFexSRJetRoI->tobWord()==0) continue; //remove empty TOBs
            jFexSRJetModule=jFexSRJetRoI->jFexNumber();
            jFexSRJetFPGA=jFexSRJetRoI->fpgaNumber();
            jFexSRJetEt=jFexSRJetRoI->tobEt();
            float eta = jFexSRJetRoI->eta();
            float phi = jFexSRJetRoI->phi();
            jFexSRJeteta=eta;
            jFexSRJetphi=phi;
            jFexSRJeteta_glo=jFexSRJetRoI->globalEta();
            jFexSRJetphi_glo=jFexSRJetRoI->globalPhi();
            fill(m_Grouphist,jFexSRJetModule,jFexSRJetFPGA,jFexSRJetEt,jFexSRJeteta,jFexSRJetphi,jFexSRJeteta_glo,jFexSRJetphi_glo);
            if (abs(eta) > 2.5 && abs(eta) < 3.1) {
                jFexSRJetphi = phi - M_PI/64;
                fill(m_Groupmaps,jFexSRJeteta,jFexSRJetphi,weight);
                if (jFexSRJetRoI->et() >= 20000)
                    fill(m_GroupmapsHighPt,jFexSRJeteta,jFexSRJetphi,weight);
                jFexSRJetphi = phi + M_PI/64;
                fill(m_Groupmaps,jFexSRJeteta,jFexSRJetphi,weight);
                if (jFexSRJetRoI->et() >= 20000)
                    fill(m_GroupmapsHighPt,jFexSRJeteta,jFexSRJetphi,weight);
            } else if (abs(eta) > 3.1 && abs(eta) < 3.2) {
                uint8_t localEta = jFexSRJetRoI->tobLocalEta();
                bool isFCAL = localEta >= 13;
                if (isFCAL) {
                    jFexSRJeteta = eta > 0 ? 3.175 : -3.175;
                    jFexSRJetphi = phi;
                    fill(m_Groupmaps,jFexSRJeteta,jFexSRJetphi,weight);
                    if (jFexSRJetRoI->et() >= 20000)
                        fill(m_GroupmapsHighPt,jFexSRJeteta,jFexSRJetphi,weight);
                } else {
                    jFexSRJeteta = eta > 0 ? 3.125 : -3.125;
                    jFexSRJetphi = phi - M_PI/64;
                    fill(m_Groupmaps,jFexSRJeteta,jFexSRJetphi,weight);
                    if (jFexSRJetRoI->et() >= 20000)
                        fill(m_GroupmapsHighPt,jFexSRJeteta,jFexSRJetphi,weight);
                    jFexSRJetphi = phi + M_PI/64;
                    fill(m_Groupmaps,jFexSRJeteta,jFexSRJetphi,weight);
                    if (jFexSRJetRoI->et() >= 20000)
                        fill(m_GroupmapsHighPt,jFexSRJeteta,jFexSRJetphi,weight);
                }
            } else {
                fill(m_Groupmaps,jFexSRJeteta,jFexSRJetphi,weight);
                if (jFexSRJetRoI->et() >= 20000)
                    fill(m_GroupmapsHighPt,jFexSRJeteta,jFexSRJetphi,weight);
            }
        }
    }

    if(!jLJ_isInValid) {
        for(const xAOD::jFexLRJetRoI* jFexLRJetRoI : *jFexLRJetContainer) {
            if(jFexLRJetRoI->tobWord()==0) continue; //remove empty TOBs
            jFexLRJetModule=jFexLRJetRoI->jFexNumber();
            jFexLRJetFPGA=jFexLRJetRoI->fpgaNumber();
            jFexLRJetEt=jFexLRJetRoI->tobEt();
            jFexLRJeteta=jFexLRJetRoI->eta();
            jFexLRJetphi=jFexLRJetRoI->phi();
            jFexLRJeteta_glo=jFexLRJetRoI->globalEta();
            jFexLRJetphi_glo=jFexLRJetRoI->globalPhi();
            fill(m_Grouphist,jFexLRJetModule,jFexLRJetFPGA,jFexLRJetEt,jFexLRJeteta,jFexLRJetphi,jFexLRJeteta_glo,jFexLRJetphi_glo);
        }
    }

    if(!jTau_isInValid) {
        for(const xAOD::jFexTauRoI* jFexTauRoI : *jFexTauContainer) {
            if(jFexTauRoI->tobWord()==0) continue; //remove empty TOBs
            jFexTauModule =jFexTauRoI->jFexNumber();
            jFexTauFPGA =jFexTauRoI->fpgaNumber();
            jFexTauEt =jFexTauRoI->tobEt();
            jFexTauIso=jFexTauRoI->tobIso();
            jFexTaueta=jFexTauRoI->eta();
            jFexTauphi=jFexTauRoI->phi();
            jFexTaueta_glo=jFexTauRoI->globalEta();
            jFexTauphi_glo=jFexTauRoI->globalPhi();
            fill(m_Grouphist,jFexTauModule,jFexTauFPGA,jFexTauEt,jFexTauIso,jFexTaueta,jFexTauphi,jFexTaueta_glo,jFexTauphi_glo);
            if (jFexTauRoI->et() >= 10000)
                fill(m_GroupmapsHighPt,jFexTaueta,jFexTauphi);
        }
    }

    if(!jEM_isInValid){
        for(const xAOD::jFexFwdElRoI* jFexFwdElRoI : *jFexEMContainer) {
            if(jFexFwdElRoI->tobWord()==0) continue; //remove empty TOBs
            jFexEMModule =jFexFwdElRoI->jFexNumber();
            jFexEMFPGA =jFexFwdElRoI->fpgaNumber();
            jFexEMEt =jFexFwdElRoI->tobEt();
            float eta = jFexFwdElRoI->eta();
            float phi = jFexFwdElRoI->phi();
            jFexEMeta=eta;
            jFexEMphi=phi;
            jFexEMeta_glo=jFexFwdElRoI->globalEta();
            jFexEMphi_glo=jFexFwdElRoI->globalPhi();
            jFexEMIso=jFexFwdElRoI->tobEMIso();
            jFexEMf1 =jFexFwdElRoI->tobEMf1();
            jFexEMf2 =jFexFwdElRoI->tobEMf2();
            fill(m_Grouphist,jFexEMModule,jFexEMFPGA,jFexEMEt,jFexEMeta,jFexEMphi,jFexEMeta_glo,jFexEMphi_glo,jFexEMIso,jFexEMf1,jFexEMf2);
            if (abs(eta) > 2.5 && abs(eta) < 3.2) {
                jFexEMphi = phi - M_PI/64;
                fill(m_Groupmaps,jFexEMeta,jFexEMphi,weight);
                if (jFexFwdElRoI->et() >= 10000)
                    fill(m_GroupmapsHighPt,jFexEMeta,jFexEMphi,weight);
                jFexEMphi = phi + M_PI/64;
                fill(m_Groupmaps,jFexEMeta,jFexEMphi,weight);
                if (jFexFwdElRoI->et() >= 10000)
                    fill(m_GroupmapsHighPt,jFexEMeta,jFexEMphi,weight);
            } else {
                fill(m_Groupmaps,jFexEMeta,jFexEMphi,weight);
                if (jFexFwdElRoI->et() >= 10000)
                    fill(m_GroupmapsHighPt,jFexEMeta,jFexEMphi,weight);
            }
        }    
    }

    if(!jXE_isInValid){
        float metx = 0;
        float mety = 0;
        for(const xAOD::jFexMETRoI* jFexMETRoI : *jFexMETContainer) {
            jFexMETX =jFexMETRoI->tobEx();
            jFexMETY =jFexMETRoI->tobEy();
            if(jFexMETRoI->tobEtScale() != 0) {
                metx += jFexMETRoI->Ex()/jFexMETRoI->tobEtScale();
                mety += jFexMETRoI->Ey()/jFexMETRoI->tobEtScale();
            }

            fill(m_Grouphist,jFexMETX,jFexMETY);
        }
        if(jFexMETContainer->size()>0) {
            jFexMET    = TMath::Sqrt(std::pow(metx,2)+std::pow(mety,2));
            jFexMETphi = TMath::ATan2(mety,metx);
            fill(m_Grouphist,jFexMET,jFexMETphi);
        }    
    }

    if(!jTE_isInValid){
        int sumEt_total = 0;
        for(const xAOD::jFexSumETRoI* jFexSumETRoI : *jFexSumETContainer) {
            jFexSumEt_low =jFexSumETRoI->tobEt_lower();
            jFexSumEt_high =jFexSumETRoI->tobEt_upper();
            sumEt_total += jFexSumETRoI->tobEt_lower()+jFexSumETRoI->tobEt_upper();
            fill(m_Grouphist,jFexSumEt_low,jFexSumEt_high);
        }
        if(jFexSumETContainer->size()>0) {
            jFexSumEt_total = sumEt_total;
            fill(m_Grouphist,jFexSumEt_total);
        }    
    }



    return StatusCode::SUCCESS;
}
