/*
*   Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
#include "./GepTopoTowerAlg.h"
#include "./Cluster.h"

#include "CaloDetDescr/CaloDetDescrManager.h"
#include "xAODCaloEvent/CaloClusterAuxContainer.h"

GepTopoTowerAlg::GepTopoTowerAlg( const std::string& name, ISvcLocator* pSvcLocator ) : 
   AthReentrantAlgorithm( name, pSvcLocator ){
}


GepTopoTowerAlg::~GepTopoTowerAlg() {}


StatusCode GepTopoTowerAlg::initialize() {
  ATH_MSG_INFO ("Initializing " << name() << "...");
  CHECK(m_caloCellsKey.initialize());
  CHECK(m_caloClustersKey.initialize());
  CHECK(m_outputCaloClustersKey.initialize());
  CHECK(m_gepCellsKey.initialize());

  return StatusCode::SUCCESS;
}

StatusCode GepTopoTowerAlg::finalize() {
  ATH_MSG_INFO ("Finalizing " << name() << "...");
  return StatusCode::SUCCESS;
}

StatusCode GepTopoTowerAlg::execute(const EventContext& context) const {
  ATH_MSG_DEBUG ("Executing " << name() << "...");
  setFilterPassed(false, context); //optional: start with algorithm not passed

  // read in clusters
  auto h_caloClusters = SG::makeHandle(m_caloClustersKey, context);
  CHECK(h_caloClusters.isValid());
  ATH_MSG_DEBUG("Read in " << h_caloClusters->size() << " clusters");

  const auto inputTopoClusters = *h_caloClusters;

/*  auto h_gepCellsMap = SG::makeHandle(m_gepCellsKey, context);
  CHECK(h_gepCellsMap.isValid());
  auto gepCellsMap = *h_gepCellsMap;
  ATH_MSG_INFO("Read in " << gepCellsMap.size() << " GEP cells");

  auto cellsPtr = gepCellsMap.getCellMap(); // Unique pointer to map
  auto& cells = *cellsPtr; // Dereference to get std::map<unsigned int, Gep::GepCaloCell>
*/

  auto h_caloCells = SG::makeHandle(m_caloCellsKey, context);
  CHECK(h_caloCells.isValid());
  auto cells = *h_caloCells;

  // container for CaloCluster wrappers for Gep Clusters
  SG::WriteHandle<xAOD::CaloClusterContainer> h_outputCaloClusters =
    SG::makeHandle(m_outputCaloClustersKey, context);
  CHECK(h_outputCaloClusters.record(std::make_unique<xAOD::CaloClusterContainer>(),
                                    std::make_unique<xAOD::CaloClusterAuxContainer>()));

  // Define tower array (98 eta bins x 64 phi bins)
  Gep::Cluster tow[98][64];

  // Initialize towers (erase previous data)
  for (int i = 0; i < 98; ++i) {
      for (int j = 0; j < 64; ++j) {
          tow[i][j].erase();
      }
  }

  // Loop over clusters and their associated cells
  for (const auto& iClust : *h_caloClusters) {
      CaloClusterCellLink::const_iterator cellBegin = iClust->cell_begin();
      CaloClusterCellLink::const_iterator cellEnd = iClust->cell_end();

      for (; cellBegin != cellEnd; ++cellBegin) {
          unsigned int cellIndex = cellBegin.index();
          const CaloCell* cell = cells.at(cellIndex);

          // Compute eta and phi indices (binning in steps of 0.1)
          int eta_index = static_cast<int>(std::floor(cell->eta() * 10)) + 49;
          int phi_index = static_cast<int>(std::floor(cell->phi() * 10)) + 32;

          // Ensure indices are within bounds
          if (eta_index < 0 || eta_index >= 98 || phi_index < 0 || phi_index >= 64) continue;

          // Accumulate cell data into the corresponding tower
          TLorentzVector cellVector;
          cellVector.SetPtEtaPhiE(cell->energy() * 1.0 / TMath::CosH(cell->eta()),
                                  cell->eta(), cell->phi(), cell->energy());
          tow[eta_index][phi_index].vec += cellVector;
      }
  }

  // Collect non-empty towers into a vector
  std::vector<Gep::Cluster> customTowers;
  for (int i = 0; i < 98; ++i) {
      for (int j = 0; j < 64; ++j) {
          if (tow[i][j].vec.Et() > 0) {
              customTowers.push_back(tow[i][j]);
          }
      }
  }  

  // Store the Gep clusters to a CaloClusters, and write out.
  h_outputCaloClusters->reserve(customTowers.size());

  for(const auto& gepclus: customTowers){

    // make a unique_ptr, but keep hold of the bare pointer
    auto caloCluster = std::make_unique<xAOD::CaloCluster>();
    auto *ptr = caloCluster.get();

    // store the calCluster to fix up the Aux container:
    h_outputCaloClusters->push_back(std::move(caloCluster));

    // this invalidates the unque_ptr, but can use the bare ptr
    // to update the calo cluster.
    ptr->setE(gepclus.vec.E());
    ptr->setEta(gepclus.vec.Eta());
    ptr->setPhi(gepclus.vec.Phi());
    ptr->setTime(gepclus.time);
  }

  setFilterPassed(true,context); //if got here, assume that means algorithm passed
  return StatusCode::SUCCESS;
}
