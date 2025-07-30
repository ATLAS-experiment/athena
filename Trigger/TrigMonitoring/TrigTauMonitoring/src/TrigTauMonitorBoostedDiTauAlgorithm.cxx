/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigTauMonitorBoostedDiTauAlgorithm.h"


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

        if(m_do_variable_plots) fillBoostedDiTauVars(trigger, hlt_boosted_ditaus);
    }

    return StatusCode::SUCCESS;
}

std::vector<const xAOD::DiTauJet*> TrigTauMonitorBoostedDiTauAlgorithm::getOnlineBoostedDiTausAll(const std::string& trigger) const
{
    std::vector<const xAOD::DiTauJet*> boosted_ditau_vec;

    if (!m_trigDecTool->isPassed(trigger)) {
        ATH_MSG_INFO("Trigger " << trigger << " not passed ");
        return boosted_ditau_vec;
    }

    SG::ReadHandle<xAOD::DiTauJetContainer> ditauJets(m_hltBoostedDiTauJetKey, Gaudi::Hive::currentContext());


    ATH_MSG_INFO(" Container Size : " << ditauJets->size());

    for (const xAOD::DiTauJet* ditau : *ditauJets) {
        if (!ditau) continue;
        boosted_ditau_vec.push_back(ditau);
    }

    return boosted_ditau_vec;
}

void TrigTauMonitorBoostedDiTauAlgorithm::fillBoostedDiTauVars(const std::string& trigger, const std::vector<const xAOD::DiTauJet*>& boosted_ditau_vec) const
{
    auto monGroup = getGroup(trigger+"_BoostedDiTauVars");

    if (boosted_ditau_vec.empty()) {
        ATH_MSG_INFO("boosted_ditau_vec is empty for trigger: " << trigger);
        return;
    }

    static const SG::ConstAccessor<float> OmniScore("omni_score");
    static const SG::ConstAccessor<float> RTracksLead("R_tracks_lead");
    static const SG::ConstAccessor<float> RTracksSubl("R_tracks_subl");
    static const SG::ConstAccessor<float> FCoreLead("f_core_lead");
    static const SG::ConstAccessor<float> FCoreSubl("f_core_subl");

    const auto* ditau = boosted_ditau_vec.at(0);

    auto omni_score     = Monitored::Scalar<float>("omni_score",    OmniScore(*ditau));
    auto R_tracks_lead  = Monitored::Scalar<float>("R_tracks_lead", RTracksLead(*ditau));
    auto R_tracks_subl  = Monitored::Scalar<float>("R_tracks_subl", RTracksSubl(*ditau));
    auto f_core_lead    = Monitored::Scalar<float>("f_core_lead",   FCoreLead(*ditau));
    auto f_core_subl    = Monitored::Scalar<float>("f_core_subl",   FCoreSubl(*ditau));

    fill(monGroup, omni_score, R_tracks_lead, R_tracks_subl, f_core_lead, f_core_subl);
}
