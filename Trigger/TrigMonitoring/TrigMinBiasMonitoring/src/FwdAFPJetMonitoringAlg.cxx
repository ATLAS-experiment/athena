/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "FwdAFPJetMonitoringAlg.h"

#include <AthenaBaseComps/AthMsgStreamMacros.h>
#include <AthenaMonitoringKernel/MonitoredCollection.h>
#include "TrigDecisionTool/TrigDecisionTool.h"

#include <algorithm>

#include "Utils.h"

FwdAFPJetMonitoringAlg::FwdAFPJetMonitoringAlg(const std::string& name, ISvcLocator* pSvcLocator)
    : AthMonitorAlgorithm(name, pSvcLocator) {}

StatusCode FwdAFPJetMonitoringAlg::initialize() {
  ATH_CHECK(m_jetKey.initialize());

  return AthMonitorAlgorithm::initialize();
}

StatusCode FwdAFPJetMonitoringAlg::fillHistograms(const EventContext& context) const {
  using namespace Monitored;

  const auto& trigDecTool = getTrigDecisionTool();
  SG::ReadHandle<xAOD::JetContainer> jetsHandle(m_jetKey, context);

  ATH_MSG_DEBUG("Using: " << m_jetKey.key() << ", handle state: " << jetsHandle.isValid());
  if (not jetsHandle.isValid()) {
    return StatusCode::SUCCESS;
  }

  for (const auto& chain : m_chains) {
    ATH_MSG_DEBUG("Chain: " << chain << " - Checking...");
    if (not trigDecTool->isPassed(chain, TrigDefs::requireDecision)) {
      ATH_MSG_DEBUG("Chain: " << chain << " - Failed");
      continue;
    }
    ATH_MSG_DEBUG("Chain: " << chain << " - Passed");

    auto jetPt = Collection("jetPt", *jetsHandle, [](const auto& jet) { return jet->pt() * 1.e-3; });
    auto jetEta = Collection("jetEta", *jetsHandle, &xAOD::Jet::eta);
    auto jetPhi = Collection("jetPhi", *jetsHandle, &xAOD::Jet::phi);

    const auto leadingJet = Utils::findLeadingJet(jetsHandle);
    if (leadingJet == nullptr) {
      fill(chain + "_" + m_jetKey.key(), jetPt, jetEta, jetPhi);
      continue;
    }

    auto leadingJetPt = Scalar("leadingJetPt", leadingJet->pt() * 1.e-3);
    auto leadingJetEta = Scalar("leadingJetEta", leadingJet->eta());
    auto leadingJetPhi = Scalar("leadingJetPhi", leadingJet->phi());

    fill(chain + "_" + m_jetKey.key(), jetPt, jetEta, jetPhi, leadingJetPt, leadingJetEta, leadingJetPhi);
  }

  return StatusCode::SUCCESS;
}
