/*
 *   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

/*
  This algorithm creates clusters from CaloCells, and writes them out
   as Caloclusters. The clustering strategy is carried out by helper objects.
   The strategy used is chosen accoeding to string set at configure time. *
*/

#include "./GepTowersAlg.h"

// concrete cluster maker classes:
#include "./TCTowerMaker.h"
#include "./TopoTowerMaker.h"

#include "./Cluster.h"
#include "GaudiKernel/EventContext.h"

#include "CaloDetDescr/CaloDetDescrManager.h"
#include "xAODCaloEvent/CaloClusterAuxContainer.h"

GepTowersAlg::GepTowersAlg( const std::string& name, ISvcLocator* pSvcLocator ) : 
AthReentrantAlgorithm( name, pSvcLocator ){
   }


StatusCode GepTowersAlg::initialize() {
  ATH_MSG_INFO ("Initializing " << name() << "...");
  ATH_MSG_INFO ("Tower alg " << m_towerAlg);

  // Initialize read and write handles
  CHECK(m_eventInfoKey.initialize());
  CHECK(m_caloCellsKey.initialize());
  CHECK(m_caloClustersKey.initialize());
  CHECK(m_outputCaloClustersKey.initialize());
  CHECK(m_gepCellsKey.initialize());

  return StatusCode::SUCCESS;
}


StatusCode GepTowersAlg::execute(const EventContext& context) const {
  // Feed the specified cell map to a cluster creation algorithm and writes 
  // them out

  ATH_MSG_DEBUG ("Executing " << name() << "...");

  auto h_eventInfo = SG::makeHandle(m_eventInfoKey, context);
  CHECK(h_eventInfo.isValid());
  ATH_MSG_DEBUG("eventNumber=" << h_eventInfo->eventNumber() );

  // read in clusters
  auto h_caloClusters = SG::makeHandle(m_caloClustersKey, context);
  CHECK(h_caloClusters.isValid());
  ATH_MSG_DEBUG("Read in " << h_caloClusters->size() << " clusters");

  auto h_caloCells = SG::makeHandle(m_caloCellsKey, context);
  CHECK(h_caloCells.isValid());

  // container for CaloCluster wrappers for Gep Clusters
  SG::WriteHandle<xAOD::CaloClusterContainer> h_outputCaloClusters =
    SG::makeHandle(m_outputCaloClustersKey, context);
  CHECK(h_outputCaloClusters.record(std::make_unique<xAOD::CaloClusterContainer>(),
                                    std::make_unique<xAOD::CaloClusterAuxContainer>()));

  // Run  a cluster algorithm
  std::unique_ptr<Gep::ITowerMaker> towerMaker{};

  // Instantiate a cluster creater object 
  if( m_towerAlg == "TCTower" ){
    towerMaker.reset(new Gep::TCTowerMaker());
  }
  else if( m_towerAlg == "TopoTower" ){
    towerMaker.reset(new Gep::TopoTowerMaker());
  }
  if( !towerMaker ){ 
    ATH_MSG_ERROR( "Unknown towerMaker" + m_towerAlg );
    return StatusCode::FAILURE;
  }

  ATH_MSG_DEBUG( "Running " << towerMaker->getName() << " tower algorithm." );

  // pass them to the tower maker
  const xAOD::CaloClusterContainer& clusters = *h_caloClusters;
  const CaloCellContainer& cells = *h_caloCells;
  std::vector<Gep::Cluster> customTowers = towerMaker->makeTowers(clusters,cells);

  ATH_MSG_DEBUG( "Tower Maker Algorithm completed." );
  ATH_MSG_DEBUG("No of towers: " << customTowers.size());
  if (!customTowers.empty()){
    ATH_MSG_DEBUG("Tower 0 Energy: " << (customTowers[0]).vec.E());
  }
 
  // Store the Gep clusters to a CaloClusters, and write out.
  h_outputCaloClusters->reserve(customTowers.size());

  for(const auto& gepclus: customTowers){

    // make a unique_ptr, but keep hold of the bare pointer
    auto *ptr = h_outputCaloClusters->push_back(std::make_unique<xAOD::CaloCluster>());

    // to update the calo cluster.
    ptr->setE(gepclus.vec.E());
    ptr->setEta(gepclus.vec.Eta());
    ptr->setPhi(gepclus.vec.Phi());
    ptr->setTime(gepclus.time);
  }
    
  return StatusCode::SUCCESS;
}

