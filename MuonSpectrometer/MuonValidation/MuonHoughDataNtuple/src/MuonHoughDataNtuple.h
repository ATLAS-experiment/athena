/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef HOUGHIDITESTER_H 
#define HOUGHIDITESTER_H 

// Framework includes
#include "AthenaBaseComps/AthHistogramAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"

// EDM includes
#include "MuonRecToolInterfaces/HoughDataPerSec.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"

#include "xAODMuon/MuonSegmentContainer.h"
#include "xAODTruth/TruthParticle.h"
#include "xAODTruth/TruthParticleContainer.h"

// Muon includes
#include "MuonTesterTree/MuonTesterTree.h"
#include "MuonTesterTree/TwoVectorBranch.h"
#include "MuonTesterTree/ThreeVectorBranch.h"
#include "MuonTesterTree/EventInfoBranch.h"
#include "MuonTesterTree/IdentifierBranch.h"
#include "MuonLayerHough/MuonLayerHough.h"

/**
 * @class MuonHoughDataNtuple
 * @brief Register all hit and truth info into a ttree
 **/
class MuonHoughDataNtuple : public AthHistogramAlgorithm {
public:
  MuonHoughDataNtuple(const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~MuonHoughDataNtuple() override;

  virtual StatusCode initialize() override;
  virtual StatusCode execute() override;
  virtual StatusCode finalize() override;

private:
  int channel(const Identifier& measId) const;
  int gasGap(const Identifier& measId) const;
  // truth info from xAOD 
  SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthMuonKey{
    this, "MuonTruthParticlesKey", "MuonTruthParticles"};
  SG::ReadHandleKey<xAOD::MuonSegmentContainer> m_truthSegmentsKey{
    this, "MuonTruthSegmentsKey", "MuonTruthSegments"};
  // Hough Data
  SG::ReadHandleKey<Muon::HoughDataPerSectorVec> m_houghDataPerSectorVecKey{this, "HoughKey", "HoughDataPerSectorVec"};
  ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

  // eta hit output tree 
  MuonVal::MuonTesterTree m_eta_hit_tree{"EtaHitHoughData","MuonHoughDataNtuple"}; 
  // hough maximum eta hit info
  MuonVal::ScalarBranch<float>& m_maxHit_sector{m_eta_hit_tree.newScalar<float>("maxHit_Sector")}; 
  MuonVal::ScalarBranch<float>& m_maxHit_z0{m_eta_hit_tree.newScalar<float>("maxHit_Z0")}; 
  MuonVal::ScalarBranch<float>& m_maxHit_theta{m_eta_hit_tree.newScalar<float>("maxHit_Theta")}; 
  MuonVal::ScalarBranch<int>&   m_maxHit_region{m_eta_hit_tree.newScalar<int>("maxHit_Region")}; 
  // station index/eta/phi
  MuonVal::ScalarBranch<int>& m_maxHit_stationIndex{m_eta_hit_tree.newScalar<int>("hit_StationIndex", -999)}; 
  MuonVal::ScalarBranch<int>& m_maxHit_stationEta{m_eta_hit_tree.newScalar<int>("hit_StationEta", -999)}; 
  MuonVal::ScalarBranch<int>& m_maxHit_stationPhi{m_eta_hit_tree.newScalar<int>("hit_StationPhi", -999)}; 

  // eta hit info from hough maximum
  MuonVal::VectorBranch<float>& m_hit_x{m_eta_hit_tree.newVector<float>("hit_X")}; 
  MuonVal::VectorBranch<float>& m_hit_ymin{m_eta_hit_tree.newVector<float>("hit_YMin")}; 
  MuonVal::VectorBranch<float>& m_hit_ymax{m_eta_hit_tree.newVector<float>("hit_YMax")}; 
  MuonVal::VectorBranch<float>& m_hit_w{m_eta_hit_tree.newVector<float>("hit_W")}; 
  // eta hit debug info
  MuonVal::VectorBranch<int>&   m_hit_tech{m_eta_hit_tree.newVector<int>("hit_Tech")}; 

  // local position info from associated PrepRawData
  MuonVal::TwoVectorBranch m_hit_local{m_eta_hit_tree, "hit_Local"}; 
  MuonVal::ThreeVectorBranch m_hit_global{m_eta_hit_tree, "hit_Global"}; 

  // record info from identifier and do truth matching
  MuonVal::MdtIdentifierBranch m_hit_mdtId{m_eta_hit_tree, "hit_MDTId"};
  MuonVal::CscIdentifierBranch m_hit_cscId{m_eta_hit_tree, "hit_CSCId"};
  MuonVal::TgcIdentifierBranch m_hit_tgcId{m_eta_hit_tree, "hit_TGCId"};
  MuonVal::RpcIdentifierBranch m_hit_rpcId{m_eta_hit_tree, "hit_RPCId"};
  MuonVal::TgcIdentifierBranch m_hit_mmId{m_eta_hit_tree, "hit_MMId"};
  MuonVal::TgcIdentifierBranch m_hit_stgcId{m_eta_hit_tree, "hit_STGCId"};

  MuonVal::VectorBranch<int>& m_hit_mdtIndex{m_eta_hit_tree.newVector<int>("hit_MDTIndex")}; 
  MuonVal::VectorBranch<int>& m_hit_cscIndex{m_eta_hit_tree.newVector<int>("hit_CSCIndex")}; 
  MuonVal::VectorBranch<int>& m_hit_tgcIndex{m_eta_hit_tree.newVector<int>("hit_TGCIndex")}; 
  MuonVal::VectorBranch<int>& m_hit_rpcIndex{m_eta_hit_tree.newVector<int>("hit_RPCIndex")}; 
  MuonVal::VectorBranch<int>& m_hit_mmIndex{m_eta_hit_tree.newVector<int>("hit_MMIndex")}; 
  MuonVal::VectorBranch<int>& m_hit_stgcIndex{m_eta_hit_tree.newVector<int>("hit_STGCIndex")}; 
  
  MuonVal::VectorBranch<bool>& m_hit_truthMatched{m_eta_hit_tree.newVector<bool>("hit_TruthMatched")}; 

  // phi hit output tree 
  MuonVal::MuonTesterTree m_phi_hit_tree{"PhiHitHoughData","MuonHoughDataNtuple"}; 
  // hough maximum phi hit info
  MuonVal::ScalarBranch<float>& m_maxPhiHit_sector{m_phi_hit_tree.newScalar<float>("maxPhiHit_Sector")}; 
  MuonVal::ScalarBranch<float>& m_maxPhiHit_z0{m_phi_hit_tree.newScalar<float>("maxPhiHit_Z0")}; 
  // station index/eta/phi
  MuonVal::ScalarBranch<int>& m_maxPhiHit_stationIndex{m_phi_hit_tree.newScalar<int>("phiHit_StationIndex", -999)}; 
  MuonVal::ScalarBranch<int>& m_maxPhiHit_stationEta{m_phi_hit_tree.newScalar<int>("phiHit_StationEta", -999)}; 
  MuonVal::ScalarBranch<int>& m_maxPhiHit_stationPhi{m_phi_hit_tree.newScalar<int>("phiHit_StationPhi", -999)}; 

  // phi hit info from hough maximum
  MuonVal::VectorBranch<float>& m_phiHit_x{m_phi_hit_tree.newVector<float>("phiHit_X")}; 
  MuonVal::VectorBranch<float>& m_phiHit_ymin{m_phi_hit_tree.newVector<float>("phiHit_YMin")}; 
  MuonVal::VectorBranch<float>& m_phiHit_ymax{m_phi_hit_tree.newVector<float>("phiHit_YMax")}; 
  MuonVal::VectorBranch<float>& m_phiHit_w{m_phi_hit_tree.newVector<float>("phiHit_W")}; 
  // phi hit debug info
  MuonVal::VectorBranch<int>&   m_phiHit_tech{m_phi_hit_tree.newVector<int>("phiHit_Tech")}; 

  // local position info from associated PrepRawData
  MuonVal::TwoVectorBranch m_phiHit_local{m_phi_hit_tree, "phiHit_Local"}; 
  MuonVal::ThreeVectorBranch m_phiHit_global{m_phi_hit_tree, "phiHit_Global"}; 

  // record info from identifier and do truth matching
  MuonVal::MdtIdentifierBranch m_phiHit_mdtId{m_phi_hit_tree, "phiHit_MDTId"};
  MuonVal::CscIdentifierBranch m_phiHit_cscId{m_phi_hit_tree, "phiHit_CSCId"};
  MuonVal::TgcIdentifierBranch m_phiHit_tgcId{m_phi_hit_tree, "phiHit_TGCId"};
  MuonVal::RpcIdentifierBranch m_phiHit_rpcId{m_phi_hit_tree, "phiHit_RPCId"};
  MuonVal::TgcIdentifierBranch m_phiHit_mmId{m_phi_hit_tree, "phiHit_MMId"};
  MuonVal::TgcIdentifierBranch m_phiHit_stgcId{m_phi_hit_tree, "phiHit_STGCId"};

  MuonVal::VectorBranch<int>& m_phiHit_mdtIndex{m_phi_hit_tree.newVector<int>("phiHit_MdtIndex")}; 
  MuonVal::VectorBranch<int>& m_phiHit_cscIndex{m_phi_hit_tree.newVector<int>("phiHit_CscIndex")}; 
  MuonVal::VectorBranch<int>& m_phiHit_tgcIndex{m_phi_hit_tree.newVector<int>("phiHit_TgcIndex")}; 
  MuonVal::VectorBranch<int>& m_phiHit_rpcIndex{m_phi_hit_tree.newVector<int>("phiHit_RpcIndex")}; 
  MuonVal::VectorBranch<int>& m_phiHit_mmIndex{m_phi_hit_tree.newVector<int>("phiHit_MMIndex")}; 
  MuonVal::VectorBranch<int>& m_phiHit_stgcIndex{m_phi_hit_tree.newVector<int>("phiHit_STGCIndex")}; 
  
  MuonVal::VectorBranch<bool>& m_phiHit_truthMatched{m_phi_hit_tree.newVector<bool>("phiHit_TruthMatched")}; 
                                                   
  // truth output tree 
  MuonVal::MuonTesterTree m_truth_tree{"TruthHoughData","MuonHoughDataNtuple"}; 
  // muon truth info
  MuonVal::ScalarBranch<int>& m_truth_pdgId{m_truth_tree.newScalar<int>("pdgId", -999)}; 
  MuonVal::ScalarBranch<int>& m_truth_barcode{m_truth_tree.newScalar<int>("barcode", -999)}; 

  MuonVal::ScalarBranch<float>& m_truth_pt{m_truth_tree.newScalar<float>("truth_Pt", -999)}; 
  MuonVal::ScalarBranch<float>& m_truth_eta{m_truth_tree.newScalar<float>("truth_Eta", -999)}; 
  MuonVal::ScalarBranch<float>& m_truth_phi{m_truth_tree.newScalar<float>("truth_Phi", -999)}; 

  // muon truth segment info
  MuonVal::ThreeVectorBranch m_truth_seg_pos{m_truth_tree, "truth_seg_Pos"};
  MuonVal::ThreeVectorBranch m_truth_seg_p{m_truth_tree, "truth_seg_P"};

  MuonVal::VectorBranch<int>& m_truth_seg_nPrecisionHits{m_truth_tree.newVector<int>("truth_seg_NPrecisionHits")};
  MuonVal::VectorBranch<int>& m_truth_seg_nTriggerHits{m_truth_tree.newVector<int>("truth_seg_NTriggerHits")};
  MuonVal::VectorBranch<int>& m_truth_seg_sector{m_truth_tree.newVector<int>("truth_seg_Sector")};
};

#endif // HOUGHIDITESTER_H
