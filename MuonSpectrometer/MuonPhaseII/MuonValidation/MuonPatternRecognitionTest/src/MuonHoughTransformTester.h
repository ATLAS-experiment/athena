/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONVALR4_MuonHoughTransformTester_H
#define MUONVALR4_MuonHoughTransformTester_H

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

#include "TCanvas.h"
///  @brief Lightweight algorithm to read xAOD MDT sim hits and 
///  (fast-digitised) drift circles from SG and fill a 
///  validation NTuple with identifier and drift circle info.

namespace MuonValR4{

  class MuonHoughTransformTester : public AthHistogramAlgorithm {
  public:
    MuonHoughTransformTester(const std::string& name, ISvcLocator* pSvcLocator);
    virtual ~MuonHoughTransformTester()  = default;

    virtual StatusCode initialize() override;
    virtual StatusCode execute() override;
    virtual StatusCode finalize() override;

  private:
    Amg::Transform3D toChamberTrf(const ActsGeometryContext& gctx,
                                  const Identifier& hitId) const;
    StatusCode drawEventDisplay(const EventContext& ctx,
                                const std::vector<const xAOD::MuonSimHit*>& simHits,
                                const MuonR4::HoughSegmentSeed* foundMax) const;
    
    // MDT sim hits in xAOD format 
    SG::ReadHandleKeyArray<xAOD::MuonSimHitContainer> m_inSimHitKeys {this, "SimHitKeys",{ "xMdtSimHits","xRpcSimHits","xTgcSimHits"}, "xAOD  SimHit collections"};
                                                          
    SG::ReadHandleKey<MuonR4::StationHoughMaxContainer> m_inHoughMaximaKey{this, "StationHoughMaxContainer", "MuonHoughStationMaxima"};
    SG::ReadHandleKey<MuonR4::StationHoughSegmentSeedContainer> m_inHoughSegmentSeedKey{this, "StationHoughSegmentSeedContainer", "MuonHoughStationSegmentSeeds"};
    SG::ReadHandleKey<MuonR4::MuonSpacePointContainer> m_spacePointKey{this, "SpacePointContainer", "MuonSpacePoints"};
    
    SG::ReadHandleKey<ActsGeometryContext> m_geoCtxKey{this, "AlignmentKey", "ActsAlignment", "cond handle key"};

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
    MuonVal::ScalarBranch<unsigned int>&   m_out_gen_nsTGCHits{m_tree.newScalar<unsigned int>("genNsTgcHits",0)};
    MuonVal::ScalarBranch<unsigned int>&   m_out_gen_nMMits{m_tree.newScalar<unsigned int>("genNMmHits",0)};
    
    
    MuonVal::ScalarBranch<float>& m_out_gen_tantheta{m_tree.newScalar<float>("genTanTheta", 0.0)}; 
    MuonVal::ScalarBranch<float>& m_out_gen_z0{m_tree.newScalar<float>("genZ0", 0.0)}; 
    MuonVal::ScalarBranch<float>& m_out_gen_tanphi{m_tree.newScalar<float>("genTanPhi", 0.0)}; 
    MuonVal::ScalarBranch<float>& m_out_gen_x0{m_tree.newScalar<float>("genX0", 0.0)}; 
    MuonVal::ScalarBranch<bool>&  m_out_hasMax {m_tree.newScalar<bool>("hasMax", false)}; 
    MuonVal::ScalarBranch<bool>&  m_out_max_hasPhiExtension {m_tree.newScalar<bool>("maxHasPhiExtension", false)}; 
    MuonVal::ScalarBranch<float>& m_out_max_tantheta{m_tree.newScalar<float>("maxTanTheta", 0.0)}; 
    MuonVal::ScalarBranch<float>& m_out_max_z0{m_tree.newScalar<float>("maxZ0", 0.0)}; 
    MuonVal::ScalarBranch<float>& m_out_max_tanphi{m_tree.newScalar<float>("maxTanPhi", 0.0)}; 
    MuonVal::ScalarBranch<float>& m_out_max_x0{m_tree.newScalar<float>("maxX0", 0.0)}; 
    
    MuonVal::ScalarBranch<unsigned int>& m_out_max_nHits{m_tree.newScalar<unsigned int>("maxNHits", 0)}; 
    MuonVal::ScalarBranch<unsigned int>& m_out_max_nEtaHits{m_tree.newScalar<unsigned int>("maxNEtaHits", 0)}; 
    MuonVal::ScalarBranch<unsigned int>& m_out_max_nPhiHits{m_tree.newScalar<unsigned int>("maxNPhiHits", 0)}; 
    MuonVal::ScalarBranch<unsigned int>& m_out_max_nMdt{m_tree.newScalar<unsigned int>("maxNMdtHits", 0)}; 
    MuonVal::ScalarBranch<unsigned int>& m_out_max_nRpc{m_tree.newScalar<unsigned int>("maxNRpcHits", 0)}; 
    MuonVal::ScalarBranch<unsigned int>& m_out_max_nTgc{m_tree.newScalar<unsigned int>("maxNTgcHits", 0)};
    MuonVal::ScalarBranch<unsigned int>& m_out_max_nsTgc{m_tree.newScalar<unsigned int>("maxNsTgcHits", 0)};
    MuonVal::ScalarBranch<unsigned int>& m_out_max_nMm{m_tree.newScalar<unsigned int>("maxNMmHits", 0)};
    
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

    MuonVal::sTgcIdentifierBranch   m_max_stgcHitId{m_tree, "maxsTgcId"};
    MuonVal::ThreeVectorBranch      m_max_stgcHitPos{m_tree,"maxsTgcHitPos"};
    MuonVal::VectorBranch<bool> &   m_max_stgcHitHasPhiMeas{m_tree.newVector<bool>("maxsTgcHasPhiMeas")};
    MuonVal::VectorBranch<float>&   m_max_stgcHitErrorX{m_tree.newVector<float>("maxsTgcEtaMeasError")};
    MuonVal::VectorBranch<float>&   m_max_stgcHitErrorY{m_tree.newVector<float>("maxsTgcPhiMeasError")};

    MuonVal::MmIdentifierBranch     m_max_MmHitId{m_tree, "maxMmId"};
    MuonVal::ThreeVectorBranch      m_max_MmHitPos{m_tree,"maxMmHitPos"};
    MuonVal::VectorBranch<bool> &   m_max_MmHitIsStero{m_tree.newVector<bool>("maxMmIsStero")};
    MuonVal::VectorBranch<float>&   m_max_MmHitErrorX{m_tree.newVector<float>("maxMmEtaMeasError")};
    MuonVal::VectorBranch<float>&   m_max_MmHitErrorY{m_tree.newVector<float>("maxMmPhiMeasError")};


    /// Draw the event display for the cases where the hough transform did not find any hough maximum
    Gaudi::Property<bool> m_drawEvtDisplayFailure{this, "drawDisplayFailed", false};
    /// Draw the event dispalty for the successful cases
    Gaudi::Property<bool> m_drawEvtDisplaySuccess{this, "drawDisplaySuccss", false};
    
    std::unique_ptr<TCanvas> m_allCan{};
    Gaudi::Property<std::string> m_allCanName{this, "AllCanvasName", "AllHoughiDiPuffDisplays.pdf"};

  };
}

#endif // MUONFASTDIGITEST_MUONVALR4_MuonHoughTransformTester_H
