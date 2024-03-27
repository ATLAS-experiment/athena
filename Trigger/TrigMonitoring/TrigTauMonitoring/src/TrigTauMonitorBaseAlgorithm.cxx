/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "StoreGate/ReadDecorHandle.h"

#include "LArRecEvent/LArEventBitInfo.h"

#include "TrigTauMonitorBaseAlgorithm.h"

TrigTauMonitorBaseAlgorithm::TrigTauMonitorBaseAlgorithm(const std::string& name, ISvcLocator* pSvcLocator)
    : AthMonitorAlgorithm(name, pSvcLocator)
{}


StatusCode TrigTauMonitorBaseAlgorithm::initialize() {
    ATH_CHECK( AthMonitorAlgorithm::initialize() );
    ATH_CHECK( m_trigDecTool.retrieve() );
    ATH_CHECK( m_eventInfoDecorKey.initialize() );

    ATH_CHECK( m_offlineTauJetKey.initialize() );

    if(m_L1_select_by_et_only) ATH_MSG_INFO("L1 RoI selection by Et cut only! No isolated L1 tau items are allowed!");
    ATH_CHECK( m_legacyl1TauRoIKey.initialize() );
    ATH_CHECK( m_phase1l1eTauRoIKey.initialize() );
    ATH_CHECK( m_phase1l1eTauRoIThresholdPatternsKey.initialize(!m_L1_select_by_et_only) );
    ATH_CHECK( m_phase1l1jTauRoIKey.initialize() );
    ATH_CHECK( m_phase1l1jTauRoIThresholdPatternsKey.initialize(!m_L1_select_by_et_only) );
    ATH_CHECK( m_phase1l1cTauRoIKey.initialize() );
    ATH_CHECK( m_phase1l1cTauRoIThresholdPatternsKey.initialize(!m_L1_select_by_et_only) );
    ATH_CHECK( m_phase1l1cTauRoIDecorKey.initialize() );

    ATH_CHECK( m_hltTauJetKey.initialize() );
    ATH_CHECK( m_hltTauJetLLPKey.initialize() );
    ATH_CHECK( m_hltTauJetLRTKey.initialize() );
    ATH_CHECK( m_hltTauJetCaloMVAOnlyKey.initialize() );

    // Parse TauTrigInfo objects
    for(const std::string& trigger : m_triggers) {
	if(m_L1_select_by_et_only) {
            m_trigInfo[trigger] = TrigTauInfo(trigger, m_L1_Phase1_thresholds);

            if(m_trigInfo[trigger].areAnyL1TauIsolated()) {
                ATH_MSG_FATAL("Cannot use isolated L1 tau items if running with SelectL1ByETOnly = True: " << trigger);
	        return StatusCode::FAILURE;
            }
        } else {
            m_trigInfo[trigger] = TrigTauInfo(trigger, m_L1_Phase1_thresholds, m_L1_Phase1_threshold_patterns);
        }
    }

    return StatusCode::SUCCESS;
}

std::vector<const xAOD::TauJet*> TrigTauMonitorBaseAlgorithm::getOnlineTausAll(const std::string& trigger, bool include_0P) const
{
    std::vector<const xAOD::TauJet*> tau_vec;

    const std::string tau_container_name = getOnlineContainerKey(trigger).key();
    ATH_MSG_DEBUG("Tau container name is: " << tau_container_name);
    auto vec = m_trigDecTool->features<xAOD::TauJetContainer>(trigger, TrigDefs::Physics, tau_container_name);
    for(auto& featLinkInfo : vec) {
        const xAOD::TauJet* feat = *(featLinkInfo.link);
        if(!feat) continue;

        int nTracks = -1;
        feat->detail(xAOD::TauJetParameters::nChargedTracks, nTracks);
        ATH_MSG_DEBUG("NTracks Online: " << nTracks);

        if(include_0P && nTracks == 0) tau_vec.push_back(feat);
        else tau_vec.push_back(feat);
    }

    return tau_vec;
}

std::tuple<std::vector<const xAOD::TauJet*>, std::vector<const xAOD::TauJet*>, std::vector<const xAOD::TauJet*>> TrigTauMonitorBaseAlgorithm::getOnlineTaus(const std::string& trigger) const
{
    return classifyOnlineTaus(getOnlineTausAll(trigger, true), 0.0);
}


std::vector<const xAOD::TauJet*> TrigTauMonitorBaseAlgorithm::getOfflineTausAll(const EventContext& ctx, const float threshold) const
{
    ATH_MSG_DEBUG("Retrieving offline Taus");

    std::vector<const xAOD::TauJet*> tau_vec;

    SG::ReadHandle<xAOD::TauJetContainer> taus(m_offlineTauJetKey, ctx);
    if(!taus.isValid()) {
        ATH_MSG_WARNING("Failed to retrieve offline Taus");
        return tau_vec;
    }

    for(const xAOD::TauJet* const tau : *taus) {
        // Consider only offline taus with a certain minimum pT
        if(tau->pt() < threshold*Gaudi::Units::GeV) continue;

        // Consider only offline taus outside of the crack region
        if(std::abs(tau->eta()) > 1.37 && std::abs(tau->eta()) < 1.52) continue;

        // Consider only offline taus which pass RNN medium WP 
        if(!tau->isTau(xAOD::TauJetParameters::JetRNNSigMedium)) continue;

        // Consider only offline taus which pass thinning 
        if(tau->isAvailable<char>("passThinning") && !tau->auxdata<char>("passThinning") ) continue;

        int nTracks = -1;
        tau->detail(xAOD::TauJetParameters::nChargedTracks, nTracks);
        ATH_MSG_DEBUG("NTracks Offline: " << nTracks);
        if(nTracks == 1 || nTracks == 3) tau_vec.push_back(tau);
    }

    return tau_vec;
}


std::pair<std::vector<const xAOD::TauJet*>, std::vector<const xAOD::TauJet*>> TrigTauMonitorBaseAlgorithm::getOfflineTaus(const EventContext& ctx, const float threshold) const
{
    return classifyOfflineTaus(getOfflineTausAll(ctx, threshold), threshold);
}


std::vector<const xAOD::eFexTauRoI*> TrigTauMonitorBaseAlgorithm::getL1eTAUs(const EventContext& ctx, const std::string& l1_item) const
{
    std::vector<const xAOD::eFexTauRoI*> roi_vec;

    SG::ReadHandle<xAOD::eFexTauRoIContainer> rois(m_phase1l1eTauRoIKey, ctx);
    if(!rois.isValid()) {
        ATH_MSG_WARNING("Failed to retrieve the L1_eTauRoi container");
        return roi_vec;
    }

    if(m_L1_select_by_et_only) {
        for(const xAOD::eFexTauRoI* roi : *rois) {
            // Select by RoI ET value only
            if(roi->et() > m_L1_Phase1_thresholds.value().at(l1_item)) roi_vec.push_back(roi);
        }
    } else {
        SG::ReadDecorHandle<xAOD::eFexTauRoIContainer, uint64_t> thresholdPatterns(m_phase1l1eTauRoIThresholdPatternsKey, ctx);
        if(!thresholdPatterns.isValid()) {
            ATH_MSG_WARNING("Failed to create thresholdPatterns property accessor for the L1_eTauRoi container");
            return roi_vec;
        }
        
        for(const xAOD::eFexTauRoI* roi : *rois) {
            // Check that the RoI passed the threshold selection
            if(thresholdPatterns(*roi) & m_L1_Phase1_threshold_patterns.value().at(l1_item)) roi_vec.push_back(roi);
        }
    }

    return roi_vec;
}


std::vector<const xAOD::jFexTauRoI*> TrigTauMonitorBaseAlgorithm::getL1jTAUs(const EventContext& ctx, const std::string& l1_item) const
{
    std::vector<const xAOD::jFexTauRoI*> roi_vec;

    SG::ReadHandle<xAOD::jFexTauRoIContainer> rois(m_phase1l1jTauRoIKey, ctx);
    if(!rois.isValid()) {
        ATH_MSG_WARNING("Failed to retrieve the L1_jTauRoi container");
        return roi_vec;
    }

    if(m_L1_select_by_et_only) {
        for(const xAOD::jFexTauRoI* roi : *rois) {
            // Select by RoI ET value only
            if(roi->et() > m_L1_Phase1_thresholds.value().at(l1_item)) roi_vec.push_back(roi);
        }
    } else {
        SG::ReadDecorHandle<xAOD::jFexTauRoIContainer, uint64_t> thresholdPatterns(m_phase1l1jTauRoIThresholdPatternsKey, ctx);
        if(!thresholdPatterns.isValid()) {
            ATH_MSG_WARNING("Failed to create thresholdPatterns property accessor for the L1_jTauRoi container");
            return roi_vec;
        }
        
        for(const xAOD::jFexTauRoI* roi : *rois) {
            // Check that the RoI passed the threshold selection
            if(thresholdPatterns(*roi) & m_L1_Phase1_threshold_patterns.value().at(l1_item)) roi_vec.push_back(roi);
        }
    }

    return roi_vec;
}


std::vector<std::pair<const xAOD::eFexTauRoI*, const xAOD::jFexTauRoI*>> TrigTauMonitorBaseAlgorithm::getL1cTAUs(const EventContext& ctx, const std::string& l1_item) const
{
    std::vector<std::pair<const xAOD::eFexTauRoI*, const xAOD::jFexTauRoI*>> roi_vec;

    SG::ReadHandle<xAOD::eFexTauRoIContainer> rois(m_phase1l1cTauRoIKey, ctx);
    if(!rois.isValid()) {
        ATH_MSG_WARNING("Failed to retrieve the L1_cTauRoi container");
        return roi_vec;
    }
    SG::ReadDecorHandle<xAOD::eFexTauRoIContainer, ElementLink<xAOD::jFexTauRoIContainer>> jTau_roi_link{m_phase1l1cTauRoIDecorKey, ctx};
    if(!jTau_roi_link.isValid()) {
        ATH_MSG_WARNING("Failed to create jTauLink accessor for the L1_cTauRoi container");
        return roi_vec;
    }

    if(m_L1_select_by_et_only) {
        for(size_t i = 0; i < rois->size(); i++) {
            const xAOD::eFexTauRoI* roi = (*rois)[i];
            const xAOD::jFexTauRoI* jTau_roi = jTau_roi_link(i).isValid() ? *jTau_roi_link(i) : nullptr;

            // Select by RoI ET value only
            if(roi->et() > m_L1_Phase1_thresholds.value().at(l1_item)) roi_vec.push_back(std::make_pair(roi, jTau_roi));
        }
    } else {
        SG::ReadDecorHandle<xAOD::eFexTauRoIContainer, uint64_t> thresholdPatterns(m_phase1l1cTauRoIThresholdPatternsKey, ctx);
        if(!thresholdPatterns.isValid()) {
            ATH_MSG_WARNING("Failed to create thresholdPatterns property accessor for the L1_cTauRoi container");
            return roi_vec;
        }

        for(size_t i = 0; i < rois->size(); i++) {
            const xAOD::eFexTauRoI* roi = (*rois)[i];
            const xAOD::jFexTauRoI* jTau_roi = jTau_roi_link(i).isValid() ? *jTau_roi_link(i) : nullptr;

            // Check that the RoI passed the threshold selection
            if(thresholdPatterns(*roi) & m_L1_Phase1_threshold_patterns.value().at(l1_item)) roi_vec.push_back(std::make_pair(roi, jTau_roi));
        }   
    }


    return roi_vec;
}


std::vector<const xAOD::EmTauRoI*> TrigTauMonitorBaseAlgorithm::getL1LegacyTAUs(const EventContext& ctx, const std::string& l1_item) const
{
    std::vector<const xAOD::EmTauRoI*> roi_vec;

    SG::ReadHandle<xAOD::EmTauRoIContainer> rois(m_legacyl1TauRoIKey, ctx);
    if(!rois.isValid()) {
        ATH_MSG_WARNING("Failed to retrieve EmTauRoI ");
        return roi_vec;
    }

    for(const xAOD::EmTauRoI* roi : *rois) {
        for(const std::string& thr_item : roi->thrNames()) {
            // check which threshold has passed based on the current L1 item under monitoring
            // reference : https://gitlab.cern.ch/atlas/athena/-/blob/master/Trigger/TriggerCommon/TriggerMenuMT/python/L1/Config/ItemDef.py
            if((l1_item.find("TAU8") != std::string::npos && thr_item.find("HA8") != std::string::npos)
                || (l1_item.find("TAU12IM") != std::string::npos && thr_item.find("HA12IM") != std::string::npos)
                || (l1_item.find("TAU20IM") != std::string::npos && thr_item.find("HA20IM") != std::string::npos)
                || (l1_item.find("TAU40")   != std::string::npos && thr_item.find("HA40")   != std::string::npos)
                || (l1_item.find("TAU60")   != std::string::npos && thr_item.find("HA60")   != std::string::npos)
                || (l1_item.find("TAU100")  != std::string::npos && thr_item.find("HA100")  != std::string::npos)) {
                roi_vec.push_back(roi);
                break;
            }
        }
    }

    return roi_vec;
}


const SG::ReadHandleKey<xAOD::TauJetContainer>& TrigTauMonitorBaseAlgorithm::getOnlineContainerKey(const std::string& trigger) const
{
    const TrigTauInfo& info = getTrigInfo(trigger);
    if(info.getHLTTauType() == "tracktwoMVA" || info.getHLTTauType() == "tracktwoMVABDT") return m_hltTauJetKey;
    else if(info.getHLTTauType() == "tracktwoLLP") return m_hltTauJetLLPKey;
    else if(info.getHLTTauType() == "trackLRT") return m_hltTauJetLRTKey;
    else if(info.getHLTTauType() == "ptonly") return m_hltTauJetCaloMVAOnlyKey;
    else {
        ATH_MSG_ERROR("Unknown HLT TauJet container for chain: \"" << trigger << "\", of type \"" << info.getHLTTauType() << "\". Returning the default \"" << m_hltTauJetKey.key() << "\"");
        return m_hltTauJetKey;
    }
}


StatusCode TrigTauMonitorBaseAlgorithm::fillHistograms(const EventContext& ctx) const
{
    ATH_MSG_DEBUG("Executing Monitoring algorithm");

    // Protect against truncated events
    // Since this happens very rarely, it won't bias the L1 distributions and efficiencies
    if(m_trigDecTool->ExperimentalAndExpertMethods().isHLTTruncated()){
        ATH_MSG_WARNING("HLTResult truncated, skip trigger analysis");
        return StatusCode::SUCCESS; 
    }

    // Protect against noise bursts
    SG::ReadHandle<xAOD::EventInfo> currEvent(GetEventInfo(ctx));
    ATH_CHECK(currEvent.isValid());
    if(currEvent->isEventFlagBitSet(xAOD::EventInfo::LAr, LArEventBitInfo::NOISEBURSTVETO)) return StatusCode::SUCCESS;
    
    ATH_CHECK(processEvent(ctx));

    return StatusCode::SUCCESS;
}


std::vector<const xAOD::TauJet*> TrigTauMonitorBaseAlgorithm::classifyTausAll(const std::vector<const xAOD::TauJet*> taus, const float threshold) const
{
    std::vector<const xAOD::TauJet*> tau_vec;

    for(const xAOD::TauJet* tau : taus) {
        if(tau->pt() < threshold) continue;
        tau_vec.push_back(tau);
    }

    return tau_vec;
}


std::tuple<std::vector<const xAOD::TauJet*>, std::vector<const xAOD::TauJet*>, std::vector<const xAOD::TauJet*>> TrigTauMonitorBaseAlgorithm::classifyOnlineTaus(const std::vector<const xAOD::TauJet*> taus, const float threshold) const
{
    std::vector<const xAOD::TauJet*> tau_vec_0p, tau_vec_1p, tau_vec_mp;

    for(const xAOD::TauJet* tau : taus) {
        if(tau->pt() < threshold) continue;

        int nTracks = -1;
        tau->detail(xAOD::TauJetParameters::nChargedTracks, nTracks);

        if(nTracks == 0) tau_vec_0p.push_back(tau);
        else if(nTracks == 1) tau_vec_1p.push_back(tau);
        else tau_vec_mp.push_back(tau);
    }

    return {tau_vec_0p, tau_vec_1p, tau_vec_mp};
}


std::pair<std::vector<const xAOD::TauJet*>, std::vector<const xAOD::TauJet*>> TrigTauMonitorBaseAlgorithm::classifyOfflineTaus(const std::vector<const xAOD::TauJet*> taus, const float threshold) const
{
    std::vector<const xAOD::TauJet*> tau_vec_1p, tau_vec_3p;

    for(const xAOD::TauJet* const tau : taus) {
        if(tau->pt()/Gaudi::Units::GeV < threshold) continue;

        int nTracks = -1;
        tau->detail(xAOD::TauJetParameters::nChargedTracks, nTracks);

        if(nTracks == 1) tau_vec_1p.push_back(tau);
        else if(nTracks == 3) tau_vec_3p.push_back(tau); 
    }

    return {tau_vec_1p, tau_vec_3p};
}
