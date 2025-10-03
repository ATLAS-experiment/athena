/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "MURoIThresholdsTool.h"
#include "utilities.h"
#include <memory>
using namespace HLTSeedingNs;

uint64_t MURoIThresholdsTool::getPattern(const EventContext& /*ctx*/,
                                         const xAOD::MuonRoI& roi,
                                         const RoIThresholdsTool::ThrVec& menuThresholds,
                                         const TrigConf::L1ThrExtraInfoBase& /*menuExtraInfo*/) const {
  uint32_t thr_num = static_cast<uint32_t>(roi.getThrNumber());

  uint64_t thresholdsPattern = 0;

  // Iterate through thresholds and see which ones are passed
  for (const std::shared_ptr<TrigConf::L1Threshold>& thrBase : menuThresholds) {
    std::shared_ptr<TrigConf::L1Threshold_MU> thr = std::static_pointer_cast<TrigConf::L1Threshold_MU>(thrBase);

    bool passed{false};

    // e.g. threshold number is simply required.
    if (thr_num >= thr->idxBarrel() || thr_num >= thr->idxEndcap() || thr_num >= thr->idxForward()) {
      passed = true;
    }

    if (passed) {
      // set the corresponding bit in the pattern
      thresholdsPattern |= (1_u64 << thr->mapping());
    }

  } // loop over thresholds

  return thresholdsPattern;
}
