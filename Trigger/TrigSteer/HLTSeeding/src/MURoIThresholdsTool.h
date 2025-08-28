/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef HLTSEEDING_MUROITHRESHOLDSTOOL_H
#define HLTSEEDING_MUROITHRESHOLDSTOOL_H

#include "HLTSeedingRoIToolDefs.h"
#include "HLTSeeding/IRoIThresholdsTool.h"
#include "xAODTrigger/MuonRoI.h"

class MURoIThresholdsTool : public HLTSeedingRoIToolDefs::Muon::ThresholdBaseClass {
 public:
  MURoIThresholdsTool(const std::string& type, const std::string& name, const IInterface* parent)
  : HLTSeedingRoIToolDefs::Muon::ThresholdBaseClass(type, name, parent) {}

  virtual uint64_t getPattern(const EventContext& ctx,
                              const xAOD::MuonRoI& roi,
                              const ThrVec& menuThresholds,
                              const TrigConf::L1ThrExtraInfoBase& menuExtraInfo) const override;
};

#endif // HLTSEEDING_MUROITHRESHOLDSTOOL_H
