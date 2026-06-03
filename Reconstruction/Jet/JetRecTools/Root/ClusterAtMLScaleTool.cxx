/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "JetRecTools/ClusterAtMLScaleTool.h"

#include "StoreGate/ReadDecorHandle.h"
#include "GaudiKernel/ThreadLocalContext.h"


ClusterAtMLScaleTool::ClusterAtMLScaleTool(const std::string& name) : JetConstituentModifierBase(name)
{
}


StatusCode ClusterAtMLScaleTool::initialize() {

    if (m_inputType != xAOD::Type::CaloCluster) {
        ATH_MSG_ERROR("As the name suggests, ClusterAtMLScaleTool cannot operate on objects of type "
                      << m_inputType);
        return StatusCode::FAILURE;
    }

    ATH_CHECK(m_clusterMLCorrectedEnergyKey.initialize());
    return StatusCode::SUCCESS;
}



StatusCode ClusterAtMLScaleTool::setClustersToMLScale(xAOD::CaloClusterContainer& cont) const {

    const EventContext& ctx = Gaudi::Hive::currentContext();

    SG::ReadDecorHandle<xAOD::CaloClusterContainer, double> dec(
        m_clusterMLCorrectedEnergyKey, ctx);

    if (!dec.isValid()) {
        ATH_MSG_ERROR("Decoration handle is not valid: " 
                      << m_clusterMLCorrectedEnergyKey.key());
        return StatusCode::FAILURE;
    }
    
    if (!dec.isAvailable()) {
        ATH_MSG_ERROR("Missing decoration: " << m_clusterMLCorrectedEnergyKey.key());
        return StatusCode::FAILURE;
    }


    for (xAOD::CaloCluster* cl : cont) {
        if (!cl) continue;
       
        cl->setCalE(dec(*cl));
        cl->setCalM(cl->rawM());
        cl->setCalPhi(cl->rawPhi());
        cl->setCalEta(cl->rawEta());
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
