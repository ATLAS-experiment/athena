/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef L1MuonS1RPC_RPCSIMULATION_H
#define L1MuonS1RPC_RPCSIMULATION_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "AthenaMonitoringKernel/Monitored.h"
#include "AthenaKernel/IAthRNGSvc.h"

#include "MuonIdHelpers/IMuonIdHelperSvc.h"

#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadDecorHandleKey.h"

#include "xAODTruth/TruthParticleContainer.h"
#include "xAODMuonRDO/NRPCRDOContainer.h"
#include "xAODTrigger/MuonRoIContainer.h"
#include "xAODMuonSimHit/MuonSimHit.h"

#include "xAODL0MuonCand/RPCCandDataContainer.h"
#include "xAODL0MuonCand/RPCCandDataAuxContainer.h"
#include "MuonReadoutGeometryR4/MuonDetectorManager.h"

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
    StatusCode buildFromTruth(xAOD::RPCCandDataContainer& outputCands,
                              const EventContext &ctx) const;

    std::vector<const xAOD::MuonSimHit*> collectHits(const xAOD::TruthParticle& truthPart,
                                                     const EventContext& ctx) const;


    /// configuration options
    Gaudi::Property<bool> m_useTruth{this, "UseTruth", true, "Use truth information to build candidates"};
    Gaudi::Property<bool> m_usePatterns{this, "UsePatterns", false, "Use patterns to build candidates"};

    /// RPC Rdo
    SG::ReadHandleKey<xAOD::NRPCRDOContainer> m_keyRpcRdo{this, "NrpcRdoKey", "NRPCRDO", "Location of input RpcRDO"};

    /// Output Trigger candidates
    SG::WriteHandleKey<xAOD::RPCCandDataContainer> m_outputCandKey{this, "RPCCandKey", "RPCCandData",
                                                                         "LVL0 xAOD Barrel trigger candidates in the Muon RPC"};
    ToolHandle<GenericMonitoringTool> m_monTool{this, "MonTool", "", "Monitoring Tool"};

    /// truth containers
    SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthPartKey{this, "TruthPartKey", "MuonTruthParticles"};

    SG::ReadDecorHandleKey<xAOD::TruthParticleContainer> m_segmentLinkKey{this, "SegmentLinkKey", m_truthPartKey, 
                                                                          "truthSegmentLinks"};
    
    /// helper service
    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc",
                                                         "Muon Id Helper Service"};  
    SG::ReadHandleKey<ActsTrk::GeometryContext> m_geoCtxKey{this, "AlignmentKey", "ActsAlignment", "cond handle key"};
    const MuonGMR4::MuonDetectorManager* m_detMgr{nullptr};       
  };

} // end of namespace

#endif // L0MUONRPCSIM_H
