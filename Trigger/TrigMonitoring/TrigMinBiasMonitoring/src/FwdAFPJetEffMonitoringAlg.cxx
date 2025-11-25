/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "FwdAFPJetEffMonitoringAlg.h"

#include <AthenaBaseComps/AthMsgStreamMacros.h>
#include <AthenaMonitoringKernel/MonitoredCollection.h>
#include "TrigDecisionTool/TrigDecisionTool.h"

#include <algorithm>
#include <iterator>

#include "Utils.h"

FwdAFPJetEffMonitoringAlg::FwdAFPJetEffMonitoringAlg(const std::string& name, ISvcLocator* pSvcLocator)
    : AthMonitorAlgorithm(name, pSvcLocator) {}

StatusCode FwdAFPJetEffMonitoringAlg::initialize() {
  ATH_CHECK(m_jetKey.initialize());

  return AthMonitorAlgorithm::initialize();
}

StatusCode FwdAFPJetEffMonitoringAlg::fillHistograms(const EventContext& context) const {
  using namespace Monitored;

  const auto& trigDecTool = getTrigDecisionTool();
  SG::ReadHandle<xAOD::JetContainer> jetsHandle(m_jetKey, context);

  ATH_MSG_DEBUG("Using: " << m_jetKey.key() << ", handle state: " << jetsHandle.isValid());
  if (not jetsHandle.isValid()) {
    return StatusCode::SUCCESS;
  }

  auto passedL1 = [](unsigned int bits) { return (bits & TrigDefs::L1_isPassedBeforePrescale) != 0; };
  auto passedHLT = [](unsigned int bits) { return (bits & TrigDefs::EF_passedRaw) != 0; };
  auto activeHLT = [](unsigned int bits) { return (bits & TrigDefs::EF_prescaled) == 0; };
  auto isL1 = [](const std::string& name) { return name.compare(0, 3, "L1_") == 0; };
  auto isHLT = [](const std::string& name) { return name.compare(0, 4, "HLT_") == 0; };

  for (size_t index = 0; index < m_references.size(); ++index) {
    const auto& trig = m_chains[index];
    const auto& ref = m_references[index];

    ATH_MSG_VERBOSE("Check: " << trig << " vs " << ref << "...");

    if (trigDecTool->isPassed(ref, TrigDefs::requireDecision)) {
      ATH_MSG_VERBOSE("Reference: " << ref << " - Passed!");

      const unsigned int passBits = trigDecTool->isPassedBits(trig);
      const bool wasRun = isL1(trig) or (isHLT(trig) and activeHLT(passBits));

      if (wasRun) {
        const auto decision = (isL1(trig) and passedL1(passBits)) or (isHLT(trig) and passedHLT(passBits));
        ATH_MSG_DEBUG("Chain " << trig << " vs " << ref << " - " << (decision ? "Passed!" : "Failed!"));

        auto leadingJet = Utils::findLeadingJet(jetsHandle);
        auto leadingJetPt = Scalar<float>("leadingJetPt", leadingJet ? leadingJet->pt() * 1e-3 : -1);
        auto effPassed = Scalar<int>("effPassed", decision);
        fill(trig + "_" + ref, effPassed, leadingJetPt);
      }
    }
  }

  return StatusCode::SUCCESS;
}
