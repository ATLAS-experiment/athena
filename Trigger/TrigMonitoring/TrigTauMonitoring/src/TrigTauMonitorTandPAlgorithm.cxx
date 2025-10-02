/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigTauMonitorTandPAlgorithm.h"


TrigTauMonitorTandPAlgorithm::TrigTauMonitorTandPAlgorithm(const std::string& name, ISvcLocator* pSvcLocator)
    : TrigTauMonitorBaseAlgorithm(name, pSvcLocator)
{}


StatusCode TrigTauMonitorTandPAlgorithm::initialize() {
    ATH_CHECK( TrigTauMonitorBaseAlgorithm::initialize() );

    ATH_CHECK( m_hltElectronKey.initialize() );
    ATH_CHECK( m_hltMuonKey.initialize() );

    ATH_CHECK( m_offlineElectronKey.initialize() );
    ATH_CHECK( m_offlineMuonKey.initialize() );

    return StatusCode::SUCCESS;
}


std::vector<const xAOD::Electron*> TrigTauMonitorTandPAlgorithm::getOfflineElectrons(const EventContext& ctx, const float threshold) const
{
    std::vector<const xAOD::Electron*> el_vec;

    SG::ReadHandle<xAOD::ElectronContainer> electrons(m_offlineElectronKey, ctx);
    if(!electrons.isValid()) {
        ATH_MSG_WARNING("Failed to retrieve offline Electrons");
        return el_vec;
    }

    for(const xAOD::Electron* el : *electrons) {
        // Threshold selection
        if(el->p4().Pt()/Gaudi::Units::GeV < threshold) continue; 

        // Select offline electrons passing good quality cuts
        if(!(el->passSelection("LHMedium") && el->isGoodOQ(xAOD::EgammaParameters::BADCLUSELECTRON))) continue;

        el_vec.push_back(el);
    }
    
    return el_vec;
}


std::vector<const xAOD::Muon*> TrigTauMonitorTandPAlgorithm::getOfflineMuons(const EventContext& ctx, const float threshold) const
{
    std::vector<const xAOD::Muon*> mu_vec;

    SG::ReadHandle<xAOD::MuonContainer> muons(m_offlineMuonKey, ctx);
    if(!muons.isValid()) {
        ATH_MSG_WARNING("Failed to retrieve offline Muons");
        return mu_vec;
    }

    for(const xAOD::Muon* mu : *muons) {
        // Threshold selection
        if(mu->p4().Pt()/Gaudi::Units::GeV < threshold) continue; 

        // Select offline muons passing good quality cuts (quality >= Medium, but the Enum indexes are reversed...)
        if(!(mu->quality() <= xAOD::Muon::Medium && mu->passesIDCuts())) continue;

        mu_vec.push_back(mu);
    }
    
    return mu_vec;
}


std::vector<const xAOD::Electron*> TrigTauMonitorTandPAlgorithm::getOnlineElectrons(const std::string& trigger) const
{
    std::vector<const xAOD::Electron*> el_vec;

    auto vec = m_trigDecTool->features<xAOD::ElectronContainer>(trigger, TrigDefs::Physics, m_hltElectronKey.key());
    for(auto &featLinkInfo : vec) {
        const auto *feat = *(featLinkInfo.link);
        if(!feat) continue;
        el_vec.push_back(feat);
    }

    return el_vec;
}


std::vector<const xAOD::Muon*> TrigTauMonitorTandPAlgorithm::getOnlineMuons(const std::string& trigger) const
{
    std::vector<const xAOD::Muon*> mu_vec;

    auto vec = m_trigDecTool->features<xAOD::MuonContainer>(trigger, TrigDefs::Physics, m_hltMuonKey.key());
    for(auto &featLinkInfo : vec) {
        const auto *feat = *(featLinkInfo.link);
        if(!feat) continue;
        mu_vec.push_back(feat);
    }

    return mu_vec;
}


StatusCode TrigTauMonitorTandPAlgorithm::processEvent(const EventContext& ctx) const
{
    constexpr float threshold_offset = 10.0;

    // Offline taus
    auto offline_taus_all = getOfflineTausAll(ctx, 0.0);
    if(m_requireOfflineTaus && offline_taus_all.empty()) return StatusCode::SUCCESS;

    for(const std::string& trigger : m_triggers) {
        const TrigTauInfo& info = getTrigInfo(trigger);

        if(!info.isHLTTandP()) {
            ATH_MSG_WARNING("Chain \"" << trigger << "\" is not a Tag and Probe trigger. Skipping...");
            continue;
        }

        const auto passBits = m_trigDecTool->isPassedBits(trigger);
        const bool l1_accept_flag = passBits & TrigDefs::L1_isPassedAfterVeto;
        const bool hlt_not_prescaled_flag = (passBits & TrigDefs::EF_prescaled) == 0;

        // Offline tau requirement check
        std::vector<const xAOD::TauJet*> offline_taus_with_id = classifyTausAll(offline_taus_all, 0, static_cast<TauID>(m_offline_tau_id.value()));
        if(m_requireOfflineTaus && offline_taus_with_id.empty()) continue;

        // Filter offline taus
        std::vector<const xAOD::TauJet*> offline_taus = classifyTausAll(offline_taus_with_id, info.getHLTTauThreshold() - threshold_offset);

        // Online taus
        std::vector<const xAOD::TauJet*> hlt_taus = getOnlineTausAll(trigger, true);

        if(info.hasHLTElectronLeg()) { // Electron channel
            // Offline Electrons
            std::vector<const xAOD::IParticle*> offline_electrons;
            for(const xAOD::Electron* p : getOfflineElectrons(ctx, info.getHLTElecThreshold()+1)) offline_electrons.push_back(dynamic_cast<const xAOD::IParticle*>(p));
            
            // Overlap removal: dR(Offline Tau, Offline Electron) > 0.2
            for(int i = offline_taus.size()-1; i >= 0; i--) {
                bool is_match = matchObjects(offline_taus.at(i), offline_electrons, 0.2);
                if(is_match) offline_taus.erase(offline_taus.begin() + i);
            }

            // Online Electrons
            std::vector<const xAOD::IParticle*> hlt_electrons;
            for(const xAOD::Electron* p : getOnlineElectrons(trigger)) hlt_electrons.push_back(dynamic_cast<const xAOD::IParticle*>(p));

            if(m_do_variable_plots) fillTagAndProbeVars(trigger, hlt_taus, hlt_electrons);
            if(m_do_efficiency_plots && l1_accept_flag && hlt_not_prescaled_flag) fillTAndPHLTEfficiencies(ctx, trigger, offline_electrons, hlt_electrons, offline_taus, hlt_taus);

        } else if(info.hasHLTMuonLeg()) { // Muon channel
            // Offline Muons
            std::vector<const xAOD::IParticle*> offline_muons;
            for(const xAOD::Muon* p : getOfflineMuons(ctx, info.getHLTMuonThreshold()+1)) offline_muons.push_back(dynamic_cast<const xAOD::IParticle*>(p));
            
            // Overlap removal: dR(Offline Tau, Offline Tau) > 0.2
            for(int i = offline_taus.size()-1; i >= 0; i--) {
                bool is_match = matchObjects(offline_taus.at(i), offline_muons, 0.2);
                if(is_match) offline_taus.erase(offline_taus.begin() + i);
            }

            // Online Muons
            std::vector<const xAOD::IParticle*> hlt_muons;
            for(const xAOD::Muon* p : getOnlineMuons(trigger)) hlt_muons.push_back(dynamic_cast<const xAOD::IParticle*>(p));

            if(m_do_variable_plots) fillTagAndProbeVars(trigger, hlt_taus, hlt_muons);
            if(m_do_efficiency_plots && l1_accept_flag && hlt_not_prescaled_flag) fillTAndPHLTEfficiencies(ctx, trigger, offline_muons, hlt_muons, offline_taus, hlt_taus);
        }
    }

    return StatusCode::SUCCESS;
}


void TrigTauMonitorTandPAlgorithm::fillTAndPHLTEfficiencies(const EventContext& ctx, const std::string& trigger, const std::vector<const xAOD::IParticle*>& offline_lep_vec, const std::vector<const xAOD::IParticle*>& online_lep_vec, const std::vector<const xAOD::TauJet*>& offline_tau_vec, const std::vector<const xAOD::TauJet*>& online_tau_vec) const
{
    ATH_MSG_DEBUG("Fill Tag and Probe HLT efficiencies: " << trigger);

    // Require 1 offline taus and 1 online taus
    if(online_tau_vec.size() != 1 || offline_tau_vec.size() != 1) return;
    // ...and require 1 offline lepton and 1 online lepton
    if(online_lep_vec.size() != 1 || offline_lep_vec.size() != 1) return;
    
    auto monGroup = getGroup(trigger+"_TAndPHLT_Efficiency");

    auto tauPt = Monitored::Scalar<float>("tauPt", 0.0);
    auto tauEta = Monitored::Scalar<float>("tauEta", 0.0);
    auto tauPhi = Monitored::Scalar<float>("tauPhi", 0.0);
    auto dR = Monitored::Scalar<float>("dR", 0.0);
    auto dEta = Monitored::Scalar<float>("dEta", 0.0);
    auto dPhi = Monitored::Scalar<float>("dPhi", 0.0);
    auto averageMu = Monitored::Scalar<float>("averageMu", 0.0);
    auto HLT_match = Monitored::Scalar<bool>("HLT_pass", false);
    auto HLT_match_highPt = Monitored::Scalar<bool>("HLT_pass_highPt", false);

    // efficiency denominator : 1 offline tau, 1 online tau, 1 offline lepton, 1 online lepton
    // efficiency numerator : hlt fires + offline and online tau matched + offline and online lepton matched
    bool hlt_fires = m_trigDecTool->isPassed(trigger, TrigDefs::Physics);
    bool tau1_match = matchObjects(offline_tau_vec[0], online_tau_vec, 0.2);
    bool lep1_match = matchObjects(offline_lep_vec[0], online_lep_vec, 0.2);

    tauPt = offline_tau_vec[0]->pt()/Gaudi::Units::GeV;
    tauEta = offline_tau_vec[0]->eta();
    tauPhi = offline_tau_vec[0]->phi();
    dR   = offline_tau_vec[0]->p4().DeltaR(offline_lep_vec[0]->p4());
    dEta = std::abs(offline_tau_vec[0]->eta() - offline_lep_vec[0]->eta());
    dPhi = offline_tau_vec[0]->p4().DeltaPhi(offline_lep_vec[0]->p4());
    averageMu = lbAverageInteractionsPerCrossing(ctx);
    HLT_match = hlt_fires && tau1_match && lep1_match;

    fill(monGroup, tauPt, tauEta, tauPhi, dR, dEta, dPhi, averageMu, HLT_match);

    bool is_highPt = tauPt > getTrigInfo(trigger).getHLTTauThreshold() + 20.0;
    if(is_highPt) {
        HLT_match_highPt = static_cast<bool>(HLT_match);
        fill(monGroup, tauEta, tauPhi, HLT_match_highPt);
    }


    ATH_MSG_DEBUG("After fill Tag and Probe HLT efficiencies: " << trigger);
}


void TrigTauMonitorTandPAlgorithm::fillTagAndProbeVars(const std::string& trigger, const std::vector<const xAOD::TauJet*>& tau_vec, const std::vector<const xAOD::IParticle*>& lep_vec) const
{
    ATH_MSG_DEBUG("Fill Tag & Probe Variables: " << trigger); 

    auto monGroup = getGroup(trigger+"_TAndPVars");

    // Require 1 tau and 1 lepton
    if(tau_vec.empty() || lep_vec.empty()) return; 
    
    auto dR = Monitored::Scalar<float>("dR", 0.0);
    auto dEta = Monitored::Scalar<float>("dEta", 0.0);  
    auto dPhi = Monitored::Scalar<float>("dPhi", 0.0);
    auto dPt = Monitored::Scalar<float>("dPt", 0.0); 
    
    auto Pt = Monitored::Scalar<float>("Pt", 0.0);
    auto Eta = Monitored::Scalar<float>("Eta", 0.0);
    auto Phi = Monitored::Scalar<float>("Phi", 0.0); 
    auto M = Monitored::Scalar<float>("M", 0.0);

    // Choose a pair with dR > 0.3 to fill the plot (there must be always at least one pair with dR > 0.3 if the trigger fires)
    uint index_tau = 0;
    uint index_lep = 0;
    
    for(uint i=0; i < tau_vec.size(); i++) {
        for(uint j=0; j< lep_vec.size(); j++) {
            if(tau_vec[i]->p4().DeltaR(lep_vec[j]->p4()) >= 0.3) {
                index_tau = i;
                index_lep = j;
            }  
        }
    }

    dR = tau_vec[index_tau]->p4().DeltaR(lep_vec[index_lep]->p4());
    dEta = std::abs(tau_vec[index_tau]->eta() - lep_vec[index_lep]->eta());
    dPhi = tau_vec[index_tau]->p4().DeltaPhi(lep_vec[index_lep]->p4());
    dPt = std::abs((tau_vec[index_tau]->pt() - lep_vec[index_lep]->pt())/Gaudi::Units::GeV);

    TLorentzVector diTau4V = tau_vec[index_tau]->p4() + lep_vec[index_lep]->p4();

    Pt = diTau4V.Pt()/Gaudi::Units::GeV;
    Eta = diTau4V.Eta();
    Phi = diTau4V.Phi();
    M = diTau4V.M()/Gaudi::Units::GeV;
    
    fill(monGroup, dR, dEta, dPhi, dPt, Pt, Eta, Phi, M);

    ATH_MSG_DEBUG("After fill Tag & Probe variables: " << trigger); 
}
