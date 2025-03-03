/*
*   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "./GepTCTowerAlg.h"
#include "./Cluster.h"

#include "CaloDetDescr/CaloDetDescrManager.h"
#include "xAODCaloEvent/CaloClusterAuxContainer.h"

GepTCTowerAlg::GepTCTowerAlg( const std::string& name, ISvcLocator* pSvcLocator ) : 
   AthReentrantAlgorithm( name, pSvcLocator ){
}


GepTCTowerAlg::~GepTCTowerAlg() {}


StatusCode GepTCTowerAlg::initialize() {
  ATH_MSG_INFO ("Initializing " << name() << "...");
  CHECK(m_caloClustersKey.initialize());
  CHECK(m_outputCaloClustersKey.initialize());

  return StatusCode::SUCCESS;
}

StatusCode GepTCTowerAlg::finalize() {
  ATH_MSG_INFO ("Finalizing " << name() << "...");
  return StatusCode::SUCCESS;
}

StatusCode GepTCTowerAlg::execute(const EventContext& context) const {
  ATH_MSG_DEBUG ("Executing " << name() << "...");
  setFilterPassed(false, context); //optional: start with algorithm not passed

  // read in clusters
  auto h_caloClusters = SG::makeHandle(m_caloClustersKey, context);
  CHECK(h_caloClusters.isValid());
  ATH_MSG_DEBUG("Read in " << h_caloClusters->size() << " clusters");

  std::vector<Gep::Cluster> customClusters;

  for(auto iClus: *h_caloClusters){
    Gep::Cluster clus;
    clus.vec.SetPxPyPzE(iClus->p4().Px(), iClus->p4().Py(),
                          iClus->p4().Pz(), iClus->e());
    customClusters.push_back(clus);
  }

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

  // Single loop over clusters to assign them to towers
  for (const auto& cluster : customClusters) {
      // Compute eta and phi indices
      int eta_index = static_cast<int>(std::floor(cluster.vec.Eta() * 10)) + 49;
      int phi_index = static_cast<int>(std::floor(cluster.vec.Phi() * 10)) + 32;

      // Ensure indices are within bounds
      if (eta_index < 0 || eta_index >= 98 || phi_index < 0 || phi_index >= 64) continue;

      // Accumulate cluster data into the corresponding tower
      tow[eta_index][phi_index].vec += cluster.vec;
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
