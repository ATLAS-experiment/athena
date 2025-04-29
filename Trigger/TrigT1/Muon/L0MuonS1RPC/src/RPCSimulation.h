/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef L0MuonS1RPC_RPCSIMULATION_H
#define L0MuonS1RPC_RPCSIMULATION_H 

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "AthenaMonitoringKernel/Monitored.h"
#include "AthenaKernel/IAthRNGSvc.h"

#include "StoreGate/ReadHandleKey.h"
#include "xAODMuonRDO/NRPCRDOContainer.h"
#include "xAODTrigger/MuonRoIContainer.h"
#include "L0MuonInterface/BarrelCandDataContainer.h"
#include "MuonDigitContainer/RpcDigitContainer.h"
#include "MuonCablingData/RpcCablingMap.h"

namespace L0Muon {

class RPCSimulation: public ::AthReentrantAlgorithm { 
 public: 
  using AthReentrantAlgorithm::AthReentrantAlgorithm;
  virtual ~RPCSimulation() = default;

  virtual StatusCode  initialize() override;
  virtual StatusCode  execute(const EventContext& ctx) const override;

 private:
  /// RPC Rdo
  SG::ReadHandleKey<xAOD::NRPCRDOContainer> m_keyRpcRdo{this,"NrpcRdoKey","NRPCRDO","Location of input RpcRDO"};
  /// Output RoIs
  SG::WriteHandleKey<xAOD::MuonRoIContainer> m_outputMuonRoIKey{this, "L0MuonBarrelKey", "L0MuonBarrelRoI",
    "key for LVL0 Muon RoIs in the barrel" };
  
  /// Output RoIs
  SG::WriteHandleKey<L0Muon::BarrelCandDataContainer> m_outputCandKey{this, "L0MuonBarrelCandKey", "L0MuonBarrelCand",
    "LVL0 trigger candidates in the Muon Barrel" };
  

  /// NRPC cabling map
  SG::ReadCondHandleKey<Muon::RpcCablingMap> m_cablingKey{this, "CablingKey", "MuonNRPC_CablingMap","Key of MuonNRPC_CablingMap"};

  ToolHandle<GenericMonitoringTool> m_monTool{this, "MonTool", "", "Monitoring Tool"};
  
};

}   // end of namespace

#endif  // L0MUONRPCSIM_H

