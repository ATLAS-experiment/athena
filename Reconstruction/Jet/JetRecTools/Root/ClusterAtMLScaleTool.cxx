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

    const std::string& type = m_clusterMLCorrectedEnergyDecorationType.value();
    if (type != "float" && type != "double") {
        ATH_MSG_ERROR(
            "Invalid value for ClusterMLCorrectedEnergyDecorationType: '"
            << type
            << "'. Allowed values are 'float' and 'double'.");
        return StatusCode::FAILURE;
    }

    return StatusCode::SUCCESS;
}


template <typename T>
StatusCode ClusterAtMLScaleTool::setClustersToMLScaleImpl(
      xAOD::CaloClusterContainer& cont,
      const EventContext& ctx) const
  {
      SG::ReadDecorHandle<xAOD::CaloClusterContainer, T> dec(
          m_clusterMLCorrectedEnergyKey, ctx);

      if (!dec.isValid()) {
          ATH_MSG_ERROR("Decoration handle is not valid: "
                        << m_clusterMLCorrectedEnergyKey.key());
          return StatusCode::FAILURE;
      }

      for (xAOD::CaloCluster* cl : cont) {
          if (!cl) continue;

          const double calE = dec(*cl);

          cl->setCalE(calE);
          cl->setCalM(cl->rawM());
          cl->setCalPhi(cl->rawPhi());
          cl->setCalEta(cl->rawEta());
      }

      return StatusCode::SUCCESS;
  }
template StatusCode
ClusterAtMLScaleTool::setClustersToMLScaleImpl<float>(
    xAOD::CaloClusterContainer&,
    const EventContext&) const;

template StatusCode
ClusterAtMLScaleTool::setClustersToMLScaleImpl<double>(
    xAOD::CaloClusterContainer&,
    const EventContext&) const;


StatusCode ClusterAtMLScaleTool::setClustersToMLScale(xAOD::CaloClusterContainer& cont) const {

    const EventContext& ctx = Gaudi::Hive::currentContext();
    return (m_clusterMLCorrectedEnergyDecorationType.value() == "double")
        ? setClustersToMLScaleImpl<double>(cont, ctx)
        : setClustersToMLScaleImpl<float>(cont, ctx);
}


StatusCode ClusterAtMLScaleTool::process_impl(xAOD::IParticleContainer* cont) const {
    xAOD::CaloClusterContainer* clust = dynamic_cast<xAOD::CaloClusterContainer*> (cont); // Get CaloCluster container
    if(clust)
        return setClustersToMLScale(*clust);

    return StatusCode::FAILURE;
}


ClusterAtMLScaleTool::~ClusterAtMLScaleTool()= default;
