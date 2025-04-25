/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigTauMonitorL1Algorithm.h"


TrigTauMonitorL1Algorithm::TrigTauMonitorL1Algorithm(const std::string& name, ISvcLocator* pSvcLocator)
    : TrigTauMonitorBaseAlgorithm(name, pSvcLocator)
{}


StatusCode TrigTauMonitorL1Algorithm::initialize()
{
    ATH_CHECK( TrigTauMonitorBaseAlgorithm::initialize() );

    ATH_CHECK( m_phase1l1eTauxRoIKey.initialize(SG::AllowEmpty) );

    return StatusCode::SUCCESS;
}

StatusCode TrigTauMonitorL1Algorithm::processEvent(const EventContext& ctx) const
{
    constexpr float threshold_offset = 10.0;

    // Offline taus
    auto offline_taus_all = getOfflineTausAll(ctx, 0.0);
    if(m_requireOfflineTaus && offline_taus_all.empty()) return StatusCode::SUCCESS;

    // xTOB-based eTAU RoIs
    // If the container is not in SG, it'll return an empty vector
    const std::vector<const xAOD::eFexTauRoI*> xtob_etau_rois = getL1xTOBeTAUs();

    for(const std::string& trigger : m_triggers) {
        const TrigTauInfo& info = getTrigInfo(trigger);

        if(!info.isL1TauOnly()) {
            ATH_MSG_WARNING("Chain \"" << trigger << "\" is not an L1 tau trigger. Skipping...");
            continue;
        }

        // Filter offline taus
        auto offline_taus = classifyOfflineTaus(offline_taus_all, info.getL1TauThreshold() - threshold_offset, static_cast<TauID>(m_offline_tau_id.value()));
        std::vector<const xAOD::TauJet*> offline_taus_1p = offline_taus.first;
        std::vector<const xAOD::TauJet*> offline_taus_3p = offline_taus.second;
        if(m_requireOfflineTaus && offline_taus_1p.empty() && offline_taus_3p.empty()) continue;

        if(info.getL1TauType() == "eTAU") {
            std::vector<const xAOD::eFexTauRoI*> rois = getL1eTAUs(ctx, info.getL1TauItem());

            if(m_do_variable_plots) fillL1eTauVars(trigger, rois, xtob_etau_rois);
	    if(m_do_efficiency_plots) {
                fillL1Efficiencies(ctx, offline_taus_1p, "1P", trigger, rois);
                fillL1Efficiencies(ctx, offline_taus_3p, "3P", trigger, rois);
	    }

        } else if(info.getL1TauType() == "jTAU") {
            std::vector<const xAOD::jFexTauRoI*> rois = getL1jTAUs(ctx, info.getL1TauItem());

            if(m_do_variable_plots) fillL1jTauVars(trigger, rois);
	    if(m_do_efficiency_plots) {
                fillL1Efficiencies(ctx, offline_taus_1p, "1P", trigger, rois);
                fillL1Efficiencies(ctx, offline_taus_3p, "3P", trigger, rois);
	    }

        } else if(info.getL1TauType() == "cTAU") {
            std::vector<std::pair<const xAOD::eFexTauRoI*, const xAOD::jFexTauRoI*>> rois = getL1cTAUs(ctx, info.getL1TauItem());

            std::vector<const xAOD::eFexTauRoI*> eTau_rois;
            eTau_rois.reserve(rois.size());
for(const auto& [eTau_roi, jTau_roi] : rois) eTau_rois.push_back(eTau_roi);

            if(m_do_variable_plots) fillL1cTauVars(trigger, rois, xtob_etau_rois);
	    if(m_do_efficiency_plots) {
                fillL1Efficiencies(ctx, offline_taus_1p, "1P", trigger, eTau_rois);
                fillL1Efficiencies(ctx, offline_taus_3p, "3P", trigger, eTau_rois);
	    }

        } 
    }

    return StatusCode::SUCCESS;
}


void TrigTauMonitorL1Algorithm::fillL1eTauVars(const std::string& trigger, const std::vector<const xAOD::eFexTauRoI*>& rois, const std::vector<const xAOD::eFexTauRoI*>& xtob_rois) const
{
    ATH_MSG_DEBUG("Fill L1 variables: " << trigger);

    auto monGroup = getGroup(trigger+"_L1Vars");

    auto L1RoIEt        = Monitored::Collection("L1RoIEt"       , rois, [](const xAOD::eFexTauRoI* L1roi){ return L1roi->et()/Gaudi::Units::GeV; });
    auto L1RoIEta       = Monitored::Collection("L1RoIEta"      , rois, [](const xAOD::eFexTauRoI* L1roi){ return L1roi->eta(); });
    auto L1RoIPhi       = Monitored::Collection("L1RoIPhi"      , rois, [](const xAOD::eFexTauRoI* L1roi){ return L1roi->phi(); });
    auto L1RoIRCore     = Monitored::Collection("L1eFexRoIRCore", rois, [](const xAOD::eFexTauRoI* L1roi){ return L1roi->rCore(); });
    auto L1RoIRHad      = Monitored::Collection("L1eFexRoIRHad" , rois, [](const xAOD::eFexTauRoI* L1roi){ return L1roi->rHad(); });
    auto L1RoIBDTScore  = Monitored::Collection("L1eFexRoIBDTScore" , rois, [this, &xtob_rois](const xAOD::eFexTauRoI* L1roi){ return getBDTScore(L1roi, xtob_rois); });
    fill(monGroup, L1RoIEt, L1RoIEta, L1RoIPhi, L1RoIRCore, L1RoIRHad, L1RoIBDTScore);

    ATH_MSG_DEBUG("After fill L1 variables: " << trigger);
}


void TrigTauMonitorL1Algorithm::fillL1jTauVars(const std::string& trigger, const std::vector<const xAOD::jFexTauRoI*>& rois)  const
{
    ATH_MSG_DEBUG("Fill L1 variables: " << trigger);

    auto monGroup = getGroup(trigger+"_L1Vars");

    auto L1RoIEt      = Monitored::Collection("L1RoIEt"     , rois, [](const xAOD::jFexTauRoI* L1roi){ return L1roi->et()/Gaudi::Units::GeV; });
    auto L1RoIEta     = Monitored::Collection("L1RoIEta"    , rois, [](const xAOD::jFexTauRoI* L1roi){ return L1roi->eta(); });
    auto L1RoIPhi     = Monitored::Collection("L1RoIPhi"    , rois, [](const xAOD::jFexTauRoI* L1roi){ return L1roi->phi(); });
    auto L1jFexRoIIso = Monitored::Collection("L1jFexRoIIso", rois, [](const xAOD::jFexTauRoI* L1roi){ return L1roi->iso()/Gaudi::Units::GeV; });

    fill(monGroup, L1RoIEt, L1RoIEta, L1RoIPhi, L1jFexRoIIso);

    ATH_MSG_DEBUG("After fill L1 variables: " << trigger);
}



void TrigTauMonitorL1Algorithm::fillL1cTauVars(const std::string& trigger, const std::vector<std::pair<const xAOD::eFexTauRoI*, const xAOD::jFexTauRoI*>>& rois, const std::vector<const xAOD::eFexTauRoI*>& xtob_rois) const
{
    ATH_MSG_DEBUG("Fill L1 variables: " << trigger);

    auto monGroup = getGroup(trigger+"_L1Vars");

    auto L1RoIEt        = Monitored::Collection("L1RoIEt"       , rois, [](const auto L1roi){ return L1roi.first->et()/Gaudi::Units::GeV; });
    auto L1RoIEta       = Monitored::Collection("L1RoIEta"      , rois, [](const auto L1roi){ return L1roi.first->eta(); });
    auto L1RoIPhi       = Monitored::Collection("L1RoIPhi"      , rois, [](const auto L1roi){ return L1roi.first->phi(); });
    auto L1eFexRoIRCore = Monitored::Collection("L1eFexRoIRCore", rois, [](const auto L1roi){ return L1roi.first->rCore(); });
    auto L1eFexRoIRHad  = Monitored::Collection("L1eFexRoIRHad" , rois, [](const auto L1roi){ return L1roi.first->rHad(); });
    auto L1RoIBDTScore  = Monitored::Collection("L1eFexRoIBDTScore" , rois, [this, &xtob_rois](const auto L1roi){ return getBDTScore(L1roi.first, xtob_rois); });

    std::vector<bool> jFex_isMatched;
    std::vector<float> jFex_eFex_et_ratio;
    std::vector<float> jFex_isolation;
    std::vector<float> cTau_isolation;
    for(const auto& [eFexRoI, jFexRoI] : rois) {
        if(!jFexRoI) {
            jFex_isMatched.push_back(false);
            continue;
        }

        jFex_isMatched.push_back(true);
        jFex_eFex_et_ratio.push_back(jFexRoI->et()/eFexRoI->et());
        jFex_isolation.push_back(jFexRoI->iso()/Gaudi::Units::GeV);
        cTau_isolation.push_back(jFexRoI->iso()/eFexRoI->et());
    }
    auto L1cTauRoITopoMatch = Monitored::Collection("L1cTauRoITopoMatch"     , jFex_isMatched);  
    auto L1jFexRoIIso       = Monitored::Collection("L1jFexRoIIso"           , jFex_isolation);
    auto L1cTauRoIIso       = Monitored::Collection("L1cTauMatchedRoIIso"    , cTau_isolation);
    auto L1RoIcTauEtRatio   = Monitored::Collection("L1RoIcTauMatchedEtRatio", jFex_eFex_et_ratio);

    fill(monGroup, L1RoIEt, L1RoIEta, L1RoIPhi, L1eFexRoIRCore, L1eFexRoIRHad, L1cTauRoITopoMatch, L1jFexRoIIso, L1cTauRoIIso, L1RoIcTauEtRatio, L1RoIBDTScore );

    ATH_MSG_DEBUG("After fill L1 variables: " << trigger);
}

std::vector<const xAOD::eFexTauRoI*> TrigTauMonitorL1Algorithm::getL1xTOBeTAUs() const
{
    std::vector<const xAOD::eFexTauRoI*> roi_vec;

    if(m_phase1l1eTauxRoIKey.empty()) return roi_vec;

    SG::ReadHandle<xAOD::eFexTauRoIContainer> rois(m_phase1l1eTauxRoIKey);
    if(!rois.isValid()) {
        ATH_MSG_WARNING("The L1_eTauxRoI container is not available! No e/cTAU BDT score will be retrieved");
        return roi_vec;
    }
    
    for(const xAOD::eFexTauRoI* roi : *rois) roi_vec.push_back(roi);
    return roi_vec;
}

unsigned int TrigTauMonitorL1Algorithm::getBDTScore(const xAOD::eFexTauRoI* roi, const std::vector<const xAOD::eFexTauRoI*>& xtob_rois) const
{
    if(xtob_rois.empty()) return 0;

    for(const xAOD::eFexTauRoI* xroi : xtob_rois) {
        if(xroi->eta() == roi->eta() && xroi->phi() == roi->phi()) return xroi->bdtScore();
    }
    ATH_MSG_DEBUG("Unmatched RoI! et=" << roi->et() << ", eta=" << roi->eta() << ", phi=" << roi->phi());
    for(const xAOD::eFexTauRoI* xroi : xtob_rois) {
        ATH_MSG_DEBUG(" - xRoI et=" << xroi->et() << ", eta=" << xroi->eta() << ", phi=" << xroi->phi());
    }
    return 0;
}

