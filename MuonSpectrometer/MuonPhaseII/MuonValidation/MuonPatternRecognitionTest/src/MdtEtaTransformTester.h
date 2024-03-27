/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONFASTDIGITEST_MUONVALR4_MdtEtaTransformTester_H
#define MUONFASTDIGITEST_MUONVALR4_MdtEtaTransformTester_H

// Framework includes
#include "AthenaBaseComps/AthHistogramAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadHandleKeyArray.h"
#include "StoreGate/ReadCondHandleKey.h"

// EDM includes 
#include "xAODMuonSimHit/MuonSimHitContainer.h"
#include "xAODMuonPrepData/MdtDriftCircleContainer.h"
#include "xAODMuonPrepData/RpcStripContainer.h"
#include "xAODMuonPrepData/TgcStripContainer.h"
#include "MuonPatternEvent/StationHoughMaxContainer.h"
#include <MuonReadoutGeometryR4/MuonDetectorManager.h>

// muon includes
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonTesterTree/MuonTesterTree.h"
#include "MuonTesterTree/ThreeVectorBranch.h"
#include "MuonTesterTree/IdentifierBranch.h"


///  @brief Lightweight algorithm to read xAOD MDT sim hits and 
///  (fast-digitised) drift circles from SG and fill a 
///  validation NTuple with identifier and drift circle info.

namespace MuonValR4{

  class MdtEtaTransformTester : public AthHistogramAlgorithm {
  public:
    MdtEtaTransformTester(const std::string& name, ISvcLocator* pSvcLocator);
    virtual ~MdtEtaTransformTester()  = default;

    virtual StatusCode initialize() override;
    virtual StatusCode execute() override;
    virtual StatusCode finalize() override;

  private:
    Amg::Transform3D toChamberTrf(const ActsGeometryContext& gctx,
                                  const Identifier& hitId) const;
    StatusCode drawEventDisplay(const EventContext& ctx,
                                const std::vector<const xAOD::MuonSimHit*>& simHits,
                                const MuonR4::HoughMaximum* foundMax) const;
    
    // MDT sim hits in xAOD format 
    SG::ReadHandleKeyArray<xAOD::MuonSimHitContainer> m_inSimHitKeys {this, "SimHitKeys",{ "xMdtSimHits","xRpcSimHits","xTgcSimHits"}, "xAOD  SimHit collections"};
                                                          
    SG::ReadHandleKey<MuonR4::StationHoughMaxContainer> m_inHoughMaximaKey{this, "StationHoughMaxContainer", "MuonHoughStationMaxima"};
    SG::ReadHandleKey<MuonR4::MuonSpacePointContainer> m_spacePointKey{this, "SpacePointContainer", "MuonSpacePoints"};
    
    SG::ReadCondHandleKey<ActsGeometryContext> m_geoCtxKey{this, "AlignmentKey", "ActsAlignment", "cond handle key"};

    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
    const MuonGMR4::MuonDetectorManager* m_r4DetMgr{nullptr};

    // // output tree - allows to compare the sim and fast-digitised hits
    MuonVal::MuonTesterTree m_tree{"MuonEtaHoughTest","MuonEtaHoughTransformTest"}; 
    MuonVal::ScalarBranch<Long64_t>& m_evtNumber{m_tree.newScalar<Long64_t>("eventNumber")};
    MuonVal::ScalarBranch<int>&   m_out_stationName{m_tree.newScalar<int>("stationName")};
    MuonVal::ScalarBranch<int>&   m_out_stationEta{m_tree.newScalar<int>("stationEta")};
    MuonVal::ScalarBranch<int>&   m_out_stationPhi{m_tree.newScalar<int>("stationPhi")};
    MuonVal::ScalarBranch<float>& m_out_gen_Eta{m_tree.newScalar<float>("genEta")};
    MuonVal::ScalarBranch<float>& m_out_gen_Phi{m_tree.newScalar<float>("genPhi")};
    MuonVal::ScalarBranch<float>& m_out_gen_Pt{m_tree.newScalar<float>("genPt")};
    MuonVal::ScalarBranch<unsigned int>&   m_out_gen_nHits{m_tree.newScalar<unsigned int>("genNHits")};
    MuonVal::ScalarBranch<unsigned int>&   m_out_gen_nRPCHits{m_tree.newScalar<unsigned int>("genNRpcHits",0)};
    MuonVal::ScalarBranch<unsigned int>&   m_out_gen_nMDTHits{m_tree.newScalar<unsigned int>("genNMdtHits",0)};
    MuonVal::ScalarBranch<unsigned int>&   m_out_gen_nTGCHits{m_tree.newScalar<unsigned int>("genNTgcHits",0)};
    
    MuonVal::ScalarBranch<float>& m_out_gen_tantheta{m_tree.newScalar<float>("genTanTheta", 0.0)}; 
    MuonVal::ScalarBranch<float>& m_out_gen_z0{m_tree.newScalar<float>("genZ0", 0.0)}; 
    MuonVal::ScalarBranch<bool>&  m_out_hasMax {m_tree.newScalar<bool>("hasMax", false)}; 
    MuonVal::ScalarBranch<float>& m_out_max_tantheta{m_tree.newScalar<float>("maxTanTheta", 0.0)}; 
    MuonVal::ScalarBranch<float>& m_out_max_z0{m_tree.newScalar<float>("maxZ0", 0.0)}; 
    
    MuonVal::ScalarBranch<unsigned int>& m_out_max_nHits{m_tree.newScalar<unsigned int>("maxNHits", 0)}; 
    MuonVal::ScalarBranch<unsigned int>& m_out_max_nMdt{m_tree.newScalar<unsigned int>("maxNMdtHits", 0)}; 
    MuonVal::ScalarBranch<unsigned int>& m_out_max_nRpc{m_tree.newScalar<unsigned int>("maxNRpcHits", 0)}; 
    MuonVal::ScalarBranch<unsigned int>& m_out_max_nTgc{m_tree.newScalar<unsigned int>("maxNTgcHits", 0)}; 
    /// Dump of the Mdt hits on maximum
    MuonVal::MdtIdentifierBranch    m_max_driftCircleId{m_tree, "maxMdtId"}; 
    MuonVal::VectorBranch<float>&   m_max_driftCirclRadius{m_tree.newVector<float>("maxMdtDriftR")}; 
    MuonVal::ThreeVectorBranch      m_max_driftCircleTubePos{m_tree,"maxMdtTubePos"};
    MuonVal::VectorBranch<float>&   m_max_driftCircleDriftUncert{m_tree.newVector<float>("maxMdtUncertDriftR")};
    MuonVal::VectorBranch<float>&   m_max_driftCircleTubeLength{m_tree.newVector<float>("maxMdtUncertWire")};
    
    
    /// @brief  Branches to access the space points in the maximum
    MuonVal::RpcIdentifierBranch    m_max_rpcHitId{m_tree, "maxRpcId"}; 
    MuonVal::ThreeVectorBranch      m_max_rpcHitPos{m_tree,"maxRpcHitPos"};
    MuonVal::VectorBranch<bool> &   m_max_rpcHitHasPhiMeas{m_tree.newVector<bool>("maxRpcHasPhiMeas")};
    MuonVal::VectorBranch<float>&   m_max_rpcHitErrorX{m_tree.newVector<float>("maxRpcEtaMeasError")};
    MuonVal::VectorBranch<float>&   m_max_rpcHitErrorY{m_tree.newVector<float>("maxRpcPhiMeasError")};
    
    MuonVal::TgcIdentifierBranch    m_max_tgcHitId{m_tree, "maxTgcId"}; 
    MuonVal::ThreeVectorBranch      m_max_tgcHitPos{m_tree,"maxTgcHitPos"};
    MuonVal::VectorBranch<bool> &   m_max_tgcHitHasPhiMeas{m_tree.newVector<bool>("maxTgcHasPhiMeas")};
    MuonVal::VectorBranch<float>&   m_max_tgcHitErrorX{m_tree.newVector<float>("maxTgcEtaMeasError")};
    MuonVal::VectorBranch<float>&   m_max_tgcHitErrorY{m_tree.newVector<float>("maxTgcPhiMeasError")};

  };
}

#endif // MUONFASTDIGITEST_MUONVALR4_MdtEtaTransformTester_H
