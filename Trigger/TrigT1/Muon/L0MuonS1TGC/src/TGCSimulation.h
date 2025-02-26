/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef L0MUONS1TGC_TGCSIMULATION_H
#define L0MUONS1TGC_TGCSIMULATION_H 

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "AthenaMonitoringKernel/Monitored.h"

#include "MuonDigitContainer/TgcDigitContainer.h"

#include "xAODTrigger/MuonRoIContainer.h"

#include "MuonTGC_Cabling/MuonTGC_CablingSvc.h"   // TODO: to be updated for phase-II

namespace L0Muon {

class TGCSimulation : public ::AthReentrantAlgorithm { 
 public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;
  virtual ~TGCSimulation() = default;

  virtual StatusCode  initialize() override;
  virtual StatusCode  execute(const EventContext& ctx) const override;

 private:
  /// RPC Digit container
  SG::ReadHandleKey<TgcDigitContainer> m_keyTgcDigit{this, "InputDigit", "TGC_DIGITS", "Location of input TgcDigitContainer"};
  /// Output RoIs
  SG::WriteHandleKey<xAOD::MuonRoIContainer> m_outputMuonRoIKey{this, "L0MuonEndcapKey", "L0MuonEndcapRoI", "key for LVL0 Muon RoIs in the barrel" };

  /// TGC cabling map
  ServiceHandle<MuonTGC_CablingSvc> m_cabling{this, "TGCCablingSvc", "MuonTGC_CablingSvc","Key of MuonTGC_CablingSvc"};

  ToolHandle<GenericMonitoringTool> m_monTool{this, "MonTool", "", "Monitoring Tool"};
  
};

}   // end of namespace

#endif  // L0MUONS1TGC_TGCSIMULATION_H

