/*
 *   Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
 */

/*
  This algorithm creates clusters from CaloCells, and writes them out
   as Caloclusters. The clustering strategy is carried out by helper objects.
   The strategy used is chosen accoeding to string set at configure time. *
*/

#include "./GepClusteringAlg.h"

// concrete cluster maker classes:
#include "./WFSClusterMaker.h"
#include "./BasicGepClusterMaker.h"

#include "CaloDetDescr/CaloDetDescrManager.h"
#include "xAODCaloEvent/CaloClusterAuxContainer.h"

GepClusteringAlg::GepClusteringAlg( const std::string& name, ISvcLocator* pSvcLocator ) : 
AthReentrantAlgorithm( name, pSvcLocator ){
   }


StatusCode GepClusteringAlg::initialize() {
  ATH_MSG_INFO ("Initializing " << name() << "...");
  ATH_MSG_INFO ("Clustering alg " << m_clusterAlg);

  // Initialize read and write handles
  CHECK(m_eventInfoKey.initialize());
  CHECK(m_outputCaloClustersKey.initialize());
  CHECK(m_gepCellsKey.initialize());

  return StatusCode::SUCCESS;
}


StatusCode GepClusteringAlg::execute(const EventContext& ctx) const {
  // Feed the specified cell map to a cluster creation algorithm and writes 
  // them out

  ATH_MSG_DEBUG ("Executing " << name() << "...");

  auto h_eventInfo = SG::makeHandle(m_eventInfoKey, ctx);
  CHECK(h_eventInfo.isValid());
  ATH_MSG_DEBUG("eventNumber=" << h_eventInfo->eventNumber() );

  auto h_gepCellsMap = SG::makeHandle(m_gepCellsKey, ctx);
  CHECK(h_gepCellsMap.isValid());
  auto gepCellsMap = *h_gepCellsMap;

  ATH_MSG_DEBUG("Read in " << gepCellsMap.size() << " GEP cells");

  // container for CaloCluster wrappers for Gep Clusters
  SG::WriteHandle<xAOD::CaloClusterContainer> h_outputCaloClusters =
    SG::makeHandle(m_outputCaloClustersKey, ctx);
  CHECK(h_outputCaloClusters.record(std::make_unique<xAOD::CaloClusterContainer>(),
				    std::make_unique<xAOD::CaloClusterAuxContainer>()));

  // Run  a cluster algorithm
  std::unique_ptr<Gep::IClusterMaker> clusterMaker{};

  // Instantiate a cluster creater object 
  if( m_clusterAlg == "WFS" ){
    clusterMaker.reset(new Gep::WFSClusterMaker());
  }

  if( m_clusterAlg == "GEPBasic" ){
    clusterMaker.reset(new Gep::BasicGepClusterMaker());
  }

  if( !clusterMaker ){ 
    ATH_MSG_ERROR( "Unknown clusterMaker" + m_clusterAlg );
    return StatusCode::FAILURE;
  }

  ATH_MSG_DEBUG( "Running " << clusterMaker->getName() << " cluster algorithm." );

  // pass them to the cluster maker
  auto pCellMap = gepCellsMap.getCellMap();
  std::vector<Gep::Cluster> customClusters = clusterMaker->makeClusters(pCellMap);

  ATH_MSG_DEBUG( "Clustering completed." );
  ATH_MSG_DEBUG("No of clusters: " << customClusters.size());
  if (!customClusters.empty()){
    ATH_MSG_DEBUG("Cluster 0 Energy: " << (customClusters[0]).vec.E());
  }

  // Store the Gep clusters to a CaloClusters, and write out.
  h_outputCaloClusters->reserve(customClusters.size());

  for(const auto& gepclus: customClusters){

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

    CaloClusterCellLink *cccl = new CaloClusterCellLink();

    for (auto cell_id : gepclus.cell_id) 
        cccl->addCell(pCellMap->at(cell_id).index, 1.0);

    ptr->addCellLink(cccl);
  }
  
    
  return StatusCode::SUCCESS;
}

