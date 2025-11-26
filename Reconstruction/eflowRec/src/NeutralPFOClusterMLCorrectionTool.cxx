/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "eflowRec/NeutralPFOClusterMLCorrectionTool.h"

NeutralPFOClusterMLCorrectionTool::NeutralPFOClusterMLCorrectionTool(const std::string &type, const std::string &name, const IInterface *parent) 
  : base_class(type, name, parent) {}

StatusCode NeutralPFOClusterMLCorrectionTool::initialize() {
  ATH_MSG_DEBUG("Initializing with ClusterMLCorrectedEnergyDecorationKey: " << m_clusterMLCorrectedEnergyKey.value());
  return StatusCode::SUCCESS;
}

void NeutralPFOClusterMLCorrectionTool::correctContainer(xAOD::FlowElementContainer &container) const
{

  for (xAOD::FlowElement *pfo : container)
  {
    if (pfo->isCharged())
    {
      ATH_MSG_WARNING("NeutralPFOClusterMLCorrectionTool: Charged FlowElement found in neutral FlowElementContainer with index " + std::to_string(pfo->index()));
      continue;
    }

    const xAOD::CaloCluster *cls = getLinkedCluster(*pfo);
    if (cls != nullptr)
      scaleEnergyToAlternativeSignalState(*pfo, *cls);
  }
}

void NeutralPFOClusterMLCorrectionTool::scaleEnergyToAlternativeSignalState(xAOD::FlowElement &pfo, const xAOD::CaloCluster &cls) const
{
  // Scale factor is defined as the ratio of the cluster energy stored in decoration
  // to the energy in the EM calibration state (UNCALIBRATED). This scale factor is then applied to the PFO energy.
  // If the EM energy is zero or if decoration is not found, no scaling is applied.
  const SG::AuxElement::Accessor<double> clusterMLCorrectedEnergyAccessor(m_clusterMLCorrectedEnergyKey.value());
  const double clusterEMEnergy = cls.rawE();
  if (!clusterMLCorrectedEnergyAccessor.isAvailable(cls))
  {
    ATH_MSG_WARNING("NeutralPFOClusterMLCorrectionTool: No ML energy decoration '" << m_clusterMLCorrectedEnergyKey.value() 
              << "' found for cluster with index " << cls.index()
              << ". PFO energy will not be scaled.");
  }
  const double clusterDecorEnergy = clusterMLCorrectedEnergyAccessor.isAvailable(cls) ? clusterMLCorrectedEnergyAccessor(cls) : clusterEMEnergy;
  const double scaleFactor = clusterEMEnergy > FLT_MIN ? clusterDecorEnergy / clusterEMEnergy : 1.0;

  ATH_MSG_DEBUG("NeutralPFOClusterMLCorrectionTool: Scaling PFO with index " << pfo.index()
            << " energy from " << pfo.e() << " to " << (pfo.e() * scaleFactor)
            << " using cluster index " << cls.index()
            << " EM energy " << clusterEMEnergy
            << " Decor energy " << clusterDecorEnergy
            << " scale factor " << scaleFactor);

  pfo.setP4(pfo.pt() * scaleFactor, pfo.eta(), pfo.phi(), pfo.m() * scaleFactor);
}

const xAOD::CaloCluster *NeutralPFOClusterMLCorrectionTool::getLinkedCluster(const xAOD::FlowElement &pfo) const
{
  // Returns xAOD::Type::CaloCluster type link. There should be at most one such link.
  // It can happen that no such link exists. This can happen due to negative cells subtraction.
  // Empty link is returned if no valid cluster link is found.
  const std::vector<ElementLink<xAOD::IParticleContainer>> &otherObjectLinks = pfo.otherObjectLinks();
  if (otherObjectLinks.size() > 1)
    ATH_MSG_ERROR("NeutralPFOClusterMLCorrectionTool: Multiple links found for neutral FlowElement with index " + std::to_string(pfo.index()));

  bool hasValidLink = !otherObjectLinks.empty() && otherObjectLinks[0].isValid();
  if (hasValidLink)
  {
    if ((**otherObjectLinks[0]).type() != xAOD::Type::CaloCluster)
      ATH_MSG_ERROR("NeutralPFOClusterMLCorrectionTool: Link for neutral FlowElement with index " + std::to_string(pfo.index()) + " is not of type CaloCluster");

    return static_cast<const xAOD::CaloCluster *>(*otherObjectLinks[0]);
  }
  else
    return nullptr;
}