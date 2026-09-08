/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#if __GNUC__ >= 16
// Suppress false-positive warning seen with gcc16.
# pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif

#include "StoreGate/ReadDecorHandle.h"

#include "LArRecEvent/LArEventBitInfo.h"

#include "TrigTauMonitorBaseAlgorithm.h"
#include "TrigDecisionTool/TrigDecisionTool.h"

TrigTauMonitorBaseAlgorithm::TrigTauMonitorBaseAlgorithm(const std::string& name, ISvcLocator* pSvcLocator)
    : AthMonitorAlgorithm(name, pSvcLocator)
{}


StatusCode TrigTauMonitorBaseAlgorithm::initialize() {
    ATH_CHECK( AthMonitorAlgorithm::initialize() );
    ATH_CHECK( m_trigDecTool.retrieve() );
    ATH_CHECK( m_eventInfoDecorKey.initialize() );

    ATH_CHECK( m_offlineTauJetKey.initialize() );
    m_offlineGNTauDecorKey = m_offlineTauJetKey.key() + "." + m_offlineGNTauDecorKey.key();
    ATH_CHECK( m_offlineGNTauDecorKey.initialize() );

    if(m_L1_select_by_et_only) ATH_MSG_INFO("L1 RoI selection by Et cut only! No isolated L1 tau items are allowed!");
    ATH_CHECK( m_phase1l1eTauRoIKey.initialize() );
    ATH_CHECK( m_phase1l1eTauRoIThresholdPatternsKey.initialize(!m_L1_select_by_et_only) );
    ATH_CHECK( m_phase1l1jTauRoIKey.initialize() );
    ATH_CHECK( m_phase1l1jTauRoIThresholdPatternsKey.initialize(!m_L1_select_by_et_only) );
    ATH_CHECK( m_phase1l1cTauRoIKey.initialize() );
    ATH_CHECK( m_phase1l1cTauRoIThresholdPatternsKey.initialize(!m_L1_select_by_et_only) );
    ATH_CHECK( m_phase1l1cTauRoIDecorKey.initialize() );

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

    for(const TrigTauInfo& info : getTrigInfoMap() | std::views::values) {
        // First check if we already have the container key for this tau leg suffix
        const std::string sfx = info.getHLTTauLegContainerSfx();
        if(m_hltTauJetKeysMap.find(sfx) != m_hltTauJetKeysMap.end()) continue;

        // Add and initialize the container key
        m_hltTauJetKeysMap[sfx] = SG::ReadHandleKey<xAOD::TauJetContainer>("HLT_TrigTauRecMerged_" + sfx);
        ATH_CHECK( m_hltTauJetKeysMap.at(sfx).initialize() );
    }

    return StatusCode::SUCCESS;
}


std::vector<const xAOD::TauJet*> TrigTauMonitorBaseAlgorithm::getOnlineTausAll(const std::string& trigger, bool /*include_0P*/, bool filter_legs) const
{
    std::vector<const xAOD::TauJet*> tau_vec;
    
    const TrigTauInfo& info = getTrigInfo(trigger);

    std::vector<int> leg_indices = {-1};
    if(filter_legs) leg_indices = info.getHLTTauLegIndices();
    for(size_t i = 0; i < leg_indices.size(); ++i) {
        const int leg = leg_indices[i];
        const std::string tau_container_name = getOnlineContainerKey(info.getHLTTauLegContainerSfxs().at(leg == -1 ? 0 : i)).key();
        ATH_MSG_DEBUG("Tau container name is: " << tau_container_name);

        auto vec = m_trigDecTool->features<xAOD::TauJetContainer>(trigger, TrigDefs::Physics, tau_container_name, TrigDefs::lastFeatureOfType, TrigCompositeUtils::featureString(), leg);
        for(auto& featLinkInfo : vec) {
            const xAOD::TauJet* feat = *(featLinkInfo.link);
            if(!feat) continue;

            int nTracks = -1;
            feat->detail(xAOD::TauJetParameters::nChargedTracks, nTracks);
            ATH_MSG_DEBUG("NTracks Online: " << nTracks);

            tau_vec.push_back(feat);
        }
    }

    return tau_vec;
}


std::tuple<std::vector<const xAOD::TauJet*>, std::vector<const xAOD::TauJet*>, std::vector<const xAOD::TauJet*>> TrigTauMonitorBaseAlgorithm::getOnlineTaus(const EventContext& ctx, const std::string& trigger) const
{
    return classifyOnlineTaus(ctx, getOnlineTausAll(trigger, true), 0.0);
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

        // Consider only offline taus which pass thinning
        static const SG::ConstAccessor<char> passThinningAcc("passThinning");
        if(!passThinningAcc.withDefault(*tau, true)) continue;

        int nTracks = -1;
        tau->detail(xAOD::TauJetParameters::nChargedTracks, nTracks);
        ATH_MSG_DEBUG("NTracks Offline: " << nTracks);
        if(nTracks == 1 || nTracks == 3) tau_vec.push_back(tau);
    }

    return tau_vec;
}


std::pair<std::vector<const xAOD::TauJet*>, std::vector<const xAOD::TauJet*>> TrigTauMonitorBaseAlgorithm::getOfflineTaus(const EventContext& ctx, const float threshold, const TauID tau_id) const
{
  return classifyOfflineTaus(ctx, getOfflineTausAll(ctx, threshold), threshold, tau_id);
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


const SG::ReadHandleKey<xAOD::TauJetContainer>& TrigTauMonitorBaseAlgorithm::getOnlineContainerKey(const std::string& container_suffix) const
{
    if(m_hltTauJetKeysMap.find(container_suffix) != m_hltTauJetKeysMap.end()) return m_hltTauJetKeysMap.at(container_suffix);
    else {
        if(m_hltTauJetKeysMap.find("MVA") != m_hltTauJetKeysMap.end()) {
            ATH_MSG_ERROR("HLT TauJet container not registered for suffix \"" << container_suffix << "\". Returning the default \"" << m_hltTauJetKeysMap.at("MVA").key() << "\"");
            return m_hltTauJetKeysMap.at("MVA");
        } else {
            ATH_MSG_ERROR("HLT TauJet container not registered for suffix \"" << container_suffix << "\". Returning the first available container \"" << m_hltTauJetKeysMap.begin()->second.key() << "\"");
            return m_hltTauJetKeysMap.begin()->second;
        }
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

    // Protect against LAr noise bursts and other detector errors
    SG::ReadHandle<xAOD::EventInfo> eventInfo(GetEventInfo(ctx));
    ATH_CHECK(eventInfo.isValid());
    if (eventInfo->errorState(xAOD::EventInfo::LAr) == xAOD::EventInfo::Error
	|| eventInfo->errorState(xAOD::EventInfo::Tile) == xAOD::EventInfo::Error
	|| eventInfo->errorState(xAOD::EventInfo::SCT) == xAOD::EventInfo::Error
	|| eventInfo->isEventFlagBitSet(xAOD::EventInfo::Core, 18)) {
      return StatusCode::SUCCESS;
    }

    ATH_CHECK(processEvent(ctx));

    return StatusCode::SUCCESS;
}


std::vector<const xAOD::TauJet*> TrigTauMonitorBaseAlgorithm::classifyTausAll(const EventContext& ctx, const std::vector<const xAOD::TauJet*>& taus, const float threshold, const TauID tau_id) const
{
    std::vector<const xAOD::TauJet*> tau_vec;

    // Protection since GNTau is not run for taus below 13 GeV
    float upd_threshold = threshold;
    if(tau_id == TauID::GNTau){
      if( upd_threshold < 13){
         upd_threshold = 13;
      }
    } 

    SG::ReadDecorHandle<xAOD::TauJetContainer, char> tauid_medium{m_offlineGNTauDecorKey, ctx};
    if(!tauid_medium.isValid()) {
      ATH_MSG_WARNING("Cannot retrieve " << tauid_medium.key());
      return tau_vec;
    }

    for(const xAOD::TauJet* tau : taus) {
        if(tau->pt() < upd_threshold*Gaudi::Units::GeV) continue;

        // Consider only offline taus which pass medium ID WP
        if(tau_id == TauID::RNN) {
	  if(!tau->isTau(xAOD::TauJetParameters::JetRNNSigMedium)) continue;
	}
        else if(tau_id == TauID::GNTau) {
	  if(!tauid_medium(*tau)) continue;
        }

        tau_vec.push_back(tau);
    }

    return tau_vec;
}


std::tuple<std::vector<const xAOD::TauJet*>, std::vector<const xAOD::TauJet*>, std::vector<const xAOD::TauJet*>> TrigTauMonitorBaseAlgorithm::classifyOnlineTaus(const EventContext& ctx, const std::vector<const xAOD::TauJet*>& taus, const float threshold) const
{
    std::vector<const xAOD::TauJet*> tau_vec_0p, tau_vec_1p, tau_vec_mp;

    for(const xAOD::TauJet* tau : classifyTausAll(ctx, taus, threshold, TauID::None)) {
        int nTracks = -1;
        tau->detail(xAOD::TauJetParameters::nChargedTracks, nTracks);

        if(nTracks == 0) tau_vec_0p.push_back(tau);
        else if(nTracks == 1) tau_vec_1p.push_back(tau);
        else tau_vec_mp.push_back(tau);
    }

    return {tau_vec_0p, tau_vec_1p, tau_vec_mp};
}


std::pair<std::vector<const xAOD::TauJet*>, std::vector<const xAOD::TauJet*>> TrigTauMonitorBaseAlgorithm::classifyOfflineTaus(const EventContext& ctx, const std::vector<const xAOD::TauJet*>& taus, const float threshold, const TauID tau_id) const
{
    std::vector<const xAOD::TauJet*> tau_vec_1p, tau_vec_3p;

    for(const xAOD::TauJet* const tau : classifyTausAll(ctx, taus, threshold, tau_id)) {
        int nTracks = -1;
        tau->detail(xAOD::TauJetParameters::nChargedTracks, nTracks);

        if(nTracks == 1) tau_vec_1p.push_back(tau);
        else if(nTracks == 3) tau_vec_3p.push_back(tau); 
    }

    return {tau_vec_1p, tau_vec_3p};
}
