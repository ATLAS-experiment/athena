/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigTauMonitorDiTauAlgorithm.h"


TrigTauMonitorDiTauAlgorithm::TrigTauMonitorDiTauAlgorithm(const std::string& name, ISvcLocator* pSvcLocator)
    : TrigTauMonitorBaseAlgorithm(name, pSvcLocator)
{}


StatusCode TrigTauMonitorDiTauAlgorithm::processEvent(const EventContext& ctx) const
{
    constexpr float threshold_offset = 10.0;

    // Offline taus
    auto offline_taus_all = getOfflineTausAll(ctx, 0.0);

    if(m_requireOfflineTaus && offline_taus_all.empty()) return StatusCode::SUCCESS;

    for(const std::string& trigger : m_triggers) {
        const TrigTauInfo& info = getTrigInfo(trigger);

        if(!info.isHLTDiTau()) {
            ATH_MSG_WARNING("Chain \"" << trigger << "\" is not a di-tau trigger. Skipping...");
            continue;
        }

        const auto passBits = m_trigDecTool->isPassedBits(trigger);
        const bool l1_accept_flag = passBits & TrigDefs::L1_isPassedAfterVeto;
        const bool hlt_not_prescaled_flag = (passBits & TrigDefs::EF_prescaled) == 0;

        // Filter offline taus
        std::vector<const xAOD::TauJet*> offline_taus = classifyTausAll(offline_taus_all, info.getHLTTauThreshold() - threshold_offset);

        // Online taus
        std::vector<const xAOD::TauJet*> hlt_taus = getOnlineTausAll(trigger, true);

        if(m_do_variable_plots) fillDiTauVars(trigger, hlt_taus);
        if(m_do_efficiency_plots && hlt_not_prescaled_flag) fillDiTauHLTEfficiencies(ctx, trigger, l1_accept_flag, offline_taus, hlt_taus);
    }

    return StatusCode::SUCCESS;
}


void TrigTauMonitorDiTauAlgorithm::fillDiTauHLTEfficiencies(const EventContext& ctx, const std::string& trigger, const bool l1_accept_flag, const std::vector<const xAOD::TauJet*>& offline_tau_vec, const std::vector<const xAOD::TauJet*>& online_tau_vec) const
{
    ATH_MSG_DEBUG("Fill DiTau HLT efficiencies: " << trigger);

    const TrigTauInfo& info = getTrigInfo(trigger);

    // Require 2 offline taus
    if(offline_tau_vec.size() != 2) return;

    auto monGroup = getGroup(trigger+"_DiTauHLT_Efficiency");

    auto dR = Monitored::Scalar<float>("dR", 0.0);
    auto dEta = Monitored::Scalar<float>("dEta", 0.0);
    auto dPhi = Monitored::Scalar<float>("dPhi", 0.0);
    auto averageMu = Monitored::Scalar<float>("averageMu", 0.0);
    auto HLT_match = Monitored::Scalar<bool>("HLT_pass", false);
    auto HLT_match_highPt = Monitored::Scalar<bool>("HLT_pass_highPt", false);
    auto Total_match = Monitored::Scalar<bool>("Total_pass", false);
    auto Total_match_highPt = Monitored::Scalar<bool>("Total_pass_highPt", false);

    // efficiency denominator : two offline taus and two online taus (this guarantees that the event passed L1), not necesarily matched
    // efficiency numerator : hlt fires + two offline taus matched with online taus
    bool hlt_fires = m_trigDecTool->isPassed(trigger, TrigDefs::Physics);
    bool tau0_match = matchObjects(offline_tau_vec.at(0), online_tau_vec, 0.2);
    bool tau1_match = matchObjects(offline_tau_vec.at(1), online_tau_vec, 0.2);

    dR = offline_tau_vec.at(0)->p4().DeltaR(offline_tau_vec.at(1)->p4());
    dEta = std::abs(offline_tau_vec.at(0)->p4().Eta() - offline_tau_vec.at(1)->p4().Eta());
    dPhi = offline_tau_vec.at(0)->p4().DeltaPhi(offline_tau_vec.at(1)->p4());
    averageMu = lbAverageInteractionsPerCrossing(ctx);
    HLT_match = hlt_fires && tau0_match && tau1_match;

    float tau0_pT = offline_tau_vec.at(0)->pt()/Gaudi::Units::GeV;
    float tau1_pT = offline_tau_vec.at(1)->pt()/Gaudi::Units::GeV;
    bool is_highPt = tau0_pT > info.getHLTTauThresholds().at(0) + 20.0 && tau1_pT > info.getHLTTauThresholds().at(1) + 20.0;

    // Total efficiency (without L1 matching)
    if(m_doTotalEfficiency) {
        Total_match = static_cast<bool>(HLT_match);
        fill(monGroup, dR, dEta, dPhi, Total_match);

        if(is_highPt) {
            Total_match_highPt = static_cast<bool>(HLT_match);
            fill(monGroup, dR, dEta, dPhi, Total_match_highPt);
        }
    }

    // Require also 2 online tau candidates (thus, the efficiency is with respect to L1)
    if(l1_accept_flag && online_tau_vec.size() == 2) {
        fill(monGroup, dR, dEta, dPhi, averageMu, HLT_match);

        if(is_highPt) {
            HLT_match_highPt = static_cast<bool>(HLT_match);
            fill(monGroup, dR, dEta, dPhi, HLT_match_highPt);
        }
    }

    ATH_MSG_DEBUG("After fill DiTau HLT efficiencies: " << trigger);
}


void TrigTauMonitorDiTauAlgorithm::fillDiTauVars(const std::string& trigger, const std::vector<const xAOD::TauJet*>& tau_vec) const
{
    ATH_MSG_DEBUG("Fill DiTau Variables: " << trigger); 

    auto monGroup = getGroup(trigger+"_DiTauVars");

    if(tau_vec.size() != 2) return; 
    
    auto leadHLTEt = Monitored::Scalar<float>("leadHLTEt", 0.0);
    auto subleadHLTEt = Monitored::Scalar<float>("subleadHLTEt", 0.0);
    auto leadHLTEta = Monitored::Scalar<float>("leadHLTEta", 0.0);
    auto subleadHLTEta = Monitored::Scalar<float>("subleadHLTEta", 0.0);
    auto leadHLTPhi = Monitored::Scalar<float>("leadHLTPhi", 0.0);
    auto subleadHLTPhi = Monitored::Scalar<float>("subleadHLTPhi", 0.0);
    auto dR = Monitored::Scalar<float>("dR", 0.0);
    auto dEta = Monitored::Scalar<float>("dEta", 0.0);  
    auto dPhi = Monitored::Scalar<float>("dPhi", 0.0);
    
    auto Pt = Monitored::Scalar<float>("Pt", 0.0);
    auto Eta = Monitored::Scalar<float>("Eta", 0.0);
    auto Phi = Monitored::Scalar<float>("Phi", 0.0); 
    auto M = Monitored::Scalar<float>("M", 0.0);
    auto dPt = Monitored::Scalar<float>("dPt", 0.0); 

    // Get the index of the leading and the subleading tau
    unsigned int index0 = 0, index1 = 1;
    if(tau_vec.at(1)->p4().Pt() > tau_vec.at(0)->p4().Pt()) {
        index0 = 1;
        index1 = 0;
    } 

    TLorentzVector leadTau4V, subleadTau4V, diTau4V;
    leadTau4V.SetPtEtaPhiM(0,0,0,0);
    subleadTau4V.SetPtEtaPhiM(0,0,0,0);

    leadTau4V = tau_vec.at(index0)->p4();
    subleadTau4V = tau_vec.at(index1)->p4();
    diTau4V = leadTau4V + subleadTau4V;

    leadHLTEt = leadTau4V.Pt()/Gaudi::Units::GeV;
    subleadHLTEt = subleadTau4V.Pt()/Gaudi::Units::GeV;
    leadHLTEta = leadTau4V.Eta();
    subleadHLTEta = subleadTau4V.Eta();
    leadHLTPhi = leadTau4V.Phi();
    subleadHLTPhi = subleadTau4V.Phi();
    dR = leadTau4V.DeltaR(subleadTau4V);
    dEta = std::abs(leadTau4V.Eta() - subleadTau4V.Eta());
    dPhi = leadTau4V.DeltaPhi(subleadTau4V);

    dPt = std::abs((leadTau4V.Pt() - subleadTau4V.Pt())/Gaudi::Units::GeV);
    Pt = diTau4V.Pt()/Gaudi::Units::GeV;
    Eta = diTau4V.Eta();
    Phi = diTau4V.Phi();
    M = diTau4V.M()/Gaudi::Units::GeV;
    
    fill(monGroup, leadHLTEt, subleadHLTEt, leadHLTEta, subleadHLTEta, leadHLTPhi, subleadHLTPhi, dR, dEta, dPhi, dPt, Pt, Eta, Phi, M);

    ATH_MSG_DEBUG("After fill DiTau variables: " << trigger); 
}
