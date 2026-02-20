/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigTauMonitorBoostedDiTauAlgorithm.h"
#include "TrigCompositeUtils/TrigCompositeUtils.h"
#include "TrigDecisionTool/TrigDecisionTool.h"


TrigTauMonitorBoostedDiTauAlgorithm::TrigTauMonitorBoostedDiTauAlgorithm(const std::string& name, ISvcLocator* pSvcLocator)
    : TrigTauMonitorBaseAlgorithm(name, pSvcLocator)
{}

StatusCode TrigTauMonitorBoostedDiTauAlgorithm::initialize() {
    ATH_CHECK( TrigTauMonitorBaseAlgorithm::initialize() );

    ATH_CHECK( m_hltBoostedDiTauJetKey.initialize() );

    return StatusCode::SUCCESS;
}

StatusCode TrigTauMonitorBoostedDiTauAlgorithm::processEvent(const EventContext& /*ctx*/) const
{
    ATH_MSG_DEBUG("Entry process event boosted");
    for(const std::string& trigger : m_triggers) {
        const TrigTauInfo& info = getTrigInfo(trigger);
        if(!info.isHLTBoostedDiTau()) {
            ATH_MSG_WARNING("Chain \"" << trigger << "\" is not a boosted di-tau trigger. Skipping...");
            continue;
        }

        // Online taus
        std::vector<const xAOD::DiTauJet*> hlt_boosted_ditaus = getOnlineBoostedDiTausAll(trigger);

        if(m_do_variable_plots && !hlt_boosted_ditaus.empty()) fillBoostedDiTauVars(trigger, hlt_boosted_ditaus);
    }

    return StatusCode::SUCCESS;
}

std::vector<const xAOD::DiTauJet*> TrigTauMonitorBoostedDiTauAlgorithm::getOnlineBoostedDiTausAll(const std::string& trigger) const
{
    std::vector<const xAOD::DiTauJet*> boosted_ditau_vec;

    std::vector<TrigCompositeUtils::LinkInfo<xAOD::DiTauJetContainer>> features_boosted_ditau =
        m_trigDecTool->features<xAOD::DiTauJetContainer>(trigger, TrigDefs::Physics, m_hltBoostedDiTauJetKey.key());

    for (const auto& fb_ditau : features_boosted_ditau) {
        if (!fb_ditau.link.isValid()) continue;

        const xAOD::DiTauJet* ditau = *(fb_ditau.link);
        if (!ditau) continue;

        boosted_ditau_vec.push_back(ditau);
    }
    return boosted_ditau_vec;
}

void TrigTauMonitorBoostedDiTauAlgorithm::fillBoostedDiTauVars(const std::string& trigger, const std::vector<const xAOD::DiTauJet*>& boosted_ditau_vec) const
{
    auto monGroup = getGroup(trigger+"_BoostedDiTauVars");

    static const SG::ConstAccessor<float> OmniScore("omni_score");
    static const SG::ConstAccessor<float> RTracksLead("R_tracks_lead");
    static const SG::ConstAccessor<float> RTracksSubl("R_tracks_subl");
    static const SG::ConstAccessor<float> FCoreLead("f_core_lead");
    static const SG::ConstAccessor<float> FCoreSubl("f_core_subl");
    static const SG::ConstAccessor<int> NTracks("n_track");
    static const SG::ConstAccessor<int> NTracksLead("n_tracks_lead");
    static const SG::ConstAccessor<int> NTracksSubl("n_tracks_subl");

    const auto* ditau = boosted_ditau_vec.at(0);

    auto omni_score     = Monitored::Scalar<float>("omni_score",    OmniScore(*ditau));
    auto R_tracks_lead  = Monitored::Scalar<float>("R_tracks_lead", RTracksLead(*ditau));
    auto R_tracks_subl  = Monitored::Scalar<float>("R_tracks_subl", RTracksSubl(*ditau));
    auto f_core_lead    = Monitored::Scalar<float>("f_core_lead",   FCoreLead(*ditau));
    auto f_core_subl    = Monitored::Scalar<float>("f_core_subl",   FCoreSubl(*ditau));
    auto n_track       = Monitored::Scalar<int>("n_track", NTracks(*ditau));
    auto n_tracks_lead  = Monitored::Scalar<int>("n_tracks_lead", NTracksLead(*ditau));
    auto n_tracks_subl  = Monitored::Scalar<int>("n_tracks_subl", NTracksSubl(*ditau));
    auto Pt             = Monitored::Scalar<float>("Pt", 0.0);
    auto Eta            = Monitored::Scalar<float>("Eta", 0.0);
    auto Phi            = Monitored::Scalar<float>("Phi", 0.0); 
    auto M              = Monitored::Scalar<float>("M", 0.0);

    TLorentzVector boosted_diTau4V;
    boosted_diTau4V.SetPtEtaPhiM(0,0,0,0);

    boosted_diTau4V = boosted_ditau_vec.at(0)->p4();

    Pt  = boosted_diTau4V.Pt()/Gaudi::Units::GeV;
    Eta = boosted_diTau4V.Eta();
    Phi = boosted_diTau4V.Phi();
    M   = boosted_diTau4V.M()/Gaudi::Units::GeV;

    fill(monGroup, omni_score, R_tracks_lead, R_tracks_subl, f_core_lead, f_core_subl, n_track, n_tracks_lead, n_tracks_subl, Pt, Eta, Phi, M);
}
