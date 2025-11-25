/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "HIClusterCopier.h"
#include "CaloUtils/CaloClusterStoreHelper.h"
#include "xAODBase/IParticleHelpers.h"
#include "xAODCore/ShallowAuxContainer.h"
#include "xAODCore/ShallowCopy.h"

HIClusterCopier::HIClusterCopier(const std::string& name, ISvcLocator* pSvcLocator)
	: AthReentrantAlgorithm(name,pSvcLocator)
{
}

StatusCode HIClusterCopier::initialize()
{
	//First we initialize keys - after initialization they are frozen
	ATH_CHECK( m_inputKey.initialize() );
	ATH_CHECK( m_outputKey.initialize() );

	return StatusCode::SUCCESS;
}

StatusCode HIClusterCopier::execute(const EventContext &ctx) const
{
	// retrieve input
	SG::ReadHandle<xAOD::CaloClusterContainer> inputClusterHandle(m_inputKey, ctx);

	if(inputClusterHandle.isValid()) {
		ATH_MSG_DEBUG("Retrieval of CaloClusterContainer was OK");
	} else {
		ATH_MSG_ERROR("Retrieval of CaloClusterContainer failed");
		return StatusCode::FAILURE;
	}

	ATH_MSG_DEBUG("Copying CaloClusters");
 
	//make the container
	SG::WriteHandle<xAOD::CaloClusterContainer> outputClusterColl ( m_outputKey, ctx );
        ATH_CHECK(CaloClusterStoreHelper::AddContainerWriteHandle(outputClusterColl));
	// deep copy
	std::unique_ptr<xAOD::CaloClusterContainer> copiedClusters = std::make_unique<xAOD::CaloClusterContainer>();
	std::unique_ptr<xAOD::CaloClusterAuxContainer> copiedClustersAux = std::make_unique<xAOD::CaloClusterAuxContainer>();

	copiedClusters->setStore (copiedClustersAux.get());

	for(const xAOD::CaloCluster* cl : *inputClusterHandle){
		std::unique_ptr<xAOD::CaloCluster> copiedCl = std::make_unique<xAOD::CaloCluster>(*cl);
		outputClusterColl->push_back(std::move(copiedCl));
	}

  	return StatusCode::SUCCESS;
}

StatusCode HIClusterCopier::finalize()
{
	return StatusCode::SUCCESS;
}
