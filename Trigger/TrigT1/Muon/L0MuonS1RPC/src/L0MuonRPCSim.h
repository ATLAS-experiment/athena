/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef L0MUONRPCSIM_H
#define L0MUONRPCSIM_H 

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "AthenaMonitoringKernel/Monitored.h"
#include "AthenaKernel/IAthRNGSvc.h"

#include "StoreGate/ReadHandleKey.h"
#include "xAODMuonRDO/NRPCRDOContainer.h"
#include "xAODTrigger/MuonRoIContainer.h"
#include "MuonDigitContainer/RpcDigitContainer.h"
#include "MuonCablingData/RpcCablingMap.h"

namespace L0Muon {

class L0MuonRPCSim: public ::AthReentrantAlgorithm { 
 public: 
  L0MuonRPCSim(const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~L0MuonRPCSim();

  virtual StatusCode  initialize() override;
  virtual StatusCode  execute(const EventContext& ctx) const override;
  virtual StatusCode  finalize() override;

 private:

  /// RPC Digit container
  SG::ReadHandleKey<RpcDigitContainer> m_keyRpcDigit{this,"InputDigit","RpcDigitContainer","Location of input RpcDigitContainer"};
  /// RPC Rdo
  SG::ReadHandleKey<xAOD::NRPCRDOContainer> m_keyRpcRdo{this,"NrpcRdoKey","NRPCRDO","Location of input RpcRDO"};
  /// Output RoIs
  SG::WriteHandleKey<xAOD::MuonRoIContainer> m_outputMuonRoIKey{this, "L0MuonBarrelKey", "L0MuonBarrelRoI",
    "key for LVL0 Muon RoIs in the barrel" };
  

  /// NRPC cabling map
  SG::ReadCondHandleKey<Muon::RpcCablingMap> m_cablingKey{this, "CablingKey", "MuonNRPC_CablingMap","Key of MuonNRPC_CablingMap"};

  ToolHandle<GenericMonitoringTool> m_monTool{this, "MonTool", "", "Monitoring Tool"};
  
};

}   // end of namespace

#endif  // L0MUONRPCSIM_H

