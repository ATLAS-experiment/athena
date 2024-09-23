/*
Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "TRTMonitoringAlg.h"

#include "TrigDecisionTool/TrigDecisionTool.h"

TRTMonitoringAlg::TRTMonitoringAlg(const std::string& name, ISvcLocator* pSvcLocator)
    : AthMonitorAlgorithm(name, pSvcLocator) {}

StatusCode TRTMonitoringAlg::initialize() {
  ATH_CHECK(m_offlineTrkKey.initialize());

  ATH_CHECK(m_trackSelectionTool.retrieve());

  return AthMonitorAlgorithm::initialize();
}

StatusCode TRTMonitoringAlg::fillHistograms(const EventContext& context) const {
  int good_tracks = 0;
  double lead_track_pt = 0;

  // retrieve event info and make selection
  auto offlineTrkHandle = SG::makeHandle(m_offlineTrkKey, context);
  for (const auto trk : *offlineTrkHandle) {
    if (m_trackSelectionTool->accept(*trk)) {
      double pT = trk->pt()*1.e-3; // pT in GeV
      double abs_eta = std::abs(trk->eta());
      double abs_d0 = std::abs(trk->d0());

      if (pT > 0.1 and abs_eta < 2 and abs_d0 < 2) {
        ++good_tracks;

        if(pT > lead_track_pt){
          lead_track_pt = pT;
        }
      }
    }
  }

  // set monitored values
  auto n_trk = Monitored::Scalar<int>("n_trk", good_tracks);
  auto lead_trk_pT = Monitored::Scalar<double>("lead_trk_pT", lead_track_pt);

  // retrieve the trigger decisions
  const auto& trigDecTool = getTrigDecisionTool();
  auto passedL1 = [](unsigned int bits) { return (bits & TrigDefs::L1_isPassedBeforePrescale) != 0; };
  auto activeHLT = [](unsigned int bits) { return (bits & TrigDefs::EF_prescaled) == 0; };
  auto isHLT = [](const std::string& name) { return name.compare(0, 4, "HLT_") == 0; };
  auto isRefPassed = [trigDecTool](const std::string& ref) { return trigDecTool->isPassed(ref, TrigDefs::requireDecision); };

  for (const auto& trig : m_triggerList) {
    // make an "or" of all reference triggers
    if (std::any_of(m_refTriggerList.begin(), m_refTriggerList.end(), isRefPassed)) {
      ATH_MSG_DEBUG("ref passed for " << trig);

      // check the monitored trigger decision at L1
      const unsigned int passBits = trigDecTool->isPassedBits(trig);
      if (isHLT(trig) and activeHLT(passBits)) {
        const auto decision = passedL1(passBits);
        ATH_MSG_DEBUG("chain " << trig << " is " << (decision ? "passed" : "failed") << " at L1");
        
        auto effPassed = Monitored::Scalar<int>("effPassed", decision);
        fill(trig, effPassed, n_trk, lead_trk_pT);
      }
    }
  }

  return StatusCode::SUCCESS;
}
