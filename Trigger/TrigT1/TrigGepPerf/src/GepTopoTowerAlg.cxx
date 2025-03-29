/*
*   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
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

  auto h_caloCells = SG::makeHandle(m_caloCellsKey, context);
  CHECK(h_caloCells.isValid());
  auto cells = *h_caloCells;

  // container for CaloCluster wrappers for Gep Clusters
  SG::WriteHandle<xAOD::CaloClusterContainer> h_outputCaloClusters =
    SG::makeHandle(m_outputCaloClustersKey, context);
  CHECK(h_outputCaloClusters.record(std::make_unique<xAOD::CaloClusterContainer>(),
                                    std::make_unique<xAOD::CaloClusterAuxContainer>()));

  // Define tower array (98 eta bins x 64 phi bins)
  static constexpr int nEta{98};
  static constexpr int nPhi{64};
  //avoid stack use of 605kb
  auto tow = new Gep::Cluster[nEta][nPhi]();


  // Loop over clusters and their associated cells
  for (const auto iClust : *h_caloClusters) {
      CaloClusterCellLink::const_iterator cellBegin = iClust->cell_begin();
      CaloClusterCellLink::const_iterator cellEnd = iClust->cell_end();

      for (; cellBegin != cellEnd; ++cellBegin) {
          unsigned int cellIndex = cellBegin.index();
          const CaloCell* cell = cells.at(cellIndex);

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
  std::vector<Gep::Cluster> customTowers;
  for (int i = 0; i < nEta; ++i) {
      for (int j = 0; j < nPhi; ++j) {
          if (tow[i][j].vec.Et() > 0) {
              customTowers.push_back(tow[i][j]);
          }
      }
  }  
  delete[] tow;

  // Store the Gep clusters to a CaloClusters, and write out.
  h_outputCaloClusters->reserve(customTowers.size());

  for(const auto& gepclus: customTowers){
    // store the calCluster to fix up the Aux container:
    auto *ptr = h_outputCaloClusters->push_back(std::make_unique<xAOD::CaloCluster>());
    // update the calo cluster.
    ptr->setE(gepclus.vec.E());
    ptr->setEta(gepclus.vec.Eta());
    ptr->setPhi(gepclus.vec.Phi());
    ptr->setTime(gepclus.time);
  }

  setFilterPassed(true,context); //if got here, assume that means algorithm passed
  return StatusCode::SUCCESS;
}
