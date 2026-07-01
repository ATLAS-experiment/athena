/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "NeutralPFOClusterMLCorrectionTool.h"

#include "StoreGate/ReadDecorHandle.h"


NeutralPFOClusterMLCorrectionTool::NeutralPFOClusterMLCorrectionTool(const std::string &type, const std::string &name, const IInterface *parent) 
  : base_class(type, name, parent) {}

StatusCode NeutralPFOClusterMLCorrectionTool::initialize() {
  ATH_CHECK(m_clusterMLCorrectedEnergyKey.initialize());
  ATH_MSG_DEBUG("Initializing with ClusterMLCorrectedEnergyDecorationKey: " << m_clusterMLCorrectedEnergyKey.key());
  ATH_MSG_DEBUG("Initializing with MaxAllowedChargedCorrectionFraction: " << m_max_allowed_charged_correction_fraction.value());
  ATH_MSG_DEBUG("Initializing with MinAllowedEMEnergyMeV: " << m_min_allowed_em_energy.value());
  return StatusCode::SUCCESS;
}

void NeutralPFOClusterMLCorrectionTool::correctContainer(xAOD::FlowElementContainer &neutral_pfos, xAOD::FlowElementContainer &/*charged_pfos*/, const EventContext& ctx) const
{

  SG::ReadDecorHandle<xAOD::CaloClusterContainer, double> clusterMLReadHandle(m_clusterMLCorrectedEnergyKey, ctx);
  if (!clusterMLReadHandle.isValid()) {
    throw std::runtime_error("Invalid decoration handle: " +
                             m_clusterMLCorrectedEnergyKey.key());
  }

  for (xAOD::FlowElement *neutral_pfo : neutral_pfos)
  { correctNeutralFlowElement(*neutral_pfo, clusterMLReadHandle); }
}


void NeutralPFOClusterMLCorrectionTool::correctNeutralFlowElement(xAOD::FlowElement &neutral_pfo, const SG::ReadDecorHandle<xAOD::CaloClusterContainer, double>& clusterMLReadHandle) const
{
  // The correction is applied only if link to cluster exists.
  // If no link exists, it can be due to negative cell subtraction. In this case, no correction is applied.
  // The corrected neutral pfo energy is calculated from ML-corrected cluster energy
  if (neutral_pfo.isCharged())
  {
    throw std::runtime_error("Charged FlowElement found in neutral FlowElementContainer with index " + std::to_string(neutral_pfo.index()));
  }
  const xAOD::CaloCluster *cls = getLinkedCluster(neutral_pfo);
  if (cls == nullptr)
  { return; }

  const double cluster_EM_energy = cls->rawE();

  bool apply_ML_correction = (std::abs(neutral_pfo.e() - cluster_EM_energy) <= std::abs(m_max_allowed_charged_correction_fraction.value() * cluster_EM_energy));
  apply_ML_correction = apply_ML_correction && (cluster_EM_energy > m_min_allowed_em_energy.value());
  if (apply_ML_correction)
  {
    const double cluster_ML_energy = clusterMLReadHandle(*cls);
    neutral_pfo.setP4(cluster_ML_energy / cosh(neutral_pfo.eta()), neutral_pfo.eta(), neutral_pfo.phi(), neutral_pfo.m());
  }

}


const xAOD::CaloCluster *NeutralPFOClusterMLCorrectionTool::getLinkedCluster(const xAOD::FlowElement &neutral_pfo) const
{
  // Returns xAOD::Type::CaloCluster type link. There should be at most one such link.
  // It can happen that no such link exists. This can happen due to negative cells subtraction.
  // Empty link is returned if no valid cluster link is found.
  const std::vector<ElementLink<xAOD::IParticleContainer>> &otherObjectLinks = neutral_pfo.otherObjectLinks();
  if (otherObjectLinks.size() > 1)
    throw std::runtime_error("NeutralPFOClusterMLCorrectionTool: Multiple links found for neutral FlowElement with index " + std::to_string(neutral_pfo.index()));

  bool hasValidLink = !otherObjectLinks.empty() && otherObjectLinks[0].isValid();
  if (!hasValidLink)
  { return nullptr; }

  if ((**otherObjectLinks[0]).type() != xAOD::Type::CaloCluster)
    throw std::runtime_error("NeutralPFOClusterMLCorrectionTool: Link for neutral FlowElement with index " + std::to_string(neutral_pfo.index()) + " is not of type CaloCluster");

  return static_cast<const xAOD::CaloCluster *>(*otherObjectLinks[0]);
}
