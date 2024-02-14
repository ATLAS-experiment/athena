/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "TruthUtils/HepMCHelpers.h"

#include "TrigTauMonitorTruthAlgorithm.h"


TrigTauMonitorTruthAlgorithm::TrigTauMonitorTruthAlgorithm(const std::string& name, ISvcLocator* pSvcLocator)
    : TrigTauMonitorBaseAlgorithm(name, pSvcLocator)
{}


StatusCode TrigTauMonitorTruthAlgorithm::initialize() {
    ATH_CHECK( TrigTauMonitorBaseAlgorithm::initialize() );

    ATH_CHECK( m_truthParticleKey.initialize() );

    return StatusCode::SUCCESS;
}


std::pair<std::vector<std::shared_ptr<xAOD::TruthParticle>>, std::vector<std::shared_ptr<xAOD::TruthParticle>>> TrigTauMonitorTruthAlgorithm::getTruthTaus(const EventContext& ctx, const float threshold) const
{
    std::vector<std::shared_ptr<xAOD::TruthParticle>> true_taus_1p;
    std::vector<std::shared_ptr<xAOD::TruthParticle>> true_taus_3p;

    // Truth Taus distributions
    SG::ReadHandle<xAOD::TruthParticleContainer> truth_cont(m_truthParticleKey, ctx); 
    if(!truth_cont.isValid()) {
        ATH_MSG_WARNING("Failed to retrieve truth Taus");
        return {true_taus_1p, true_taus_3p};
    }

    // Fill truth tau containers
    for(const auto xTruthParticle : *truth_cont) {
        if(xTruthParticle->isTau()) {
            ATH_MSG_DEBUG("Tau with status " << xTruthParticle->status() << " and charge " << xTruthParticle->charge());

            // Create a copy of the original TruthParticle, to augment it with tau-specific properties
            std::shared_ptr xTruthTau = std::make_shared<xAOD::TruthParticle>();
            xTruthTau->makePrivateStore(*xTruthParticle);

            // Keep only truth taus
            if(examineTruthTau(xTruthTau).isFailure()) continue;

            // Keep only the hadronic decay mode
            if(xTruthTau->auxdata<char>("IsLeptonicTau")) continue;

            float pt = xTruthTau->auxdata<double>("pt_vis");
            float eta = xTruthTau->auxdata<double>("eta_vis");
            ATH_MSG_DEBUG("True Tau visible pt: " << pt << ", eta: " << eta);

            // Keep only truth taus in the barrel region, with a pT > 20 GeV (offline minimum threshold)
            if(pt < threshold || std::abs(eta) > 2.47) continue;

            if(xTruthTau->auxdata<int>("nTracks") == 1) true_taus_1p.push_back(xTruthTau);
            else if(xTruthTau->auxdata<int>("nTracks") == 3) true_taus_3p.push_back(xTruthTau);
        }
    }

    return {true_taus_1p, true_taus_3p};
}


StatusCode TrigTauMonitorTruthAlgorithm::examineTruthTau(const std::shared_ptr<xAOD::TruthParticle> xTruthTau) const
{
    if(!xTruthTau->hasDecayVtx()) return StatusCode::FAILURE;

    xTruthTau->auxdecor<char>("IsLeptonicTau") = false;
        
    TLorentzVector VisSumTLV;
    xTruthTau->auxdecor<double>("pt_vis") = 0;
    xTruthTau->auxdecor<double>("eta_vis") = 0;
    xTruthTau->auxdecor<double>("phi_vis") = 0;
    xTruthTau->auxdecor<double>("mvis") = 0;
    xTruthTau->auxdecor<int>("childChargeSum") = 0;
    xTruthTau->auxdecor<int>("nTracks") = 0;
    
    const xAOD::TruthVertex* decayvtx = xTruthTau->decayVtx();
    if(decayvtx) {
        const std::size_t nChildren = decayvtx->nOutgoingParticles();
        for(std::size_t iChild = 0; iChild != nChildren; ++iChild) {
            const xAOD::TruthParticle* child = decayvtx->outgoingParticle(iChild);
            if(child) {
                if(MC::isSMNeutrino(child)) continue;
                if(child->status() == 3) continue;
                ATH_MSG_DEBUG("Child " << child->pdgId() << ", status " << child->status() << ", charge " << child->charge());
                if(MC::isSMLepton(child)) xTruthTau->auxdecor<char>("IsLeptonicTau") = true; // Just selects charged SM Leptons as we have already skipped SM neutrinos
                VisSumTLV += child->p4();
                xTruthTau->auxdecor<int>("childChargeSum") += child->charge();
                xTruthTau->auxdecor<int>("nTracks") += std::abs(child->charge());
            }
        }
    }

    xTruthTau->auxdecor<double>("pt_vis") = VisSumTLV.Pt();
    xTruthTau->auxdecor<double>("eta_vis") = VisSumTLV.Eta();
    xTruthTau->auxdecor<double>("phi_vis") = VisSumTLV.Phi();
    xTruthTau->auxdecor<double>("mvis") = VisSumTLV.M();

    if(xTruthTau->auxdecor<int>("childChargeSum") != xTruthTau->charge() || xTruthTau->auxdecor<int>("nTracks")%2 == 0) { 
        ATH_MSG_WARNING("Strange tau: charge " << xTruthTau->auxdecor<int>("childChargeSum") << " and " << xTruthTau->auxdecor<int>("nTracks")  << " tracks");
        const std::size_t nChildren = decayvtx->nOutgoingParticles();
        for(std::size_t iChild = 0; iChild != nChildren; ++iChild) {
        const xAOD::TruthParticle * child = decayvtx->outgoingParticle(iChild);
        if(child) ATH_MSG_WARNING("Child "<< child->pdgId() << ", status "<< child->status() << ", charge "<< child->charge());
        }
    }

    return StatusCode::SUCCESS;
}


StatusCode TrigTauMonitorTruthAlgorithm::processEvent(const EventContext& ctx) const
{
    // Truth taus
    auto true_taus = getTruthTaus(ctx, 20.0);
    std::vector<std::shared_ptr<xAOD::TruthParticle>> true_taus_1p = true_taus.first;
    std::vector<std::shared_ptr<xAOD::TruthParticle>> true_taus_3p = true_taus.second;

    for(const std::string& trigger : m_triggers) {
        // Online taus
        std::vector<const xAOD::TauJet*> hlt_taus = getOnlineTausAll(trigger, true);


        if(!true_taus_1p.empty()) {
            if(m_do_variable_plots) fillTruthVars(hlt_taus, true_taus_1p, trigger, "1P");
            if(m_do_efficiency_plots) fillTruthEfficiency(hlt_taus, true_taus_1p, trigger, "1P");
        } 

        if(!true_taus_3p.empty()) {
            if(m_do_variable_plots) fillTruthVars(hlt_taus, true_taus_3p, trigger, "3P");
            if(m_do_efficiency_plots) fillTruthEfficiency(hlt_taus, true_taus_3p, trigger, "3P");
        } 
    }

    return StatusCode::SUCCESS;
}


void TrigTauMonitorTruthAlgorithm::fillTruthEfficiency(const std::vector<const xAOD::TauJet*>& online_tau_vec, const std::vector<std::shared_ptr<xAOD::TruthParticle>>& true_taus, const std::string& trigger, const std::string& nProng) const
{
    ATH_MSG_DEBUG("Fill Truth Tau Matching to Offline and Online Taus efficiencies: " << trigger);
    
    const TrigTauInfo& info = getTrigInfo(trigger);

    auto monGroup = getGroup(trigger+"_Truth_Efficiency_"+nProng);

    // Truth Tau + HLT Tau / Truth Tau
    auto pt_vis = Monitored::Scalar<float>("pt_vis", 0.0);
    auto eta_vis = Monitored::Scalar<float>("eta_vis", 0.0);
    auto phi_vis = Monitored::Scalar<float>("phi_vis", 0.0);
    auto HLT_truth_match = Monitored::Scalar<bool>("HLT_pass", false);  
    auto HLT_truth_match_highPt = Monitored::Scalar<bool>("HLT_pass_highPt", false);  

    bool hlt_fires = m_trigDecTool->isPassed(trigger, TrigDefs::Physics | TrigDefs::allowResurrectedDecision);
    for(const std::shared_ptr<xAOD::TruthParticle>& true_tau : true_taus) {
        pt_vis = true_tau->auxdata<double>("pt_vis")/Gaudi::Units::GeV;
        eta_vis = true_tau->auxdata<double>("eta_vis");
        phi_vis = true_tau->auxdata<double>("phi_vis");

        HLT_truth_match = matchObjects(true_tau.get(), online_tau_vec, 0.2) && hlt_fires;

        bool is_highPt = false;
        if(info.isHLTSingleTau() || info.isHLTTandP()) is_highPt = pt_vis > info.getHLTTauThreshold() + 20.0;
        else if (info.isHLTDiTau()) is_highPt = pt_vis > info.getHLTTauThresholds().at(0) + 20.0 || pt_vis > info.getHLTTauThresholds().at(1) + 20.0;

        fill(monGroup, pt_vis, eta_vis, phi_vis, HLT_truth_match);

        if(is_highPt) {
            HLT_truth_match_highPt = static_cast<bool>(HLT_truth_match);
            fill(monGroup, eta_vis, phi_vis, HLT_truth_match_highPt);
        }
    } 
    ATH_MSG_DEBUG("After fill Truth efficiencies");
}


void TrigTauMonitorTruthAlgorithm::fillTruthVars(const std::vector<const xAOD::TauJet*>& ef_taus, const std::vector<std::shared_ptr<xAOD::TruthParticle>>& true_taus, const std::string& trigger, const std::string& nProng) const
{
    ATH_MSG_DEBUG("Fill Truth variables: " << trigger);

    auto monGroup = getGroup(trigger+"_TruthVars_"+nProng);

    std::vector<float> ratio, ptvis, etavis, phivis, mvis;

    auto PtRatio = Monitored::Collection("PtRatio", ratio);
    auto pt_vis = Monitored::Collection("pt_vis", ptvis);
    auto eta_vis = Monitored::Collection("eta_vis", etavis);
    auto phi_vis = Monitored::Collection("phi_vis", phivis);
    auto mass_vis = Monitored::Collection("mass_vis", mvis);

    float matchedRatio = -999, matchedptvis = -999, matchedetavis = 999, matchedphivis = 999, matchedmvis = -999;

    // Visible-Truth Tau matching to HLT Tau
    for(auto& HLTTau : ef_taus) {
        for(const std::shared_ptr<xAOD::TruthParticle>& true_tau : true_taus) {
            TLorentzVector true_tau4V;
            true_tau4V.SetPtEtaPhiM(true_tau->auxdata<double>("pt_vis"), true_tau->auxdata<double>("eta_vis"), true_tau->auxdata<double>("phi_vis"), true_tau->auxdata<double>("mvis"));
            if(true_tau4V.DeltaR(HLTTau->p4()) < 0.2) {
                matchedptvis = (true_tau->auxdata<double>("pt_vis")/Gaudi::Units::GeV);
                matchedetavis = true_tau->auxdata<double>("eta_vis");
                matchedphivis = true_tau->auxdata<double>("phi_vis");
                matchedmvis = true_tau->auxdata<double>("mvis");
                matchedRatio = (HLTTau->p4().Pt() - true_tau->auxdata<double>("pt_vis"))/true_tau->auxdata<double>("pt_vis");
            }
        }

        if(matchedptvis > 0) {
            ptvis.push_back(matchedptvis);
            etavis.push_back(matchedetavis);
            phivis.push_back(matchedphivis);
            mvis.push_back(matchedmvis);
            ratio.push_back(matchedRatio);      
        }
    }

    fill(monGroup, pt_vis, eta_vis, phi_vis, mass_vis, PtRatio);

    ATH_MSG_DEBUG("After fill Truth variables");
}
