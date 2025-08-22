/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "eFexTauRoIThresholdsTool.h"

uint64_t eFexTauRoIThresholdsTool::getPattern(const EventContext& /*ctx*/,
                                              const xAOD::eFexTauRoI& roi,
                                              const RoIThresholdsTool::ThrVec& menuThresholds,
                                              const TrigConf::L1ThrExtraInfoBase& /*menuExtraInfo*/) const {
  // Get RoI properties (once, rather than for every threshold in the menu)
  unsigned int et    = roi.etTOB();
  unsigned int rcore  = roi.tauOneThresholds(); // eTau algorithm agnostic version corresponding to rCoreThresholds or bdtThresholds
  unsigned int rhad  = roi.tauTwoThresholds(); // a.k.a. rHad
  int ieta = roi.iEta();

  uint64_t thresholdMask = 0;
  // Iterate through thresholds and see which ones are passed
  for (const std::shared_ptr<TrigConf::L1Threshold>& thrBase : menuThresholds) {

    auto thr = static_cast<TrigConf::L1Threshold_eTAU*>(thrBase.get());

    // Test ET threshold and core and hadronic ratio codes, set bit in threshold word if conditions met
    if (et > thr->thrValueCounts(ieta) && rcore >= static_cast<unsigned int>(thr->rCore()) && rhad >= static_cast<unsigned int>(thr->rHad()) ) {
      thresholdMask |= (1<<thr->mapping());
    }
  }
  return thresholdMask;
}


