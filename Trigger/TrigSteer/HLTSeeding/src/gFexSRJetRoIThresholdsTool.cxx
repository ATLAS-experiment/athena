/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "gFexSRJetRoIThresholdsTool.h"

uint64_t gFexSRJetRoIThresholdsTool::getPattern(const EventContext& /*ctx*/,
                                                const xAOD::gFexJetRoI& roi,
                                                const RoIThresholdsTool::ThrVec& menuThresholds,
                                                const TrigConf::L1ThrExtraInfoBase& /*menuExtraInfo*/) const {
  float et = roi.et();
  int ieta = roi.menuEta();
  uint64_t thresholdMask = 0;

  for (const std::shared_ptr<TrigConf::L1Threshold>& thrBase : menuThresholds) {
    auto thr = static_cast<TrigConf::L1Threshold_gJ*>(thrBase.get());
    
    if (et > thr->thrValueMeV(ieta)) {
      thresholdMask |= (1<<thr->mapping());
    }
    
  }

  return thresholdMask;
}
