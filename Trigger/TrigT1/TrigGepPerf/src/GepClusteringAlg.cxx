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
#include "CaloGeoHelpers/CaloSampling.h"
#include "xAODCaloEvent/CaloClusterAuxContainer.h"

#include <cmath>
#include <vector>

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

    // Below are modifications to add per layer energy for clusters: 
    // Accumulate per-sampling (layer) transverse energy from the cluster's cells
    // and store it on the cluster as energy (E = Et * cosh(eta)), mirroring
    // GepCellTowerAlg. The cluster itself carries only cell links, so downstream
    // PU-suppressed copies (EtaSK / SK) -- which drop the cell links -- would
    // otherwise have no way to expose per-layer Et. Storing it here lets them
    // recover it via eSample(); dividing back by cosh(eta) reproduces the same
    // cell-Et sum used for the un-suppressed clusters.
    const double clusEta = gepclus.vec.Eta();
    std::vector<float> layerEnergies(static_cast<int>(CaloSampling::Unknown), 0.f);
    for (auto cell_id : gepclus.cell_id) {
      const auto& cell = pCellMap->at(cell_id);
      if (cell.sampling < static_cast<unsigned int>(CaloSampling::Unknown))
        layerEnergies[cell.sampling] += cell.et;
    }
    uint32_t samplingPattern = 0;
    for (int i = 0; i < static_cast<int>(CaloSampling::Unknown); ++i)
      if (layerEnergies[i] != 0) samplingPattern |= (0x1U << i);
    ptr->clearSamplingData();
    ptr->setSamplingPattern(samplingPattern);
    for (int i = 0; i < static_cast<int>(CaloSampling::Unknown); ++i) {
      if (layerEnergies[i] != 0)
        ptr->setEnergy(static_cast<CaloSampling::CaloSample>(i),
                       layerEnergies[i] * std::cosh(clusEta));
    }

    CaloClusterCellLink *cccl = new CaloClusterCellLink();

    for (auto cell_id : gepclus.cell_id)
        cccl->addCell(pCellMap->at(cell_id).index, 1.0);

    ptr->addCellLink(std::make_unique<CaloClusterCellLink>(*cccl));
  }
  
    
  return StatusCode::SUCCESS;
}

