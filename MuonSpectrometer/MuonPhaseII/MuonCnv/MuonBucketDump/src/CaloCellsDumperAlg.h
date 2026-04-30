/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONBUCKETDUMP_CaloCellsDumperAlg_H
#define MUONBUCKETDUMP_CaloCellsDumperAlg_H

#include "AthenaBaseComps/AthHistogramAlgorithm.h"

#include "StoreGate/ReadHandleKey.h"
#include "CaloEvent/CaloCellContainer.h"
#include "CaloEvent/CaloTowerContainer.h"
#include "CaloIdentifier/CaloCell_ID.h"

#include "MuonTesterTree/MuonTesterTree.h"
#include "MuonTesterTree/EventHashBranch.h"
#include "MuonTesterTree/IdentifierBranch.h"
#include "MuonTesterTree/ThreeVectorBranch.h"

namespace MuonR4 {

class CaloCellsDumperAlg : public AthHistogramAlgorithm {
 public:
  using AthHistogramAlgorithm::AthHistogramAlgorithm;
  ~CaloCellsDumperAlg() = default;

  StatusCode initialize() override final;
  StatusCode execute() override final;
  StatusCode finalize() override final;

 private:
  SG::ReadHandleKey<CaloCellContainer> m_cellKey{this, "CellContainerKey", "AllCalo",
    "Input CaloCellContainer key (e.g. AllCalo)"};
  SG::ReadHandleKey<CaloTowerContainer> m_towerKey{this, "TowerContainerKey", "CombinedTower",
    "Input CaloTowerContainer key (e.g. CombinedTower)"};
  Gaudi::Property<float> m_minE{this, "MinCellEnergyMeV", 0.0,
    "Minimum cell energy in MeV to be stored"};
  Gaudi::Property<int> m_maxCells{this, "MaxCells", -1,
    "Maximum number of cells to store per event (-1 = no cap)"};
  Gaudi::Property<float> m_minTowerE{this, "MinTowerEnergyMeV", 0.0,
    "Minimum tower energy in MeV to be stored"};  
  Gaudi::Property<int> m_maxTowers{this, "MaxTowers", -1,
    "Maximum number of towers to store per event (-1 = no cap)"};

  const CaloCell_ID* m_caloId{nullptr};

  MuonVal::MuonTesterTree m_tree{"CaloDump", "MuonBucketDump"};

  std::shared_ptr<MuonVal::EventHashBranch> m_evtHash{};

  // Per-cell content
  MuonVal::MuonIdentifierBranch m_cell_id{m_tree, "cell_id"};
  MuonVal::ThreeVectorBranch    m_cell_pos{m_tree, "cell_position"};
  MuonVal::VectorBranch<float>& m_cell_energy{m_tree.newVector<float>("cell_energy_mev")};
  MuonVal::VectorBranch<float>& m_cell_eta{m_tree.newVector<float>("cell_eta")};
  MuonVal::VectorBranch<float>& m_cell_phi{m_tree.newVector<float>("cell_phi")};
  MuonVal::VectorBranch<int>&   m_cell_sampling{m_tree.newVector<int>("cell_sampling")};
  MuonVal::VectorBranch<int>&   m_cell_subCalo{m_tree.newVector<int>("cell_subCalo")};
  MuonVal::VectorBranch<unsigned short>& m_cell_isTile{m_tree.newVector<unsigned short>("cell_isTile")};

  // Per-tower content
  MuonVal::VectorBranch<float>& m_tower_energy{m_tree.newVector<float>("tower_energy_mev")};
  MuonVal::VectorBranch<float>& m_tower_et{m_tree.newVector<float>("tower_et_mev")};
  MuonVal::VectorBranch<float>& m_tower_eta{m_tree.newVector<float>("tower_eta")};
  MuonVal::VectorBranch<float>& m_tower_phi{m_tree.newVector<float>("tower_phi")};
  MuonVal::ThreeVectorBranch    m_tower_dir{m_tree, "tower_direction"};
  MuonVal::VectorBranch<unsigned int>& m_tower_nCells{m_tree.newVector<unsigned int>("tower_nCells")};

};

}  // namespace MuonR4

#endif
