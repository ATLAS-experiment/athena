/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// Local includes
#include "ROBPrefetchingAlg.h"

// Trigger includes
#include "TrigSteeringEvent/TrigRoiDescriptorCollection.h"
#include "TrigSteeringEvent/TrigRoiDescriptor.h"

// System includes
#include <unordered_set>
#include <vector>
#include <sstream>

namespace {
  std::string robSetToString(const std::unordered_set<uint32_t>& robs) {
    std::ostringstream ss;
    bool first{true};
    ss << "[";
    for (const uint32_t rob : robs) {
      if (first) {first=false;}
      else {ss << ", ";}
      ss << "0x" << std::hex << rob << std::dec;
    }
    ss << "]";
    return ss.str();
  }
}

ROBPrefetchingAlg::ROBPrefetchingAlg(const std::string& name, ISvcLocator* pSvcLocator)
: AthReentrantAlgorithm(name, pSvcLocator) {}

StatusCode ROBPrefetchingAlg::initialize() {
  ATH_CHECK(m_inputDecisions.initialize());
  ATH_CHECK(m_regionSelectorTools.retrieve());
  ATH_CHECK(m_robDataProviderSvc.retrieve());
  ATH_MSG_DEBUG("Will consume " << m_inputDecisions.size() << " input decision collection(s):");
  if (msgLvl(MSG::DEBUG)) {
    for (const auto& decisionContKey : m_inputDecisions) {
      ATH_MSG_DEBUG("- " << decisionContKey.key());
    }
  }
  // Convert ChainFilter vector to set
  m_chainFilter.insert(m_chainFilterVec.value().begin(), m_chainFilterVec.value().end());
  if (not m_chainFilter.empty()) {
    ATH_MSG_DEBUG("Will consider only RoIs from " << m_chainFilter.size()
                  << " chains (or chain legs) in " << m_chainFilterVec.name());
  }
  return StatusCode::SUCCESS;
}

StatusCode ROBPrefetchingAlg::execute(const EventContext& eventContext) const {
  using namespace TrigCompositeUtils;
  std::unordered_set<uint32_t> robsToPrefetch;
  std::vector<uint32_t> robsInRoI; // reused in the loop for each RoI

  // Loop over all input Decision collections to extract RoIs
  for (const auto& decisionContKey : m_inputDecisions) {
    SG::ReadHandle<DecisionContainer> decisionCont{decisionContKey, eventContext};
    ATH_CHECK(decisionCont.isValid());
    ATH_MSG_DEBUG("Processing " << decisionCont->size() << " decisions in " << decisionContKey.key());
    for (const Decision* decision : *decisionCont) {

      if (not m_chainFilter.empty()) {
        bool skipPrefetching{true};
        // Loop over active chains for this Decision to see if at least one matches the ChainFilter
        for (const DecisionID chainLegID : decisionIDs(decision)) {
          if (m_chainFilter.find(chainLegID)!=m_chainFilter.cend()) {
            skipPrefetching = false;
            break;
          }
        }
        if (skipPrefetching) {
          ATH_MSG_DEBUG("Skipping decision " << decision->name() << " because no active chain matches the " << m_chainFilterVec.name());
          continue;
        }
      }

      ElementLink<TrigRoiDescriptorCollection> roiEL = findLink<TrigRoiDescriptorCollection>(decision, m_roiLinkName.value()).link;
      if (!roiEL.isValid()) {
        ATH_MSG_WARNING("No " << m_roiLinkName.value() << " link in " << decisionContKey.key() << " decision " << *decision);
        continue;
      }

      const TrigRoiDescriptor& roi = **roiEL;
      ATH_MSG_DEBUG("Processing decision " << *decision);
      ATH_MSG_DEBUG("Processing RoI: " << roi);

      for (const auto& tool : m_regionSelectorTools) {
        robsInRoI.clear();
        tool->lookup(eventContext)->ROBIDList( roi, robsInRoI );
        robsToPrefetch.insert(robsInRoI.begin(),robsInRoI.end());
      }
    }
  }

  // Prefetch all ROBs collected above
  ATH_MSG_DEBUG("Prefetching " << robsToPrefetch.size() << " ROBs: " << robSetToString(robsToPrefetch));
  std::vector<uint32_t> robsToPrefetchVec{robsToPrefetch.begin(),robsToPrefetch.end()};
  m_robDataProviderSvc->addROBData(eventContext, robsToPrefetchVec, name());

  return StatusCode::SUCCESS;
}
