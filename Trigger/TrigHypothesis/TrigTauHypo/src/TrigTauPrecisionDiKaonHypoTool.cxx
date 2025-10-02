/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/********************************************************************
 *
 * NAME:     TrigTauPrecisionDiKaonHypoTool.cxx
 * PACKAGE:  Trigger/TrigHypothesis/TrigTauHypo
 *
 * AUTHOR:   J. Silva based on TrigTauPrecisionIDHypoTool
 * CREATED:  Feb. 17, 2021
 *
  *********************************************************************/

#include "AthenaMonitoringKernel/Monitored.h"
#include "TrigCompositeUtils/TrigCompositeUtils.h"
#include "xAODTau/TauJetContainer.h"
#include "xAODTracking/TrackParticle.h"
#include "GaudiKernel/SystemOfUnits.h"
#include "TruthUtils/ParticleConstants.h"

#include "TrigTauPrecisionDiKaonHypoTool.h"


using namespace TrigCompositeUtils;

TrigTauPrecisionDiKaonHypoTool::TrigTauPrecisionDiKaonHypoTool(const std::string& type,
					                                           const std::string& name,
					                                           const IInterface* parent)
    : base_class(type, name, parent), m_decisionId(HLT::Identifier::fromToolName(name))
{

}


StatusCode TrigTauPrecisionDiKaonHypoTool::initialize()
{
    ATH_MSG_DEBUG(name() << ": in initialize()");

    ATH_MSG_DEBUG("TrigTauPrecisionDiKaonHypoTool will cut on:");
    ATH_MSG_DEBUG(" - PtMin: " << m_ptMin.value());
    ATH_MSG_DEBUG(" - NTracksMin: " << m_numTrackMin.value());
    ATH_MSG_DEBUG(" - NTracksMax: " << m_numTrackMax.value());
    ATH_MSG_DEBUG(" - NIsoTracksMax: " << m_numIsoTrackMax.value());

    ATH_MSG_DEBUG(" - massTrkSysMin: " << m_massTrkSysMin.value());
    ATH_MSG_DEBUG(" - massTrkSysMax: " << m_massTrkSysMax.value());
    ATH_MSG_DEBUG(" - massTrkSysKaonMin: " << m_massTrkSysKaonMin.value());
    ATH_MSG_DEBUG(" - massTrkSysKaonMax: " << m_massTrkSysKaonMax.value());
    ATH_MSG_DEBUG(" - massTrkSysKaonPiMin: " << m_massTrkSysKaonPiMin.value());
    ATH_MSG_DEBUG(" - massTrkSysKaonPiMax: " << m_massTrkSysKaonPiMax.value());
    ATH_MSG_DEBUG(" - targetMassTrkSysKaonPi: " << m_targetMassTrkSysKaonPi.value());
    ATH_MSG_DEBUG(" - leadTrkPtMin: " << m_leadTrkPtMin.value());
    ATH_MSG_DEBUG(" - dRmaxMax: " << m_dRmaxMax.value());
    ATH_MSG_DEBUG(" - etOverPtLeadTrkMin: " << m_etOverPtLeadTrkMin.value());
    ATH_MSG_DEBUG(" - etOverPtLeadTrkMax: " << m_etOverPtLeadTrkMax.value());
    ATH_MSG_DEBUG(" - EMPOverTrkSysPMax: " << m_EMPOverTrkSysPMax.value());

    if(m_massTrkSysMin > m_massTrkSysMax || m_numTrackMin > m_numTrackMax 
        || m_massTrkSysKaonPiMin > m_massTrkSysKaonPiMax || m_massTrkSysKaonMin > m_massTrkSysKaonMax
        || m_etOverPtLeadTrkMin > m_etOverPtLeadTrkMax) {
        ATH_MSG_ERROR("Invalid tool configuration!");
        return StatusCode::FAILURE;
    }
  
    return StatusCode::SUCCESS;
}


bool TrigTauPrecisionDiKaonHypoTool::decide(const ITrigTauPrecisionHypoTool::ToolInfo& input) const
{
    ATH_MSG_DEBUG(name() << ": in execute()");

    auto NInputTaus          = Monitored::Scalar<int>("NInputTaus", -1);
    auto passedCuts          = Monitored::Scalar<int>("CutCounter", 0);
    auto PtAccepted          = Monitored::Scalar<float>("PtAccepted", -1);
    auto NTracksAccepted     = Monitored::Scalar<int>("NTracksAccepted", -1);
    auto NIsoTracksAccepted  = Monitored::Scalar<int>("NIsoTracksAccepted", -1);
    auto massTrkSysAccepted         = Monitored::Scalar<float>("massTrkSysAccepted", -10);
    auto massTrkSysKaonAccepted     = Monitored::Scalar<float>("massTrkSysKaonAccepted", -10);
    auto massTrkSysKaonPiAccepted   = Monitored::Scalar<float>("massTrkSysKaonPiAccepted", -10);
    auto leadTrkPtAccepted          = Monitored::Scalar<float>("leadTrkPtAccepted", -10);
    auto dRAccepted                 = Monitored::Scalar<float>("dRAccepted", -1);
    auto etOverPtLeadTrkAccepted    = Monitored::Scalar<float>("etOverPtLeadTrkAccepted", -10);
    auto EMOverTrkSysPAccepted      = Monitored::Scalar<float>("EMOverTrkSysPAccepted", -10);

    auto monitorIt = Monitored::Group(m_monTool,
                                        NInputTaus, passedCuts,
                                        PtAccepted,  NTracksAccepted, NIsoTracksAccepted,
                                        massTrkSysAccepted, massTrkSysKaonAccepted, massTrkSysKaonPiAccepted,
                                        leadTrkPtAccepted, dRAccepted, etOverPtLeadTrkAccepted, EMOverTrkSysPAccepted);


    // Tau pass flag
    bool pass = false;

    if(m_acceptAll) {
        pass = true;
        ATH_MSG_DEBUG("AcceptAll property is set: taking all events");
    }

    // Debugging location of the TauJet RoI
    ATH_MSG_DEBUG("Input RoI eta: " << input.roi->eta() << ", phi: " << input.roi->phi() << ", z: " << input.roi->zed());

    const xAOD::TauJetContainer* TauContainer = input.tauContainer;
    NInputTaus = TauContainer->size();
    // There should only be a single TauJet in the TauJetContainer; just in case we still run the loop
    for(const xAOD::TauJet* Tau : *TauContainer) {
        ATH_MSG_DEBUG(" New HLT TauJet candidate:");
        passedCuts++;


        //---------------------------------------------------------------
        // Calibrated tau pT cut ('idperf' step)
        //---------------------------------------------------------------
        float pT = Tau->pt();
        ATH_MSG_DEBUG(" pT: " << pT / Gaudi::Units::GeV);

        if(!(pT > m_ptMin)) continue;
        passedCuts++;
        PtAccepted = pT / Gaudi::Units::GeV;


        //---------------------------------------------------------------
        // Track counting ('perf' step)
        //---------------------------------------------------------------
        int numTrack = Tau->nTracks();
        int numIsoTrack = Tau->nTracksIsolation();

        ATH_MSG_DEBUG(" N Tracks: " << numTrack);
        ATH_MSG_DEBUG(" N Iso Tracks: " << numIsoTrack);

        // Apply NTrackMin/Max and NIsoTrackMax cuts:
        if(!(numTrack >= m_numTrackMin && numTrack <= m_numTrackMax)) continue;
        if(!(numIsoTrack <= m_numIsoTrackMax)) continue;
        passedCuts++;
        NTracksAccepted = numTrack;
        NIsoTracksAccepted = numIsoTrack;

        
        //---------------------------------------------------------------
        // Leading track pT cut
        //---------------------------------------------------------------
        float leadTrkPt = -1;
        Tau->detail(xAOD::TauJetParameters::leadTrkPt, leadTrkPt);
        ATH_MSG_DEBUG(" leadTrkPt: " << leadTrkPt / Gaudi::Units::GeV);
        if(!(leadTrkPt > m_leadTrkPtMin)) continue;
        passedCuts++;
        leadTrkPtAccepted = leadTrkPt / Gaudi::Units::GeV;


        //---------------------------------------------------------------
        // Cuts on the track-system mass, for different meson hypothesis
        //---------------------------------------------------------------
        float massTrkSys = -1.;
        Tau->detail(xAOD::TauJetParameters::massTrkSys, massTrkSys);
        ATH_MSG_DEBUG(" massTrkSys: " << massTrkSys / Gaudi::Units::GeV);
        
        // For the dikaon mass hypothesis, compute the invariant mass with the kaon mass
        // Also cache the p4 of tracks, to use for other mass hypothesis
        TLorentzVector my_kaons;
        std::vector<TLorentzVector> my_trks;
        for(unsigned int i = 0; i < Tau->nTracks(); ++i) {
            const xAOD::TrackParticle* trk = nullptr;
            TLorentzVector tmpKaon;;

            try {
                trk = Tau->track(i)->track();
            } catch(const std::exception& e) {
                ATH_MSG_WARNING(" Failed to get tau track link!");
            } 

            if(trk) {
                tmpKaon.SetPtEtaPhiM(trk->pt(), trk->eta(), trk->phi(), ParticleConstants::chargedKaonMassInMeV);
                my_trks.push_back(trk->p4());
            }

            my_kaons = my_kaons + tmpKaon;
        }

        float massTrkSysKaon = my_kaons.M();
        ATH_MSG_DEBUG(" massTrkSys with kaon mass hypo: " << massTrkSysKaon / Gaudi::Units::GeV);
      
        // kaon+pi mass hypo
        float finalKPiMass = 0;
        if(my_trks.size() == 2) {
            TLorentzVector tmpKaon;

            tmpKaon.SetPtEtaPhiM(my_trks.at(0).Pt(), my_trks.at(0).Eta(), my_trks.at(0).Phi(), ParticleConstants::chargedKaonMassInMeV);
            TLorentzVector tmpPion = my_trks.at(1);
            float kPiMass1 = (tmpKaon+tmpPion).M();

            tmpKaon.SetPtEtaPhiM(my_trks.at(1).Pt(), my_trks.at(1).Eta(), my_trks.at(1).Phi(), ParticleConstants::chargedKaonMassInMeV);
            tmpPion = my_trks.at(0);
            float kPiMass2 = (tmpKaon+tmpPion).M();

            if(std::abs(kPiMass1 - m_targetMassTrkSysKaonPi) < std::abs(kPiMass2 - m_targetMassTrkSysKaonPi)) {
                finalKPiMass = kPiMass1;
            } else {
                finalKPiMass = kPiMass2;
            }
        }
        float massTrkSysKaonPi = finalKPiMass;
        ATH_MSG_DEBUG(" massTrkSys with kaon+pi mass hypo: " << massTrkSysKaonPi / Gaudi::Units::GeV);

        if(!(massTrkSys > m_massTrkSysMin && massTrkSys < m_massTrkSysMax)) continue;
        passedCuts++;
        massTrkSysAccepted = massTrkSys / Gaudi::Units::GeV;

        if(!(massTrkSysKaon > m_massTrkSysKaonMin && massTrkSysKaon < m_massTrkSysKaonMax)) continue;
        passedCuts++;   
        massTrkSysKaonAccepted = massTrkSysKaon / Gaudi::Units::GeV;

        // Use '>=' here, otherwise the singlepion chain would fail!
        if(!(massTrkSysKaonPi >= m_massTrkSysKaonPiMin && massTrkSysKaonPi < m_massTrkSysKaonPiMax)) continue;
        passedCuts++;
        massTrkSysKaonPiAccepted = massTrkSysKaonPi / Gaudi::Units::GeV;


        //---------------------------------------------------------------
        // Cut on E_{T}^{EM} / p_T^{trk sys}
        //---------------------------------------------------------------
        float EMPOverTrkSysP = -1;
        Tau->detail(xAOD::TauJetParameters::EMPOverTrkSysP, EMPOverTrkSysP);
        ATH_MSG_DEBUG(" EMPOverTrkSysP: " << EMPOverTrkSysP);
        if(!(EMPOverTrkSysP < m_EMPOverTrkSysPMax)) continue;
        passedCuts++;
        EMOverTrkSysPAccepted = EMPOverTrkSysP;

        
        //---------------------------------------------------------------
        // Cut on E_{T} / p_T^{lead trk}
        //---------------------------------------------------------------
        float etOverPtLeadTrk = -1;
        Tau->detail(xAOD::TauJetParameters::etOverPtLeadTrk, etOverPtLeadTrk);
        ATH_MSG_DEBUG(" etOverPtLeadTrk: " << etOverPtLeadTrk);
        if(!(etOverPtLeadTrk > m_etOverPtLeadTrkMin && etOverPtLeadTrk < m_etOverPtLeadTrkMax)) continue;
        passedCuts++;
        etOverPtLeadTrkAccepted = etOverPtLeadTrk;

        
        //---------------------------------------------------------------
        // Cut on Max dR(track, tau)
        //---------------------------------------------------------------
        float dRmax = -1;
        Tau->detail(xAOD::TauJetParameters::dRmax, dRmax);
        ATH_MSG_DEBUG(" dRmax: " << dRmax);
        if(!(dRmax < m_dRmaxMax)) continue;
        passedCuts++;    
        dRAccepted = dRmax; 


        //---------------------------------------------------------
        // At least one Tau passed all the cuts. Accept the event!
        //---------------------------------------------------------
        pass = true;

        ATH_MSG_DEBUG(" Pass hypo tool: " << pass);
    }
  
    return pass;
}

StatusCode TrigTauPrecisionDiKaonHypoTool::decide(std::vector<ITrigTauPrecisionHypoTool::ToolInfo>& input)  const
{
    for(auto& i : input) {
        if(passed(m_decisionId.numeric(), i.previousDecisionIDs)) {
            if(decide(i)) {
                addDecisionID(m_decisionId, i.decision);
            }
        }
    }

    return StatusCode::SUCCESS;
}

