/*
*   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "./TopoTowerMaker.h"

std::vector<Gep::Cluster>
Gep::TopoTowerMaker::makeTowers(const xAOD::CaloClusterContainer& clusters, const CaloCellContainer& cells) const {

  std::vector<Gep::Cluster> customTowers;

  // Define tower array (98 eta bins x 64 phi bins)
  static constexpr int nEta{98};
  static constexpr int nPhi{64};
  //avoid stack use of 605kb
  auto tow = new Gep::Cluster[nEta][nPhi]();


  // Loop over clusters and their associated cells
  for (const auto* iClust : clusters) {
      CaloClusterCellLink::const_iterator cellBegin = iClust->cell_begin();
      CaloClusterCellLink::const_iterator cellEnd = iClust->cell_end();

      for (; cellBegin != cellEnd; ++cellBegin) {
          unsigned int cellIndex = cellBegin.index();
	  if (cellIndex >= cells.size()) continue;  // avoid out-of-bounds
	  const CaloCell* cell = cells[cellIndex];
	  if (!cell) continue;

          // Compute eta and phi indices (binning in steps of 0.1)
          int eta_index = static_cast<int>(std::floor(cell->eta() * 10)) + 49;
          int phi_index = static_cast<int>(std::floor(cell->phi() * 10)) + 32;

          // Ensure indices are within bounds
          if (eta_index < 0 || eta_index >= nEta || phi_index < 0 || phi_index >= nPhi) continue;

          // Accumulate cell data into the corresponding tower
          TLorentzVector cellVector;
          cellVector.SetPtEtaPhiE(cell->energy() * 1.0 / TMath::CosH(cell->eta()),
                                  cell->eta(), cell->phi(), cell->energy());
          tow[eta_index][phi_index].vec += cellVector;
      }
  }

  // Collect non-empty towers into a vector
  for (int i = 0; i < nEta; ++i) {
      for (int j = 0; j < nPhi; ++j) {
          if (tow[i][j].vec.Et() > 0) {
              customTowers.push_back(tow[i][j]);
          }
      }
  }  
  delete[] tow;
  return customTowers;
}

std::string Gep::TopoTowerMaker::getName() const {
  return "TopoTower";
}
