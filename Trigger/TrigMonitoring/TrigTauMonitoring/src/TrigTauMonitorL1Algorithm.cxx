/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigTauMonitorL1Algorithm.h"


TrigTauMonitorL1Algorithm::TrigTauMonitorL1Algorithm(const std::string& name, ISvcLocator* pSvcLocator)
    : TrigTauMonitorBaseAlgorithm(name, pSvcLocator)
{}


StatusCode TrigTauMonitorL1Algorithm::processEvent(const EventContext& ctx) const
{
    constexpr float threshold_offset = 10.0;

    // Offline taus
    auto offline_taus_all = getOfflineTausAll(ctx, 0.0);

    if(m_requireOfflineTaus && offline_taus_all.empty()) return StatusCode::SUCCESS;

    for(const std::string& trigger : m_triggers) {
        const TrigTauInfo& info = getTrigInfo(trigger);

        if(!info.isL1TauOnly()) {
            ATH_MSG_WARNING("Chain \"" << trigger << "\" is not an L1 tau trigger. Skipping...");
            continue;
        }

        // Filter offline taus
        auto offline_taus = classifyOfflineTaus(offline_taus_all, info.getL1TauThreshold() - threshold_offset);
        std::vector<const xAOD::TauJet*> offline_taus_1p = offline_taus.first;
        std::vector<const xAOD::TauJet*> offline_taus_3p = offline_taus.second;

        if(info.getL1TauType() == "eTAU") {
            std::vector<const xAOD::eFexTauRoI*> rois = getL1eTAUs(ctx, info.getL1TauItem());

            if(m_do_variable_plots) fillL1eTauVars(trigger, rois);
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

            if(m_do_variable_plots) fillL1cTauVars(trigger, rois);
	    if(m_do_efficiency_plots) {
                fillL1Efficiencies(ctx, offline_taus_1p, "1P", trigger, eTau_rois);
                fillL1Efficiencies(ctx, offline_taus_3p, "3P", trigger, eTau_rois);
	    }

        } else { // Legacy
            std::vector<const xAOD::EmTauRoI*> rois = getL1LegacyTAUs(ctx, info.getL1TauItem());

            if(m_do_variable_plots) fillL1LegacyVars(trigger, rois);
	    if(m_do_efficiency_plots) {
                fillL1Efficiencies(ctx, offline_taus_1p, "1P", trigger, rois);
                fillL1Efficiencies(ctx, offline_taus_3p, "3P", trigger, rois);
	    }
        }
    }

    return StatusCode::SUCCESS;
}


void TrigTauMonitorL1Algorithm::fillL1eTauVars(const std::string& trigger, const std::vector<const xAOD::eFexTauRoI*>& rois)  const
{
    ATH_MSG_DEBUG("Fill L1 variables: " << trigger);

    auto monGroup = getGroup(trigger+"_L1Vars");

    auto L1RoIEt        = Monitored::Collection("L1RoIEt"       , rois, [](const xAOD::eFexTauRoI* L1roi){ return L1roi->et()/Gaudi::Units::GeV; });
    auto L1RoIEta       = Monitored::Collection("L1RoIEta"      , rois, [](const xAOD::eFexTauRoI* L1roi){ return L1roi->eta(); });
    auto L1RoIPhi       = Monitored::Collection("L1RoIPhi"      , rois, [](const xAOD::eFexTauRoI* L1roi){ return L1roi->phi(); });
    auto L1RoIRCore     = Monitored::Collection("L1eFexRoIRCore", rois, [](const xAOD::eFexTauRoI* L1roi){ return L1roi->rCore(); });
    auto L1RoIRHad      = Monitored::Collection("L1eFexRoIRHad" , rois, [](const xAOD::eFexTauRoI* L1roi){ return L1roi->rHad(); });
    auto L1RoIBDTScore  = Monitored::Collection("L1eFexRoIBDTScore" , rois, [](const xAOD::eFexTauRoI* L1roi){ return L1roi->bdtScore(); });
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



void TrigTauMonitorL1Algorithm::fillL1cTauVars(const std::string& trigger, const std::vector<std::pair<const xAOD::eFexTauRoI*, const xAOD::jFexTauRoI*>>& rois)  const
{
    ATH_MSG_DEBUG("Fill L1 variables: " << trigger);

    auto monGroup = getGroup(trigger+"_L1Vars");

    auto L1RoIEt        = Monitored::Collection("L1RoIEt"       , rois, [](const auto L1roi){ return L1roi.first->et()/Gaudi::Units::GeV; });
    auto L1RoIEta       = Monitored::Collection("L1RoIEta"      , rois, [](const auto L1roi){ return L1roi.first->eta(); });
    auto L1RoIPhi       = Monitored::Collection("L1RoIPhi"      , rois, [](const auto L1roi){ return L1roi.first->phi(); });
    auto L1eFexRoIRCore = Monitored::Collection("L1eFexRoIRCore", rois, [](const auto L1roi){ return L1roi.first->rCore(); });
    auto L1eFexRoIRHad  = Monitored::Collection("L1eFexRoIRHad" , rois, [](const auto L1roi){ return L1roi.first->rHad(); });
    auto L1RoIBDTScore  = Monitored::Collection("L1eFexRoIBDTScore" , rois, [](const auto L1roi){ return L1roi.first->bdtScore(); });

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


void TrigTauMonitorL1Algorithm::fillL1LegacyVars(const std::string& trigger, const std::vector<const xAOD::EmTauRoI*>& rois)  const
{
    ATH_MSG_DEBUG("Fill L1 variables: " << trigger);

    auto monGroup = getGroup(trigger+"_L1Vars");

    auto L1RoIEt      = Monitored::Collection("L1RoIEt"     , rois, [](const xAOD::EmTauRoI* L1roi){ return L1roi->eT()/Gaudi::Units::GeV; });
    auto L1RoIEta     = Monitored::Collection("L1RoIEta"    , rois, [](const xAOD::EmTauRoI* L1roi){ return L1roi->eta(); });
    auto L1RoIPhi     = Monitored::Collection("L1RoIPhi"    , rois, [](const xAOD::EmTauRoI* L1roi){ return L1roi->phi(); });
    auto L1RoITauClus = Monitored::Collection("L1RoITauClus", rois, [](const xAOD::EmTauRoI* L1roi){ return L1roi->tauClus()/Gaudi::Units::GeV; });
    auto L1RoIEMIsol  = Monitored::Collection("L1RoIEMIsol" , rois, [](const xAOD::EmTauRoI* L1roi){ return L1roi->emIsol()/Gaudi::Units::GeV; });
    auto L1RoIHadCore = Monitored::Collection("L1RoIHadCore", rois, [](const xAOD::EmTauRoI* L1roi){ return L1roi->hadCore()/Gaudi::Units::GeV; });
    auto L1RoIHadIsol = Monitored::Collection("L1RoIHadIsol", rois, [](const xAOD::EmTauRoI* L1roi){ return L1roi->hadIsol()/Gaudi::Units::GeV; });

    fill(monGroup, L1RoIEt, L1RoIEta, L1RoIPhi, L1RoITauClus, L1RoIEMIsol, L1RoIHadCore, L1RoIHadIsol);

    ATH_MSG_DEBUG("After fill L1 variables: " << trigger);
}
