/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TBREC_TBTREE_CALOCLUSTERH6_H
#define TBREC_TBTREE_CALOCLUSTERH6_H

// 
// TBTree_CaloClusterH6:
// Make ROOT TTree for CaloClusters for H6 CBT
//
#include "AthenaBaseComps/AthAlgorithm.h"
#include "CaloConditions/CaloNoise.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "xAODCaloEvent/CaloCluster.h"

#include <TRandom.h>

class TFile;
class TTree;
class CaloCell_ID;
class IToolSvc;
class LArDigitContainer;
class ILArPedestal;

class TBTree_CaloClusterH6: public AthAlgorithm {
 public:    
  
  TBTree_CaloClusterH6(const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~TBTree_CaloClusterH6();
  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) override;
  virtual StatusCode finalize() override;
  
 private: 
  void clear();
  /** Get Xcryo and Ytable from a text file */
  StatusCode getXcryoYtable(float &x, float &y, float &eBeam);
  
  Gaudi::Property<std::string> m_suffix{this, "Suffix", "", "Suffix for the cluster/cell container names"};
  int m_nEvent{0};                     // counter
  int m_nEventRejected{0};             // counter
  int m_nEventAccepted{0};             // counter
  int m_nEventRandomTrigger{0};        // counter
  Gaudi::Property<bool> m_addMoments{this, "addMoments", false, "Add cluster moments"};
  Gaudi::Property<bool> m_addGain{this, "addGain", false, "Add cell gain"};
  Gaudi::Property<bool> m_addTime{this, "addTime", false, "Add cell time"};
  Gaudi::Property<bool> m_addQuality{this, "addQuality", false, "Add cell quality"};
  Gaudi::Property<bool> m_addBeamTrack{this, "addBeamTrack", false, "Add beam track parameters"};
  Gaudi::Property<bool> m_addWTC{this, "addWTC", false, "Add warm TailCatcher data"};
  Gaudi::Property<bool> m_useEMTBCluster{this, "useEMTBCluster", false, "Use EM TB clusters"};
  bool m_first{true};             // First event flag

  // Run header
  int m_nRun{0};               /** Run number */
  float m_beamMom{0};          /** Beam momentum */
  float m_xCryo{-9999};            /** CryoX */
  float m_yTable{-9999};           /** TableY */
  Gaudi::Property<float> m_zCalo{this, "zCalo", 30000, "z-coordinate of the calorimeter surface for beam coordinates"};            /** z-coordinate of the calorimeter surface
                                at which beam coordinates calculated */

  // Variables to be in the TTree
  //--------------------------
  // Event type
  unsigned short m_evType{0};
  // Parameters of the sum of clusters (TB particle)
  float m_eTotal{0};
  float m_etaTotal{0};
  float m_phiTotal{0};
  float m_eEME2{0};
  float m_eEME3{0};
  float m_eHEC0{0};
  float m_eHEC1{0};
  float m_eHEC2{0};
  float m_eFCAL0{0};
  float m_eFCAL1{0};
  float m_eFCAL2{0};
  // Claster parameters
  int m_nClusters{0};
  std::vector<int>* m_nCellCluster{nullptr};
  std::vector<float>* m_eCluster{nullptr};
  std::vector<float>* m_etaCluster{nullptr}; 
  std::vector<float>* m_phiCluster{nullptr}; 
  // Cluster moments
  std::vector<float>* m_m1_eta{nullptr};
  std::vector<float>* m_m1_phi{nullptr};
  std::vector<float>* m_m2_r{nullptr};
  std::vector<float>* m_m2_lambda{nullptr};
  std::vector<float>* m_delta_phi{nullptr};
  std::vector<float>* m_delta_theta{nullptr};
  std::vector<float>* m_delta_alpha{nullptr};
  std::vector<float>* m_center_x{nullptr};
  std::vector<float>* m_center_y{nullptr};
  std::vector<float>* m_center_z{nullptr};
  std::vector<float>* m_center_lambda{nullptr};
  std::vector<float>* m_lateral{nullptr};
  std::vector<float>* m_longitudinal{nullptr};
  std::vector<float>* m_eng_frac_em{nullptr};
  std::vector<float>* m_eng_frac_max{nullptr};
  std::vector<float>* m_eng_frac_core{nullptr};
  std::vector<float>* m_m1_dens{nullptr};
  std::vector<float>* m_m2_dens{nullptr};
  // Cell parameters
  int m_nCells{0};
  std::vector<int>* m_cell_id{nullptr};
  std::vector<int>* m_cell_ind_cluster{nullptr};
  std::vector<float>* m_cell_energy{nullptr};
  std::vector<int>* m_cell_gain{nullptr};
  std::vector<float>* m_cell_time{nullptr};
  std::vector<float>* m_cell_quality{nullptr};
  
  // Beam track parameters
  float m_beam_coor_x{0};
  float m_beam_coor_y{0};
  float m_beam_chi2_x{0};
  float m_beam_chi2_y{0};
  float m_beam_intercept_x{0};
  float m_beam_intercept_y{0};
  float m_beam_slope_x{0};
  float m_beam_slope_y{0};

  // Warm TailCatcher data: one word per layer (6 layers at all)
  std::vector<short>* m_wtcNOverflow{nullptr};
  std::vector<float>* m_wtcSignal{nullptr};

  // Names and pointers
  Gaudi::Property<std::string> m_caloCellContainerName{this, "CellContainer", "AllCalo", "Cell container name"};    // Cell container name
  Gaudi::Property<std::string> m_clusterContainerName{this, "ClusterContainer", "TBClusters", "Cluster container name"};     // Cluster container
  std::string m_WTCContainerName{"TailCatcher"};         // Warm TailCatcher
  Gaudi::Property<std::string> m_TBTreeName{this, "TBTreeName", "TBH6Tree", "TBTree name"};               // TBTree name
  Gaudi::Property<std::string> m_rootfile_name{this, "RootFileName", "tbh6tree.root", "Name of the ROOT file with TBTree"};            // name of the ROOT file with TBTree
  TFile* m_rootfile{nullptr};                      // and its pointer
  TTree* m_tree{nullptr};                          // TBTree pointer

  const CaloCell_ID* m_calo_id{nullptr};

  SG::ReadCondHandleKey<CaloNoise> m_elecNoiseKey
    { this, "ElecNoiseKey", "electronicNoise", "SG key for electronic noise" };

  /** Text file containing xCryo and yTable */
  std::string m_txtFileWithXY{"xcryo_ytable.txt"};
};

#endif
