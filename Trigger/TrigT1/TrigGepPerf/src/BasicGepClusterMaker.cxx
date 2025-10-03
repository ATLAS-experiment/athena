/*
 *   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#include "./BasicGepClusterMaker.h"

std::vector<Gep::Cluster>
Gep::BasicGepClusterMaker::makeClusters(const pGepCellMap& caloCellsMap) const {

  std::vector<Gep::Cluster> clusters;

  // Loop over all cells
  std::vector<unsigned int> seenSeedCells;
  for (auto const& cell_itr : *caloCellsMap) {

	// Select only seed cells
	if (!isSeedCell(cell_itr.second, seenSeedCells)) continue;

	// Clustering
        std::vector<Gep::GepCaloCell> cluster_cells = clusterFromCells(cell_itr.second, caloCellsMap, seenSeedCells);
	
        Gep::Cluster cluster = getClusterFromListOfCells(cluster_cells);
        clusters.push_back(cluster);
  }

  // Order topo clusters according to their Et
  std::sort(clusters.begin(), clusters.end(),[](const auto &c1, const auto &c2) {return c1.et() > c2.et();});

  return clusters;
}


bool Gep::BasicGepClusterMaker::isSeedCell (const Gep::GepCaloCell& cell, const std::vector<unsigned int> &seenSeedCells) const {

  if (cell.isBadCell()) return false;
  if (cell.sigma < m_seed_threshold) return false;
  if (!isNewCell(cell.id, seenSeedCells)) return false;
  if (!isInAllowedSampling(cell.sampling, m_disallowed_seed_samplings)) return false;

  return true;
}


bool Gep::BasicGepClusterMaker::isInAllowedSampling(int sampling, const std::vector<int>& list_of_samplings) const {

  for (unsigned int i = 0; i < list_of_samplings.size(); ++i) {
	if (list_of_samplings[i] == sampling) return false;
  }
  return true;
}


bool Gep::BasicGepClusterMaker::isNewCell(unsigned int id, const std::vector<unsigned int>& seenCells) const {

  for (unsigned int i = 0; i < seenCells.size(); ++i) {
        if (id == seenCells[i]) return false;
  }

  return true;
}


std::vector<Gep::GepCaloCell>
Gep::BasicGepClusterMaker::clusterFromCells(const Gep::GepCaloCell& seed,
					  const pGepCellMap& caloCellsMap, std::vector<unsigned int> &seenSeedCells) const {

  std::vector<Gep::GepCaloCell> v_clusterCells;

  std::vector<Gep::GepCaloCell> cellsNextLayer, cellsThisLayer;
  std::vector<unsigned int> seenCells;

  // Fill seed into supporting vectors
  v_clusterCells.push_back(seed);
  cellsNextLayer.push_back(seed);
  seenCells.push_back(seed.id);
  seenSeedCells.push_back(seed.id);

  int i_shell = 1;

  while (!cellsNextLayer.empty() && i_shell <= m_max_shells) {

        cellsThisLayer.swap(cellsNextLayer);
        cellsNextLayer.clear();
        ++i_shell;

        // Loop over all cells in this shell
        for (unsigned int i_cell = 0; i_cell < cellsThisLayer.size(); ++i_cell) {

                // Go through list of neighbouring cells and check whether they are part of the cluster
                for (unsigned int i_neighbour = 0; i_neighbour < (cellsThisLayer[i_cell]).neighbours.size(); ++i_neighbour) {

			// Check whether this neighbouring cell was sent to the GEP
			auto const& nghbr_itr = caloCellsMap->find((cellsThisLayer[i_cell]).neighbours[i_neighbour]);
			if (nghbr_itr == caloCellsMap->end()) continue;

			Gep::GepCaloCell neighbour = nghbr_itr->second;

                        // reject if bad cell
                        if (neighbour.isBadCell()) continue;

                        // Reject if cell is not above clustering threshold
                        if (neighbour.sigma < m_clustering_threshold) continue;

                        // Reject if cell was already considered
                        if (!isNewCell(neighbour.id, seenCells)) continue;

                        // Ignore cells in disallowed samplings
                        if (!isInAllowedSampling(neighbour.sampling, m_disallowed_clustering_samplings)) continue;

                        seenCells.push_back(neighbour.id);
                        cellsNextLayer.push_back(neighbour);
                        v_clusterCells.push_back(neighbour);

			// If the cell is another seed cell, we write it into the list of seen seed cells to not reconstruct this cluster again
                        if (neighbour.sigma > m_seed_threshold) seenSeedCells.push_back(neighbour.id);
                }
        }
        cellsThisLayer.clear();
  }

  return v_clusterCells;
}


Gep::Cluster Gep::BasicGepClusterMaker::getClusterFromListOfCells(const std::vector<Gep::GepCaloCell>& cells) const {

  Gep::Cluster cluster;
  std::vector<unsigned int> v_cellIDs;

  TLorentzVector tlv_cluster;
  for (unsigned int i_cell = 0; i_cell < cells.size(); ++i_cell) {
	TLorentzVector cell;
	cell.SetPtEtaPhiE(cells[i_cell].et, cells[i_cell].eta, cells[i_cell].phi, cells[i_cell].et*std::cosh(cells[i_cell].eta));

	tlv_cluster += cell;

	v_cellIDs.push_back(cells[i_cell].id);
  }

  cluster.ncells = cells.size();
  cluster.time = cells[0].time; // Take time of seed cell
  cluster.cell_id = std::move(v_cellIDs);
  cluster.setEtEtaPhi(tlv_cluster.Et(), tlv_cluster.Eta(), tlv_cluster.Phi());

  return cluster;
}


std::string Gep::BasicGepClusterMaker::getName() const {
  return "GEPBasic";
}
