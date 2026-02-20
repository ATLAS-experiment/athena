/*
*   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "./TCTowerMaker.h"
std::vector<Gep::Cluster>
Gep::TCTowerMaker::makeTowers(const xAOD::CaloClusterContainer& clusters, const CaloCellContainer& cells) const {

  std::vector<Gep::Cluster> customClusters;
  (void)cells;  // Mark as intentionally unused
  for(const auto* iClus : clusters){
    Gep::Cluster clus;
    clus.vec.SetPxPyPzE(iClus->p4().Px(), iClus->p4().Py(),
                          iClus->p4().Pz(), iClus->e());
    customClusters.push_back(std::move(clus));
  }

  // Define tower array (98 eta bins x 64 phi bins)
  static constexpr int nEta{98};
  static constexpr int nPhi{64};
  //avoid stack use of 605kb
  auto tow = new Gep::Cluster[nEta][nPhi]();


  // Single loop over clusters to assign them to towers
  for (const auto& cluster : customClusters) {
      // Compute eta and phi indices
      int eta_index = static_cast<int>(std::floor(cluster.vec.Eta() * 10)) + 49;
      int phi_index = static_cast<int>(std::floor(cluster.vec.Phi() * 10)) + 32;

      // Ensure indices are within bounds
      if (eta_index < 0 || eta_index >= nEta || phi_index < 0 || phi_index >= nPhi) continue;

      // Accumulate cluster data into the corresponding tower
      tow[eta_index][phi_index].vec += cluster.vec;
  }

  // Collect non-empty towers into a vector
  std::vector<Gep::Cluster> customTowers;
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

std::string Gep::TCTowerMaker::getName() const {
  return "TCTower";
}
