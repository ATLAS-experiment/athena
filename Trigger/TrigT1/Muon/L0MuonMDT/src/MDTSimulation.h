/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef L0MuonMDT_MDTSIMULATION_H
#define L0MuonMDT_MDTSIMULATION_H 

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "AthenaMonitoringKernel/Monitored.h"
#include "AthenaKernel/IAthRNGSvc.h"

#include "StoreGate/ReadHandleKey.h"
#include "MuonRDO/MdtCsmContainer.h"
#include "xAODTrigger/MuonRoIContainer.h"
#include "MuonCablingData/MuonMDT_CablingMap.h"

namespace L0Muon {

class MDTSimulation: public ::AthReentrantAlgorithm {
 public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;
  virtual ~MDTSimulation() = default;

  virtual StatusCode  initialize() override;
  virtual StatusCode  execute(const EventContext& ctx) const override;

 private:
  /// MDT Rdo
  SG::ReadHandleKey<MdtCsmContainer> m_keyMdtRdo{this,"MdtRdoKey","MDTCSM", "Mdt RDO Input"};
  /// Output RoIs
  SG::WriteHandleKey<xAOD::MuonRoIContainer> m_outputMuonRoIKey{this, "L0MuonBarrelKey", "L0MuonBarrelRoI",
    "key for LVL0 Muon RoIs in the barrel" };

  
  /// MDT cabling map
  SG::ReadCondHandleKey<MuonMDT_CablingMap> m_cablingKey{this, "CablingKey", "MuonMDT_CablingMap","Key of MuonMDT_CablingMap"};

  ToolHandle<GenericMonitoringTool> m_monTool{this, "MonTool", "", "Monitoring Tool"};

};

}   // end of namespace

#endif  // L0MuonMDT_MDTSIMULATION_H
