/*
*   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "./GepCellTowerAlg.h"
#include "./Cluster.h"

#include "CaloDetDescr/CaloDetDescrManager.h"
#include "xAODCaloEvent/CaloClusterAuxContainer.h"

GepCellTowerAlg::GepCellTowerAlg( const std::string& name, ISvcLocator* pSvcLocator ) : 
   AthReentrantAlgorithm( name, pSvcLocator ){
}


GepCellTowerAlg::~GepCellTowerAlg() {}


StatusCode GepCellTowerAlg::initialize() {
  ATH_MSG_DEBUG ("Initializing " << name() << "...");

  // Retrieve AlgTools
  CHECK(m_outputCellTowerKey.initialize());
  CHECK(m_gepCellsKey.initialize());

  return StatusCode::SUCCESS;
}

StatusCode GepCellTowerAlg::finalize() {
  ATH_MSG_DEBUG ("Finalizing " << name() << "...");
  return StatusCode::SUCCESS;
}

StatusCode GepCellTowerAlg::execute(const EventContext& context) const {
  ATH_MSG_DEBUG ("Executing " << name() << "...");
  setFilterPassed(false, context); //optional: start with algorithm not passed

  std::vector<Gep::GepCaloCell> cells;

  auto h_gepCellsMap = SG::makeHandle(m_gepCellsKey, context);
  CHECK(h_gepCellsMap.isValid());
  auto gepCellsMap = *h_gepCellsMap;

  auto cell_map = gepCellsMap.getCellMap();

  // Loop over all cells
  for (auto const& cell_itr : *cell_map) {
    cells.push_back(cell_itr.second);
  }

  // container for CaloCluster wrappers for Gep Clusters
  SG::WriteHandle<xAOD::CaloClusterContainer> h_outputCaloClusters =
    SG::makeHandle(m_outputCellTowerKey, context);
  CHECK(h_outputCaloClusters.record(std::make_unique<xAOD::CaloClusterContainer>(),
                                    std::make_unique<xAOD::CaloClusterAuxContainer>()));

  // Define tower array (98 eta bins x 64 phi bins)
  static constexpr int nEta{98};
  static constexpr int nPhi{64};
  //avoid stack use of 605kb
  auto tow = new Gep::Cluster[nEta][nPhi]();
  // Single loop over cells to accumulate energy into the correct tower
  for (const auto& cell : cells) {
      if (cell.sigma < 2) continue;
      if (cell.isBadCell()) continue;

      // Compute eta and phi indices (binning in steps of 0.1)
      int eta_index = static_cast<int>(std::floor(cell.eta * 10)) + 49;
      int phi_index = static_cast<int>(std::floor(cell.phi * 10)) + 32;

      // Ensure indices are within bounds
      if (eta_index < 0 || eta_index >= nEta || phi_index < 0 || phi_index >= nPhi) continue;

      // Accumulate cell data into the corresponding tower
      TLorentzVector cellsVector;
      cellsVector.SetPtEtaPhiE(cell.et, cell.eta, cell.phi, cell.e);
      tow[eta_index][phi_index].vec += cellsVector;
      tow[eta_index][phi_index].cell_id.push_back(cell.id);
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
  delete [] tow;
  // Store the Gep clusters to a CaloClusters, and write out.
  h_outputCaloClusters->reserve(customTowers.size());

  for(const auto& gepclus: customTowers){
    // store the calCluster to fix up the Aux container:
    auto *ptr = h_outputCaloClusters->push_back(std::make_unique<xAOD::CaloCluster>());
    ptr->setE(gepclus.vec.E());
    ptr->setEta(gepclus.vec.Eta());
    ptr->setPhi(gepclus.vec.Phi());
    ptr->setTime(gepclus.time);

    CaloClusterCellLink *cccl = new CaloClusterCellLink();

    for (auto cell_id : gepclus.cell_id)
      cccl->addCell(cell_map->at(cell_id).index, 1.0);

    ptr->addCellLink(std::make_unique<CaloClusterCellLink>(*cccl));
  }

  setFilterPassed(true,context); //if got here, assume that means algorithm passed
  return StatusCode::SUCCESS;
}
