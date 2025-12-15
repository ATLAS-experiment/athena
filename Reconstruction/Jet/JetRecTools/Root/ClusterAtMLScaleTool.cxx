/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "JetRecTools/ClusterAtMLScaleTool.h"

ClusterAtMLScaleTool::ClusterAtMLScaleTool(const std::string& name) : JetConstituentModifierBase(name)
{
}

StatusCode ClusterAtMLScaleTool::initialize() {
    if(m_inputType!=xAOD::Type::CaloCluster) {
        ATH_MSG_ERROR("As the name suggests, ClusterAtMLScaleTool cannot operate on objects of type "
            << m_inputType);
        return StatusCode::FAILURE;
    }
    return StatusCode::SUCCESS;
}

StatusCode ClusterAtMLScaleTool::setClustersToMLScale(xAOD::CaloClusterContainer& cont) const {
  
    const SG::AuxElement::Accessor<double> clusterMLCorrectedEnergyAccessor(m_clusterMLCorrectedEnergyKey.value());

    for(xAOD::CaloCluster* cl : cont ) {
        if (!cl)
            continue;
	
        if (clusterMLCorrectedEnergyAccessor.isAvailable(*cl))
        {
            cl->setCalE( clusterMLCorrectedEnergyAccessor(*cl) );
        }
        else
        {
            ATH_MSG_WARNING("No ML energy decoration '" << m_clusterMLCorrectedEnergyKey.value() 
                    << "' found for cluster with index " << cl->index()
                    << ". Cluster energy is set to EM energy.");
            cl->setCalE( cl->rawE() );
        }
            
        cl->setCalM( cl->rawM() );
        cl->setCalPhi( cl->rawPhi() );
        cl->setCalEta( cl->rawEta() );
    }

  return StatusCode::SUCCESS;
}

StatusCode ClusterAtMLScaleTool::process_impl(xAOD::IParticleContainer* cont) const {
    xAOD::CaloClusterContainer* clust = dynamic_cast<xAOD::CaloClusterContainer*> (cont); // Get CaloCluster container
    if(clust)
        return setClustersToMLScale(*clust);

    return StatusCode::FAILURE;
}


ClusterAtMLScaleTool::~ClusterAtMLScaleTool()= default;
