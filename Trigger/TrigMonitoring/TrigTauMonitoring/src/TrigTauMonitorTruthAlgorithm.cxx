/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "TruthUtils/HepMCHelpers.h"

#include "TrigTauMonitorTruthAlgorithm.h"
#include "AthContainers/ConstAccessor.h"
#include "AthContainers/Decorator.h"


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
            static const SG::ConstAccessor<char> IsLeptonicTauAcc("IsLeptonicTau");
            if(IsLeptonicTauAcc(*xTruthTau)) continue;

            static const SG::ConstAccessor<double> pt_visAcc("pt_vis");
            static const SG::ConstAccessor<double> eta_visAcc("eta_vis");
            float pt = pt_visAcc(*xTruthTau);
            float eta = eta_visAcc(*xTruthTau);
            ATH_MSG_DEBUG("True Tau visible pt: " << pt << ", eta: " << eta);

            // Keep only truth taus in the barrel region, with a pT > 20 GeV (offline minimum threshold)
            if(pt < threshold || std::abs(eta) > 2.47) continue;

            static const SG::ConstAccessor<int> nTracksAcc("nTracks");
            if(nTracksAcc(*xTruthTau) == 1) true_taus_1p.push_back(xTruthTau);
            else if(nTracksAcc(*xTruthTau) == 3) true_taus_3p.push_back(xTruthTau);
        }
    }

    return {true_taus_1p, true_taus_3p};
}


StatusCode TrigTauMonitorTruthAlgorithm::examineTruthTau(const std::shared_ptr<xAOD::TruthParticle>& xTruthTau) const
{
    if(!xTruthTau->hasDecayVtx()) return StatusCode::FAILURE;

    static const SG::Decorator<char> IsLeptonicTauDec("IsLeptonicTau");
    IsLeptonicTauDec(*xTruthTau) = false;
        
    TLorentzVector VisSumTLV;
    static const SG::Decorator<double> pt_visDec("pt_vis");
    static const SG::Decorator<double> eta_visDec("eta_vis");
    static const SG::Decorator<double> phi_visDec("phi_vis");
    static const SG::Decorator<double> mvisDec("mvis");
    static const SG::Decorator<int> childChargeSumDec("childChargeSum");
    static const SG::Decorator<int> nTracksDec("nTracks");
    pt_visDec(*xTruthTau) = 0;
    eta_visDec(*xTruthTau) = 0;
    phi_visDec(*xTruthTau) = 0;
    mvisDec(*xTruthTau) = 0;
    childChargeSumDec(*xTruthTau) = 0;
    nTracksDec(*xTruthTau) = 0;
    
    const xAOD::TruthVertex* decayvtx = xTruthTau->decayVtx();
    if(decayvtx) {
        const std::size_t nChildren = decayvtx->nOutgoingParticles();
        for(std::size_t iChild = 0; iChild != nChildren; ++iChild) {
            const xAOD::TruthParticle* child = decayvtx->outgoingParticle(iChild);
            if(child) {
                if(MC::isSMNeutrino(child)) continue;
                if(!MC::isPhysical(child)) continue;
                ATH_MSG_DEBUG("Child " << child->pdgId() << ", status " << child->status() << ", charge " << child->charge());
                if(MC::isSMLepton(child)) IsLeptonicTauDec(*xTruthTau) = true; // Just selects charged SM Leptons as we have already skipped SM neutrinos
                VisSumTLV += child->p4();
                childChargeSumDec(*xTruthTau) += child->charge();
                nTracksDec(*xTruthTau) += std::abs(child->charge());
            }
        }
    }

    pt_visDec(*xTruthTau) = VisSumTLV.Pt();
    eta_visDec(*xTruthTau) = VisSumTLV.Eta();
    phi_visDec(*xTruthTau) = VisSumTLV.Phi();
    mvisDec(*xTruthTau) = VisSumTLV.M();

    if(childChargeSumDec(*xTruthTau) != xTruthTau->charge() || nTracksDec(*xTruthTau)%2 == 0) { 
        ATH_MSG_WARNING("Strange tau: charge " << childChargeSumDec(*xTruthTau) << " and " << nTracksDec(*xTruthTau)  << " tracks");
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

	// skip ditau and T&P chains:
	const TrigTauInfo& info = getTrigInfo(trigger);
	if( info.isHLTTandP() || info.isHLTDiTau()) continue;

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
    static const SG::ConstAccessor<double> pt_visAcc("pt_vis");
    static const SG::ConstAccessor<double> eta_visAcc("eta_vis");
    static const SG::ConstAccessor<double> phi_visAcc("phi_vis");
    for(const std::shared_ptr<xAOD::TruthParticle>& true_tau : true_taus) {
        pt_vis = pt_visAcc(*true_tau)/Gaudi::Units::GeV;
        eta_vis = eta_visAcc(*true_tau);
        phi_vis = phi_visAcc(*true_tau);

        HLT_truth_match = matchObjects(true_tau.get(), online_tau_vec, 0.2) && hlt_fires;

        bool is_highPt = false;
        if(info.isHLTSingleTau())  is_highPt = pt_vis > info.getHLTTauThreshold() + 20.0;

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
    static const SG::ConstAccessor<double> pt_visAcc("pt_vis");
    static const SG::ConstAccessor<double> eta_visAcc("eta_vis");
    static const SG::ConstAccessor<double> phi_visAcc("phi_vis");
    static const SG::ConstAccessor<double> mvisAcc("mvis");
    for(auto& HLTTau : ef_taus) {
        for(const std::shared_ptr<xAOD::TruthParticle>& true_tau : true_taus) {
            TLorentzVector true_tau4V;
            true_tau4V.SetPtEtaPhiM(pt_visAcc(*true_tau),
                                    eta_visAcc(*true_tau),
                                    phi_visAcc(*true_tau),
                                    mvisAcc(*true_tau));
            if(true_tau4V.DeltaR(HLTTau->p4()) < 0.2) {
                matchedptvis = (pt_visAcc(*true_tau)/Gaudi::Units::GeV);
                matchedetavis = eta_visAcc(*true_tau);
                matchedphivis = phi_visAcc(*true_tau);
                matchedmvis = mvisAcc(*true_tau);
                matchedRatio = (HLTTau->p4().Pt() - pt_visAcc(*true_tau))/pt_visAcc(*true_tau);
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
