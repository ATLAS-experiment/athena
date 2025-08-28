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
#include "StoreGate/ReadDecorHandleKey.h"

#include "xAODTruth/TruthParticleContainer.h"
#include "xAODMuonRDO/NRPCRDOContainer.h"
#include "xAODTrigger/MuonRoIContainer.h"
#include "xAODMuonSimHit/MuonSimHit.h"

#include "L0MuonInterface/RPCCandDataContainer.h"
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
    StatusCode buildFromTruth(L0Muon::RPCCandDataContainer& outputCands,
                              const EventContext &ctx) const;

    std::vector<const xAOD::MuonSimHit*> collectHits(const xAOD::TruthParticle& truthPart,
                                                     const EventContext& ctx) const;


    /// configuration options
    Gaudi::Property<bool> m_useTruth{this, "UseTruth", true, "Use truth information to build candidates"};
    Gaudi::Property<bool> m_usePatterns{this, "UsePatterns", false, "Use patterns to build candidates"};

    /// RPC Rdo
    SG::ReadHandleKey<xAOD::NRPCRDOContainer> m_keyRpcRdo{this, "NrpcRdoKey", "NRPCRDO", "Location of input RpcRDO"};

    /// Output Trigger candidates
    SG::WriteHandleKey<L0Muon::RPCCandDataContainer> m_outputCandKey{this, "L0MuonRPCCandKey", "L0MuonRPCCand",
                                                                        "LVL0 Barrel trigger candidates in the Muon RPC"};
    ToolHandle<GenericMonitoringTool> m_monTool{this, "MonTool", "", "Monitoring Tool"};

    /// truth containers
    SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthPartKey{this, "TruthPartKey", "MuonTruthParticles"};

    SG::ReadDecorHandleKey<xAOD::TruthParticleContainer> m_segmentLinkKey{this, "SegmentLinkKey", m_truthPartKey, 
                                                                          "truthSegmentLinks"};
    
    /// helper service
    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc",
                                                         "Muon Id Helper Service"};  
    SG::ReadHandleKey<ActsGeometryContext> m_geoCtxKey{this, "AlignmentKey", "ActsAlignment", "cond handle key"};
    const MuonGMR4::MuonDetectorManager* m_detMgr{nullptr};       
  };

} // end of namespace

#endif // L0MUONRPCSIM_H
