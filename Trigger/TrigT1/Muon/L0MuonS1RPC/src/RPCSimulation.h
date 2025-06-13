/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef L0MuonS1RPC_RPCSIMULATION_H
#define L0MuonS1RPC_RPCSIMULATION_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "AthenaMonitoringKernel/Monitored.h"
#include "AthenaKernel/IAthRNGSvc.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "StoreGate/ReadHandleKey.h"
#include "xAODMuonRDO/NRPCRDOContainer.h"
#include "xAODTrigger/MuonRoIContainer.h"
#include "L0MuonInterface/BarrelCandDataContainer.h"
#include "MuonReadoutGeometryR4/MuonDetectorManager.h"
#include "MuonDigitContainer/RpcDigitContainer.h"
#include "GeneratorObjects/McEventCollection.h"
#include "AsgTools/PropertyWrapper.h"
#include "xAODMuonSimHit/MuonSimHitContainer.h"
#include <ActsGeometryInterfaces/ActsGeometryContext.h>

namespace L0Muon
{

  class RPCSimulation : public ::AthReentrantAlgorithm
  {
  public:
    using AthReentrantAlgorithm::AthReentrantAlgorithm;
    virtual ~RPCSimulation() = default;

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext &ctx) const override;

  private:
    /// build the candidates from the MC truth
    StatusCode buildFromTruth(SG::WriteHandle<L0Muon::BarrelCandDataContainer>
                                  outputCands,
                              const EventContext &ctx) const;


    /// configuration options
    Gaudi::Property<bool> m_useTruth{this, "UseTruth", true, "Use truth information to build candidates"};
    Gaudi::Property<bool> m_usePatterns{this, "UsePatterns", false, "Use patterns to build candidates"};

    /// RPC Rdo
    SG::ReadHandleKey<xAOD::NRPCRDOContainer> m_keyRpcRdo{this, "NrpcRdoKey", "NRPCRDO", "Location of input RpcRDO"};

    /// Output Trigger candidates
    SG::WriteHandleKey<L0Muon::BarrelCandDataContainer> m_outputCandKey{this, "L0MuonBarrelCandKey", "L0MuonBarrelCand",
                                                                        "LVL0 trigger candidates in the Muon Barrel"};
    ToolHandle<GenericMonitoringTool> m_monTool{this, "MonTool", "", "Monitoring Tool"};

    /// truth containers
    SG::ReadHandleKey<McEventCollection> m_mcEventCollectionKey{this, "TruthEventKey", "TruthEvent"};
    SG::ReadHandleKey<xAOD::MuonSimHitContainer> m_simHitContainerKey{this, "RPC_SDO", "RPC_SDO",
                                                                       "RPC SimHit container key"};
 
    /// helper service
    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc",
                                                         "Muon Id Helper Service"};  
    SG::ReadHandleKey<ActsGeometryContext> m_geoCtxKey{this, "AlignmentKey", "ActsAlignment", "cond handle key"};
    const MuonGMR4::MuonDetectorManager* m_detMgr{nullptr};       
  };

} // end of namespace

#endif // L0MUONRPCSIM_H
