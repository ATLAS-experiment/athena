/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// ********************************************************************
//
// NAME:     TrigCaloClusterMaker.cxx
// PACKAGE:  Trigger/TrigAlgorithms/TrigCaloRec
//
// AUTHOR:   C. Santamarina 
//           This is an Hlt algorithm that creates a cell container
//           with calorimeter cells within an RoI. Afterwards a cluster
//           container with the clusters made with those cells is
//           created with the standard offline clustering algorithms.
//
// ********************************************************************
//

#include "TrigCaloClusterMaker.h"

#include "AthenaMonitoringKernel/MonitoredCollection.h"
#include "AthenaMonitoringKernel/MonitoredGroup.h"
#include "AthenaMonitoringKernel/MonitoredScalar.h"
#include "AthenaMonitoringKernel/MonitoredTimer.h"
#include "CaloEvent/CaloCellContainer.h"
#include "CaloInterface/ISetCaloCellContainerName.h"
#include "CaloUtils/CaloClusterCollectionProcessor.h"
#include "CaloUtils/CaloClusterProcessor.h"
#include "CaloUtils/CaloClusterStoreHelper.h"
#include "GaudiKernel/StatusCode.h"
#include "StoreGate/ReadDecorHandle.h"
#include "StoreGate/WriteDecorHandle.h"
#include "xAODTrigCalo/CaloClusterTrigAuxContainer.h"


/////////////////////////////////////////////////////////////////////
// CONSTRUCTOR:
/////////////////////////////////////////////////////////////////////
//
TrigCaloClusterMaker::TrigCaloClusterMaker(const std::string& name, ISvcLocator* pSvcLocator)
  : AthReentrantAlgorithm(name, pSvcLocator)
{
}

/////////////////////////////////////////////////////////////////////
// INITIALIZE:
// The initialize method will create all the required algorithm objects
// Note that it is NOT NECESSARY to run the initialize of individual
// sub-algorithms.  The framework takes care of it.
/////////////////////////////////////////////////////////////////////
//

StatusCode TrigCaloClusterMaker::initialize()
{
  ATH_MSG_DEBUG("in TrigCaloClusterMaker::initialize()" );

  if (!m_monTool.empty()) {
    ATH_MSG_DEBUG("Retrieving monTool");
    CHECK(m_monTool.retrieve());
  } else {
    ATH_MSG_INFO("No monTool configured => NO MONITOING");
  }
     
  m_mDecor_ncells = m_outputClustersKey.key() + "." + m_mDecor_ncells.key();
  ATH_CHECK( m_mDecor_ncells.initialize());

  ATH_CHECK( m_clusterMakers.retrieve() );
  ATH_CHECK( m_clusterCorrections.retrieve() );
 
  ATH_CHECK( m_inputCellsKey.initialize() );
  ATH_CHECK( m_outputClustersKey.initialize() );
  ATH_CHECK( m_clusterCellLinkOutput.initialize() );
  ATH_CHECK( m_avgMuKey.initialize() );
  ATH_CHECK( m_noiseCDOKey.initialize(m_monCells) );

  for (ToolHandle<CaloClusterCollectionProcessor>& clproc : m_clusterMakers) {
    // Set the CellsName property on the input tool (why isn't this done in
    // python?)
    AlgTool* algtool = dynamic_cast<AlgTool*> (clproc.get());
    if (clproc->name().find("CaloTopoClusterMaker") != std::string::npos) {
      if (!algtool) {
        ATH_MSG_ERROR("Could not cast " << clproc->name() << " to an AlgTool!");
        return StatusCode::FAILURE;
      }
      ATH_CHECK(algtool->setProperty(StringProperty("CellsName", m_inputCellsKey.key())));
    }
    if (clproc->name().find("trigslw") != std::string::npos)
      m_isSW = true;
  }

  for (ToolHandle<CaloClusterProcessor>& clcorr : m_clusterCorrections) {
    ISetCaloCellContainerName* setter = 
      dynamic_cast<ISetCaloCellContainerName*> (clcorr.get());
    if (setter) 
      ATH_CHECK(setter->setCaloCellContainerName(m_inputCellsKey.key()));
  }

  ATH_MSG_DEBUG("Initialization of TrigCaloClusterMaker completed successfully");

  return StatusCode::SUCCESS;
}


StatusCode TrigCaloClusterMaker::execute(const EventContext& ctx) const
{
  // Monitoring initialization...
  auto time_tot = Monitored::Timer("TIME_execute");
  auto time_clusMaker = Monitored::Timer("TIME_ClustMaker");
  auto time_clusCorr = Monitored::Timer("TIME_ClustCorr");

  // Start timer
  time_tot.start();

  ATH_MSG_DEBUG("in TrigCaloClusterMaker::execute()" );
  bool shouldMon = ((ctx.eventID().event_number() % 20)==0);

  // We now take care of the Cluster Making... 
  auto  clusterContainer =   SG::makeHandle (m_outputClustersKey, ctx); 
  ATH_MSG_VERBOSE(" Output Clusters : " <<  clusterContainer.name());
  ATH_CHECK( clusterContainer.record (std::make_unique<xAOD::CaloClusterContainer>(),  std::make_unique<xAOD::CaloClusterTrigAuxContainer> () ));

  xAOD::CaloClusterContainer* pCaloClusterContainer = clusterContainer.ptr();
  ATH_MSG_VERBOSE(" created ClusterContainer at 0x" << std::hex << pCaloClusterContainer<< std::dec);


  // monitored variables 
  auto mon_clusEt = Monitored::Collection("Et",   *pCaloClusterContainer, &xAOD::CaloCluster::et );
  auto mon_clusSignalState = Monitored::Collection("signalState",   *pCaloClusterContainer, &xAOD::CaloCluster::signalState );
  auto mon_clusSize = Monitored::Collection("clusterSize",   *pCaloClusterContainer, &xAOD::CaloCluster::clusterSize );
  std::vector<double>       clus_phi;
  std::vector<double>       clus_eta;
  std::vector<double>       N_BAD_CELLS;
  std::vector<double>       ENG_FRAC_MAX;
  std::vector<unsigned int> sizeVec; 
  clus_phi.reserve(300); clus_eta.reserve(300);
  N_BAD_CELLS.reserve(300); ENG_FRAC_MAX.reserve(300);
  sizeVec.reserve(300);
  auto mon_clusPhi = Monitored::Collection("Phi", clus_phi); // phi and eta are virtual methods of CaloCluster
  auto mon_clusEta = Monitored::Collection("Eta", clus_eta);
  auto mon_badCells = Monitored::Collection("N_BAD_CELLS",N_BAD_CELLS );
  auto mon_engFrac = Monitored::Collection("ENG_FRAC_MAX",N_BAD_CELLS );
  auto mon_size = Monitored::Collection("size",sizeVec );
  auto monmu = Monitored::Scalar("mu",-999.0);
  auto mon_container_size = Monitored::Scalar("container_size", 0.);


  // Looping over cluster maker tools...
  
  time_clusMaker.start();

  auto cells = SG::makeHandle(m_inputCellsKey, ctx);
  ATH_MSG_VERBOSE(" Input Cells : " << cells.name() <<" of size " <<cells->size() );

  float mu(0.0);
  SG::ReadDecorHandle<xAOD::EventInfo,float> eventInfoDecor(m_avgMuKey,ctx);
  if(eventInfoDecor.isPresent()) {
       mu = eventInfoDecor(0);
  }
  unsigned int count_1thrsigma(0), count_2thrsigma(0);
  if (m_monCells && shouldMon) {
     SG::ReadCondHandle<CaloNoise> noiseHdl{m_noiseCDOKey, ctx};
     const CaloNoise *noisep = *noiseHdl;
     for (const auto cell : *cells ) {
        float energy = cell->energy();
        if ( energy <= 0.0 ) continue;
        const CaloDetDescrElement* cdde = cell->caloDDE();
        if (cdde->is_tile() ) continue;
        float thr=noisep->getNoise(cdde->identifyHash(), cell->gain());
        if ( energy <= thr ) continue;
        if ( energy > m_1thr*thr ){
           count_1thrsigma++;
           if ( energy > m_2thr*thr )count_2thrsigma++;
        } // if 1th
     } // end of for over cells
  } // end of if m_monCells
  
   

  for (const ToolHandle<CaloClusterCollectionProcessor>& clproc : m_clusterMakers) {
    
    ATH_CHECK(clproc->execute(ctx, pCaloClusterContainer));
    ATH_MSG_VERBOSE("Executed tool " << clproc->name() );

  }
  time_clusMaker.stop();
  
  ATH_MSG_VERBOSE("......pCaloClusterContainer size: " << pCaloClusterContainer->size());
  //save raw state (uncalibrated)
  for (xAOD::CaloCluster* cl : *pCaloClusterContainer)
    {
#ifndef NDEBUG
      ATH_MSG_VERBOSE("found cluster with state "
		      << cl->signalState() <<  ", calE: " << cl->calE() << ", calEta: " << cl->calEta() << ", calPhi: " << cl->calPhi() << " calM: " <<cl->calM());
      ATH_MSG_VERBOSE(" Cluster Et  = " << cl->et() );
      ATH_MSG_VERBOSE(" Cluster eta = " << cl->eta() );
      ATH_MSG_VERBOSE(" Cluster phi = " << cl->phi() );
#endif
      cl->setRawE(cl->calE());
      cl->setRawEta(cl->calEta());
      cl->setRawPhi(cl->calPhi());
      cl->setRawM(cl->calM());
#ifndef NDEBUG
      ATH_MSG_VERBOSE(" before correction=>Cluster Et  = " << cl->et() );
      ATH_MSG_VERBOSE(" before correction=>Cluster eta = " << cl->eta() );
      ATH_MSG_VERBOSE(" before correction=>Cluster phi = " << cl->phi() );
#endif
    }
  
  
  
  // Looping over cluster correction tools... 
  
  time_clusCorr.start();
    
  for (const ToolHandle<CaloClusterProcessor>& clcorr : m_clusterCorrections) {

    for (xAOD::CaloCluster* cl : *pCaloClusterContainer) {
      if (!m_isSW ||
          (std::abs(cl->eta0()) < 1.45  && clcorr->name().find("37") != std::string::npos) ||
          (std::abs(cl->eta0()) >= 1.45 && clcorr->name().find("55") != std::string::npos) ) {
        ATH_CHECK(clcorr->execute(ctx, cl) );
      }
    }
  }
  time_clusCorr.stop();

  // Decorator handle
  SG::WriteDecorHandle<xAOD::CaloClusterContainer, int> mDecor_ncells(m_mDecor_ncells, ctx);

  // fill monitored variables
  for (xAOD::CaloCluster* cl : *pCaloClusterContainer) {
    
    const CaloClusterCellLink* num_cell_links = cl->getCellLinks();
    if(! num_cell_links) {
      sizeVec.push_back(0);
      mDecor_ncells(*cl) = 0;
    } else {
      sizeVec.push_back(num_cell_links->size()); 
      mDecor_ncells(*cl) = num_cell_links->size();
    }
    clus_phi.push_back(cl->phi());
    clus_eta.push_back(cl->eta());
    N_BAD_CELLS.push_back(cl->getMomentValue(xAOD::CaloCluster::N_BAD_CELLS));
    ENG_FRAC_MAX.push_back(cl->getMomentValue(xAOD::CaloCluster::ENG_FRAC_MAX));
  }
  
  // Finalize the clusters so cells are available in later steps
  SG::WriteHandle<CaloClusterCellLinkContainer> cellLinks (m_clusterCellLinkOutput, ctx);  
  ATH_CHECK(CaloClusterStoreHelper::finalizeClusters (cellLinks, pCaloClusterContainer));
  
  ATH_MSG_DEBUG(" REGTEST: Produced a Cluster Container of Size= " << pCaloClusterContainer->size() );
  if(!pCaloClusterContainer->empty()) {
    ATH_MSG_DEBUG(" REGTEST: Last Cluster Et  = " << (pCaloClusterContainer->back())->et() );
    ATH_MSG_DEBUG(" REGTEST: Last Cluster eta = " << (pCaloClusterContainer->back())->eta() );
    ATH_MSG_DEBUG(" REGTEST: Last Cluster phi = " << (pCaloClusterContainer->back())->phi() );
    mon_container_size = pCaloClusterContainer->size(); // fill monitored variable
  }
  monmu=mu;
  if ( shouldMon ){
    auto moncount_1thrsigma = Monitored::Scalar("count_1thrsigma",-999.0);
    auto moncount_2thrsigma = Monitored::Scalar("count_2thrsigma",-999.0);
    auto mon_container_size_by_mu  = Monitored::Scalar("container_size_by_mu", 0.);
    auto moncount_1thrsigma_by_mu2 = Monitored::Scalar("count_1thrsigma_by_mu2",-999.0);
    auto moncount_2thrsigma_by_mu2 = Monitored::Scalar("count_2thrsigma_by_mu2",-999.0);
    moncount_1thrsigma = count_1thrsigma;
    moncount_2thrsigma = count_2thrsigma;
    if ( mu > 5 ){
      mon_container_size_by_mu  = pCaloClusterContainer->size()/mu; // fill monitored variable
      float onemu2 = 1.0/(mu*mu);
      moncount_1thrsigma_by_mu2 = count_1thrsigma*onemu2;
      moncount_2thrsigma_by_mu2 = count_2thrsigma*onemu2;
    }
    auto monitorIt = Monitored::Group( m_monTool, time_tot, time_clusMaker,  time_clusCorr, mon_container_size, mon_clusEt,
					    mon_clusPhi, mon_clusEta, mon_clusSignalState, mon_clusSize, 
					    mon_badCells, mon_engFrac, mon_size, monmu, moncount_1thrsigma, moncount_2thrsigma, mon_container_size_by_mu, moncount_1thrsigma_by_mu2, moncount_2thrsigma_by_mu2);
  }else{
    auto monitorIt = Monitored::Group( m_monTool, time_tot, time_clusMaker,  time_clusCorr, mon_container_size, mon_clusEt,
					    mon_clusPhi, mon_clusEta, mon_clusSignalState, mon_clusSize, 
					    mon_badCells, mon_engFrac, mon_size, monmu );
  }

  // Stop timer
  time_tot.stop();
  
  return StatusCode::SUCCESS; 
}



