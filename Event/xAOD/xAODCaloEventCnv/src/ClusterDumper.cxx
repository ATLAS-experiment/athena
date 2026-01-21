/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// Gaudi/Athena include(s):
#include "AthenaKernel/errorcheck.h"

#include "xAODCaloEvent/CaloClusterContainer.h"
#include "xAODCaloEvent/CaloClusterAuxContainer.h"

// Local include(s):
#include "ClusterDumper.h"

StatusCode ClusterDumper::initialize() {
  ATH_MSG_INFO( "Initializing" );

  std::lock_guard<std::mutex> fileLock{m_fileMutex};
  if (!m_fileName.empty()) {
    m_fileOut.open(m_fileName);
    if (m_fileOut.is_open()) {
      m_out=&m_fileOut;
      ATH_MSG_INFO("Writing to file " << m_fileName);
    }
    else {
      msg(MSG::ERROR) << "Failed to open file " << m_fileName << endmsg;
      return StatusCode::FAILURE;
    }
  }
  else {
    ATH_MSG_INFO("Writing to stdout");
  }

  ATH_CHECK(m_eventInfoKey.initialize());
  ATH_CHECK(m_containerName.initialize());
  return StatusCode::SUCCESS;
}


StatusCode ClusterDumper::finalize() {
  if (m_fileOut.is_open()) {
    m_fileOut.close();
  }
   return StatusCode::SUCCESS;
}
 
StatusCode ClusterDumper::execute() {


  SG::ReadHandle<xAOD::EventInfo> eventInfo (m_eventInfoKey);
  
  SG::ReadHandle<xAOD::CaloClusterContainer> clustercontainer{m_containerName};
  ATH_MSG_DEBUG( "Retrieved clusters with key: " << m_containerName.key() );

  const CaloClusterCellLinkContainer* cclptr=nullptr;
  if (evtStore()->contains<CaloClusterCellLinkContainer>(m_containerName.key()+"_links")) {
    CHECK(evtStore()->retrieve(cclptr,m_containerName.key()+"_links"));
    ATH_MSG_INFO("Found corresponding cell-link container with size " << cclptr->size());
  }
  else
    ATH_MSG_INFO("Did not find corresponding cell-link container");

  std::lock_guard<std::mutex> fileLock{m_fileMutex};
  (*m_out) << "Run " << eventInfo->runNumber() << ", evt " << eventInfo->eventNumber() << " contains " << clustercontainer->size() << " CaloClusters" << std::endl;

  for (const auto itr: *clustercontainer) {
    const xAOD::CaloCluster& cluster=*itr;
    (*m_out) << "Kinematics :" << std::endl;
    (*m_out) << "E=" << cluster.e() << ", eta=" << cluster.eta() << ", phi=" << cluster.phi() << ", m=" << cluster.m() << ", pt=" << cluster.pt() << std::endl;
    (*m_out) << "Eta0=" << cluster.eta0() << ", Phi0=" << cluster.phi0() << std::endl;

    (*m_out) << "TLorentzVector :" << std::endl;
    const xAOD::CaloCluster::FourMom_t& p4=cluster.p4();
    (*m_out) << " p4.E=" << p4.E() << ", x=" << p4.X() << ", y=" << p4.Y() << ", z=" << p4.Z() << ", m=" << p4.M() << ", pt=" << p4.Pt() << std::endl;

    (*m_out) << "Sampling variables :" << std::endl;
    for (unsigned iSamp=0;iSamp<CaloSampling::Unknown;++iSamp) {
	xAOD::CaloCluster_v1::CaloSample s=(CaloSampling::CaloSample)iSamp;
	if (cluster.hasSampling(s)) {
	  (*m_out) << "  Sampling #" << s << ": E=" <<  cluster.eSample(s) << ", eta=" << cluster.etaSample(s) << ", phi=" << cluster.phiSample(s) << std::endl;
	}
      }
      
    

    //(*m_out) << "Auxiliary variables: " << std::endl;
    // const SG::auxid_set_t& auxIds=cluster.container()->getAuxIDs(); //->getDynamicAuxIDs();
    // const size_t idx= cluster.index();
    // for (auto ai: auxIds) {
    //   const std::string& auxName=SG::AuxTypeRegistry::instance().getName(ai);
    //   const std::type_info* ti=SG::AuxTypeRegistry::instance().getType (ai);
    //   if ((*ti)==typeid(float)) {
    // 	const float v=clustercontainer->getData<float>(ai,idx);
    // 	(*m_out) << "  Index=" <<idx << ", Auxid=" << ai << ", Name=" << auxName << " value=" << v << std::endl;
    //   }
    //   else
    // 	(*m_out) << "  Index=" <<idx << ", Auxid=" << ai << ", Name=" << auxName << ", unknown type" << ti->name() << std::endl;
    // }

    constexpr auto allMoments=std::to_array<const char*>({"FIRST_PHI","FIRST_ETA","SECOND_R","SECOND_LAMBDA","DELTA_PHI","DELTA_THETA","DELTA_ALPHA","CENTER_X","CENTER_Y","CENTER_Z","CENTER_MAG","CENTER_LAMBDA","LATERAL","LONGITUDINAL","ENG_FRAC_EM","ENG_FRAC_MAX","ENG_FRAC_CORE","FIRST_ENG_DENS","SECOND_ENG_DENS","ISOLATION","ENG_BAD_CELLS","N_BAD_CELLS","N_BAD_CELLS_CORR","BAD_CELLS_CORR_E","BADLARQ_FRAC","ENG_POS","SIGNIFICANCE","CELL_SIGNIFICANCE","CELL_SIG_SAMPLING","AVG_LAR_Q","AVG_TILE_Q","EM_PROBABILITY","HAD_WEIGHT","OOC_WEIGHT","DM_WEIGHT","TILE_CONFIDENCE_LEVEL","VERTEX_FRACTION","NVERTEX_FRACTION","ENG_CALIB_TOT","ENG_CALIB_OUT_L","ENG_CALIB_OUT_M","ENG_CALIB_OUT_T","ENG_CALIB_DEAD_L","ENG_CALIB_DEAD_M","ENG_CALIB_DEAD_T","ENG_CALIB_EMB0","ENG_CALIB_EME0","ENG_CALIB_TILEG3","ENG_CALIB_DEAD_TOT","ENG_CALIB_DEAD_EMB0","ENG_CALIB_DEAD_TILE0","ENG_CALIB_DEAD_TILEG3","ENG_CALIB_DEAD_EME0","ENG_CALIB_DEAD_HEC0","ENG_CALIB_DEAD_FCAL","ENG_CALIB_DEAD_LEAKAGE","ENG_CALIB_DEAD_UNCLASS","ENG_CALIB_FRAC_EM","ENG_CALIB_FRAC_HAD","ENG_CALIB_FRAC_REST", "ENERGY_Truth"});
    (*m_out) << "Cluster Moments" << std::endl;
    for (const auto& momName : allMoments) {
      SG::AuxElement::Accessor<float> a(momName);
      if (a.isAvailable(cluster)) {
        float v = a(cluster);
        (*m_out) << "   " << momName << ": " << v << std::endl;
      }
    }

    SG::AuxElement::Accessor<xAOD::CaloClusterBadChannelList> a("BadChannelList");
    if (a.isAvailable(cluster)) {
      (*m_out) << "Bad Channel data: " << std::endl;
      for (const auto& bc : cluster.badChannelList()) {
        (*m_out) << "   eta=" << bc.eta() << ", phi=" << bc.phi() << ", layer=" << bc.layer() << ", word=" << bc.badChannel() << std::endl;
      }
    }

    const CaloClusterCellLink* cellLinks = cluster.getCellLinks();
    if (cellLinks) {
      if (m_printCellLinks) {
        (*m_out) << "Cell-links:" << std::endl;
        CaloClusterCellLink::const_iterator lnk_it = cellLinks->begin();
        CaloClusterCellLink::const_iterator lnk_it_e = cellLinks->end();
        for (; lnk_it != lnk_it_e; ++lnk_it) {
          const CaloCell* cell = *lnk_it;
          (*m_out) << "   ID=" << std::hex << cell->ID() << std::dec << ", E=" << cell->e() << ", weight=" << lnk_it.weight() << std::endl;
        }
      } else 
       (*m_out) << "  Nbr of cells: " << cellLinks->size() << std::endl;
    } else {
       (*m_out) << "  No Cell Links found" << std::endl; 
    }

  }//end loop over clusters
  
  return StatusCode::SUCCESS;
}

