/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "CaloCellsDumperAlg.h"

#include "StoreGate/ReadHandle.h"
#include "CaloDetDescr/CaloDetDescrElement.h"
#include "CaloEvent/CaloCell.h"
#include "CaloEvent/CaloTower.h"
#include "AthenaKernel/errorcheck.h"
#include <cmath>

namespace MuonR4 {

StatusCode CaloCellsDumperAlg::initialize() {
  ATH_CHECK(m_cellKey.initialize());
  ATH_CHECK(m_towerKey.initialize(SG::AllowEmpty));

  // CaloCell_ID helper (to provide subCalo/sampling via Identifier decoding if desired)
  ATH_CHECK(detStore()->retrieve(m_caloId, "CaloCell_ID"));

  // Event hash branch
  m_evtHash = std::make_shared<MuonVal::EventHashBranch>(m_tree.tree());
  m_tree.addBranch(m_evtHash);

  ATH_CHECK(m_tree.init(this));
  ATH_MSG_INFO("Initialized Calo dump with:"
               << " CellContainerKey=" << m_cellKey.key()
               << " MinCellEnergyMeV=" << m_minE
               << " MaxCells=" << m_maxCells
               << " | TowerContainerKey=" << m_towerKey.key()
               << " MinTowerEnergyMeV=" << m_minTowerE
               << " MaxTowers=" << m_maxTowers);
  return StatusCode::SUCCESS;
}

StatusCode CaloCellsDumperAlg::finalize() {
  ATH_CHECK(m_tree.write());
  return StatusCode::SUCCESS;
}

StatusCode CaloCellsDumperAlg::execute() {
  const EventContext& ctx{Gaudi::Hive::currentContext()};

  // -----------------------------
  // Cells
  // -----------------------------
  const CaloCellContainer* cells{};
  ATH_CHECK(SG::get(cells, m_cellKey, ctx));

  int nStored = 0;
  for (const CaloCell* cell : *cells) {
    if (!cell) continue;

    // energy() is in MeV for CaloCell
    const float eMeV = cell->energy();
    if (eMeV < m_minE) continue;

    const CaloDetDescrElement* dde = cell->caloDDE();
    if (!dde) continue;

    // Store Identifier
    const Identifier& id = cell->ID();
    m_cell_id.push_back(id);

    // Store position (x,y,z) in mm, eta/phi from DDE
    const Amg::Vector3D pos(dde->x(), dde->y(), dde->z());
    m_cell_pos.push_back(pos);
    m_cell_energy.push_back(eMeV);
    m_cell_eta.push_back(dde->eta());
    m_cell_phi.push_back(dde->phi());

    // Sampling / subCalo
    // - dde->getSampling() gives CaloSampling::CaloSample enum (int)
    // - CaloCell_ID can decode subCalo for the Identifier
    m_cell_sampling.push_back(static_cast<int>(dde->getSampling()));
    m_cell_subCalo.push_back(static_cast<int>(m_caloId->sub_calo(id)));
    m_cell_isTile.push_back(static_cast<unsigned short>(dde->is_tile()));

    ++nStored;
    if (m_maxCells.value() > 0 && nStored >= m_maxCells.value()) break;
  }

  // -----------------------------
  // Towers
  // -----------------------------
  const CaloTowerContainer* towers{};
  ATH_CHECK(SG::get(towers, m_towerKey, ctx));
    int nTowStored = 0;
    for (const CaloTower* tow : *towers) {
      if (!tow) continue;

      // CaloTower energy is in MeV (same convention as cells)
      const float eMeV = tow->energy();
      if (eMeV < m_minTowerE) continue;

      const float eta = tow->eta();
      const float phi = tow->phi();
      const float etMeV = eMeV / std::cosh(eta);

      m_tower_energy.push_back(eMeV);
      m_tower_et.push_back(etMeV);
      m_tower_eta.push_back(eta);
      m_tower_phi.push_back(phi);

      // Derived direction vector (unit length) from eta/phi
      const double theta = 2.0 * std::atan(std::exp(-static_cast<double>(eta)));
      const double st = std::sin(theta);
      const double ct = std::cos(theta);
      const double cp = std::cos(static_cast<double>(phi));
      const double sp = std::sin(static_cast<double>(phi));
      const Amg::Vector3D dir(st * cp, st * sp, ct);
      m_tower_dir.push_back(dir);

      // Try to provide tower "size" (how many cells contribute)
      m_tower_nCells.push_back(static_cast<unsigned int>(tow->size()));

      ++nTowStored;
      if (m_maxTowers.value() > 0 && nTowStored >= m_maxTowers.value()) break;
    }

  if (!m_tree.fill(ctx)) return StatusCode::FAILURE;
  return StatusCode::SUCCESS;
}

}  // namespace MuonR4
