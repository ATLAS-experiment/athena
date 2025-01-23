/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "CaloUtils/CaloClusterStoreHelper.h"

#include "CaloClusterSnapshot.h"
#include "AthenaKernel/errorcheck.h"
#include "CaloClusterMaker.h"

// -------------------------------------------------------------
// Constructor 
// -------------------------------------------------------------
CaloClusterSnapshot::CaloClusterSnapshot(const std::string& type, 
				   const std::string& name,
				   const IInterface* parent):
  AthAlgTool(type, name, parent),
  m_outputKey("")
{ 
  declareInterface<CaloClusterCollectionProcessor> (this);
  declareProperty("OutputName",m_outputKey);
  declareProperty("CellLinkName",m_cellLinkOutputKey);
  declareProperty("SetCrossLinks",m_setCrossLinks=false);
  declareProperty("FinalClusterContainerName", m_finalContName);
  
}

// -------------------------------------------------------------
// Destructor 
// -------------------------------------------------------------
CaloClusterSnapshot::~CaloClusterSnapshot()
= default;

StatusCode CaloClusterSnapshot::initialize() {
  ATH_CHECK(m_outputKey.initialize());
  if (m_cellLinkOutputKey.key().empty()) {
    m_cellLinkOutputKey = m_outputKey.key() + "_links";
  }
  ATH_CHECK(m_cellLinkOutputKey.initialize());
  return StatusCode::SUCCESS;

}


StatusCode
CaloClusterSnapshot::execute(const EventContext& ctx,
                             xAOD::CaloClusterContainer* clusColl) const
{
  ATH_MSG_DEBUG("Executing CaloClusterSnapshot");
  
  SG::WriteHandle<xAOD::CaloClusterContainer>  outputColl(m_outputKey, ctx);
  
  ATH_CHECK(CaloClusterStoreHelper::AddContainerWriteHandle(outputColl));
  
  CaloClusterStoreHelper::copyContainer(clusColl,outputColl.ptr());

  if (m_setCrossLinks) {
    const size_t nClusters=clusColl->size();
    if (nClusters!=outputColl->size()) {
      ATH_MSG_ERROR("After copy of container: Unequal number of entries! Original " << nClusters << ", copy " << outputColl->size());
      return StatusCode::FAILURE;
    }

    //final cluster to snapshot:
    using ClusterLink_t = ElementLink<xAOD::CaloClusterContainer>;
    ClusterLink_t outputEL (outputColl.name(), 0, ctx);
    for (size_t i=0;i<nClusters;++i) {
      (*clusColl)[i]->setSisterClusterLink(ClusterLink_t(outputEL, i));
    }

    //From snapshot to final cluster
    if (m_setCrossLinks) {
       ClusterLink_t finalEL (m_finalContName, 0, ctx);
       if (finalEL.isValid()) {
	 for (size_t i=0;i<nClusters;++i) {
	   (*outputColl)[i]->setSisterClusterLink(ClusterLink_t(finalEL, i));
	   //	 std::cout << "Setting link to " << m_finalContName << ", index "<< i <<std::endl;
	 }
       }
       else {
	 ATH_MSG_DEBUG("Can't set element link from snapshot to final cluster, likely mismatch in the FinalClusterContainerName");	 
       }
    }
  }

  
  SG::WriteHandle<CaloClusterCellLinkContainer> cellLinks (m_cellLinkOutputKey, ctx);
  ATH_CHECK(CaloClusterStoreHelper::finalizeClusters (cellLinks,
                                                      outputColl.ptr()));
  
  return StatusCode::SUCCESS;
}

