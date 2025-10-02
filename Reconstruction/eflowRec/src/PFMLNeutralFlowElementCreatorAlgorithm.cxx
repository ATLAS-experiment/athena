/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "PFMLNeutralFlowElementCreatorAlgorithm.h"
#include "xAODCore/ShallowCopy.h"
#include "xAODCaloEvent/CaloClusterContainer.h"

StatusCode PFMLNeutralFlowElementCreatorAlgorithm::initialize()
{

  ATH_CHECK(m_neutralFEContainerReadHandleKey.initialize());
  ATH_CHECK(m_neutralFEMLContainerWriteHandleKey.initialize());

  return StatusCode::SUCCESS;
}

StatusCode PFMLNeutralFlowElementCreatorAlgorithm::execute(const EventContext &ctx) const
{

  ATH_MSG_DEBUG("Executing");

  /* Create Neutral PFOs from all eflowCaloObjects */
  SG::ReadHandle<xAOD::FlowElementContainer> neutralFEContainerReadHandle(m_neutralFEContainerReadHandleKey, ctx);

  std::pair<xAOD::FlowElementContainer *, xAOD::ShallowAuxContainer *> shallowCopyPair = xAOD::shallowCopyContainer(*neutralFEContainerReadHandle);
  std::unique_ptr<xAOD::FlowElementContainer> neutralFEMLContainer{shallowCopyPair.first};
  std::unique_ptr<xAOD::ShallowAuxContainer> neutralFEMLContainerAux{shallowCopyPair.second};
  SG::WriteHandle<xAOD::FlowElementContainer> neutralFEMLContainerWriteHandle(m_neutralFEMLContainerWriteHandleKey, ctx);
  ATH_CHECK(neutralFEMLContainerWriteHandle.record(std::move(neutralFEMLContainer), std::move(neutralFEMLContainerAux)));

  for (unsigned int counter = 0; counter < (*neutralFEContainerReadHandle).size(); counter++)
  {
    const xAOD::FlowElement *thisFE = (*neutralFEContainerReadHandle)[counter];
    xAOD::FlowElement *theCopiedFE = (*neutralFEMLContainerWriteHandle)[counter];

    const ElementLink<xAOD::IParticleContainer> clusterLink = getClusterLink(*thisFE);
    // Correction is applied only if a valid cluster link exists
    if (clusterLink.isValid())
    {
      const xAOD::CaloCluster *cls = static_cast<const xAOD::CaloCluster *>(*clusterLink); // Safe cast since we checked the type in getClusterLink
      scaleEnergyToAlternativeSignalState(*theCopiedFE, *cls);
    }
  }

  return StatusCode::SUCCESS;
}

void PFMLNeutralFlowElementCreatorAlgorithm::scaleEnergyToAlternativeSignalState(xAOD::FlowElement &pfo, const xAOD::CaloCluster &cls) const
{
  // Scale factor is defined as the ratio of the cluster energy in the alternative
  // calibration state (ALTCALIBRATED) to the energy in the EM calibration
  // state (UNCALIBRATED). This scale factor is then applied to the PFO energy.
  // If the EM energy is zero, no scaling is applied.
  const double clusterEMEnergy = cls.rawE();
  const double clusterAltEnergy = cls.altE();
  const double scaleFactor = clusterEMEnergy != 0 ? clusterAltEnergy / clusterEMEnergy : 1.0;

  pfo.setP4(pfo.pt() * scaleFactor, pfo.eta(), pfo.phi(), pfo.m() * scaleFactor);
}

ElementLink<xAOD::IParticleContainer> PFMLNeutralFlowElementCreatorAlgorithm::getClusterLink(const xAOD::FlowElement &pfo) const
{
  // Returns xAOD::Type::CaloCluster type link. There should be at most one such link.
  // It can happen that no such link exists. This can happen due to negative cells subtraction.
  // Empty link is returned if no valid cluster link is found.
  const std::vector<ElementLink<xAOD::IParticleContainer>> &otherObjectLinks = pfo.otherObjectLinks();
  if (otherObjectLinks.size() > 1)
    ATH_MSG_ERROR("Multiple links found for neutral FlowElement with index " << pfo.index());

  bool hasValidLink = !otherObjectLinks.empty() && otherObjectLinks[0].isValid();
  if (hasValidLink)
  {
    if ((**otherObjectLinks[0]).type() != xAOD::Type::CaloCluster)
      ATH_MSG_ERROR("Link for neutral FlowElement with index " << pfo.index() << " is not of type CaloCluster");
    return otherObjectLinks[0];
  }
  else
    return ElementLink<xAOD::IParticleContainer>();
}
