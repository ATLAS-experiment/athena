/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "NeutralPFOClusterMLCorrectionTool.h"

NeutralPFOClusterMLCorrectionTool::NeutralPFOClusterMLCorrectionTool(const std::string &type, const std::string &name, const IInterface *parent) 
  : base_class(type, name, parent) {}

StatusCode NeutralPFOClusterMLCorrectionTool::initialize() {
  ATH_MSG_DEBUG("Initializing with ClusterMLCorrectedEnergyDecorationKey: " << m_clusterMLCorrectedEnergyKey.value());
  return StatusCode::SUCCESS;
}

void NeutralPFOClusterMLCorrectionTool::correctContainer(xAOD::FlowElementContainer &neutral_pfos, xAOD::FlowElementContainer &charged_pfos) const
{
  for (xAOD::FlowElement *neutral_pfo : neutral_pfos)
  { correctNeutralFlowElement(*neutral_pfo, charged_pfos); }
}

void NeutralPFOClusterMLCorrectionTool::correctNeutralFlowElement(xAOD::FlowElement &neutral_pfo, const xAOD::FlowElementContainer &charged_pfos) const
{
  // The correction is applied only if link to cluster exists.
  // If no link exists, it can be due to negative cell subtraction. In this case, no correction is applied.
  // The corrected neutral pfo energy is calculated from ML-corrected cluster energy, from which the expected charged contribution to the cluster is subtracted.
  // The charged contribution is described in getChargedCorrectionsToClusterFromSingleChargedFe.
  if (neutral_pfo.isCharged())
  {
    throw std::runtime_error("Charged FlowElement found in neutral FlowElementContainer with index " + std::to_string(neutral_pfo.index()));
  }
  const xAOD::CaloCluster *cls = getLinkedCluster(neutral_pfo);
  if (cls == nullptr)
  { return; }
  
  const double cluster_ML_energy = getClusterMLCorrectedEnergy(*cls);
  const float charged_correction = getChargedCorrectionToCluster(cls, charged_pfos);

  const double neutral_pfo_e_corrected = cluster_ML_energy - charged_correction;
  std::cout << "NeutralPFOClusterMLCorrectionTool: neutral pfo index " << neutral_pfo.index() << ", original energy " << neutral_pfo.e() << ", ML-corrected cluster energy " << cluster_ML_energy << ", charged correction " << charged_correction << ", final corrected energy " << neutral_pfo_e_corrected << std::endl;  

  neutral_pfo.setP4(neutral_pfo_e_corrected / cosh(neutral_pfo.eta()), neutral_pfo.eta(), neutral_pfo.phi(), neutral_pfo.m());
}


double NeutralPFOClusterMLCorrectionTool::getClusterMLCorrectedEnergy(const xAOD::CaloCluster &cls) const
{
  const SG::Accessor<double> clusterMLCorrectedEnergyAccessor(m_clusterMLCorrectedEnergyKey.value());
  if (!clusterMLCorrectedEnergyAccessor.isAvailable(cls))
  {
    throw std::runtime_error("No ML energy decoration '" + m_clusterMLCorrectedEnergyKey.value()
              + "' found for cluster with index " + std::to_string(cls.index())
              + ". Returning EM energy.");
  }
  return clusterMLCorrectedEnergyAccessor(cls);
}

float NeutralPFOClusterMLCorrectionTool::getChargedCorrectionToCluster(
    const xAOD::CaloCluster* cls_ptr,
    const xAOD::FlowElementContainer& charged_pfos) const
{
    float corr_total  = 0.f;
    for (const xAOD::FlowElement* charged_fe : charged_pfos) {
      corr_total += getChargedCorrectionsToClusterFromSingleChargedFe(cls_ptr, *charged_fe);
    }
    return corr_total;
}

float NeutralPFOClusterMLCorrectionTool::getChargedCorrectionsToClusterFromSingleChargedFe(
    const xAOD::CaloCluster* cls_ptr,
    const xAOD::FlowElement& charged_fe) const
{
  // The original charged correction is given by the linked weight. This weight is proportional to the expected calo deposit at EM scale for a given track.
  // The ML corrections should make deposits closer to the overlapping part of the shower initialized by the charged particle.
  // Therefore, the correction with ML is made proportional to the track energy scaled by the ratio of the subtracted energy for the concrete cluster to the sum of subtracted energies for all clusters linked to the charged FE.
  // This should be understood as approximation. The whole PFlow algorithm should ideally be re-optimized with the ML corrections, which is beyond the scope of this tool.
  const auto& cl_links = charged_fe.otherObjectLinks();
  const auto& cl_weights = charged_fe.otherObjectWeights();
  
  float this_cluster_subtracted_e = 0.f; // Numerator: sum of subtracted energy corresponding to the concrete cluster
  float sum_all_linked_clusters_subtracted_e = 0.f; // Denominator: sum of subtracted energy corresponding to all clusters linked to the charged FE (should be positive)
  
  auto getClusterFromLink = [&cl_links](size_t i) -> const xAOD::CaloCluster* {
    if (!cl_links[i].isValid()) return nullptr;
    return dynamic_cast<const xAOD::CaloCluster*>(*cl_links[i]);
  };

  for (size_t i = 0; i < cl_links.size(); ++i) {
    const xAOD::CaloCluster* linked_cluster = getClusterFromLink(i);
    if (!linked_cluster) continue;

    const float& energy_subtracted_from_linked_cluster = cl_weights[i];
    sum_all_linked_clusters_subtracted_e += energy_subtracted_from_linked_cluster;
    if (linked_cluster == cls_ptr) {
      this_cluster_subtracted_e += energy_subtracted_from_linked_cluster;
    }
  }
  
  if (this_cluster_subtracted_e > 0.f) {
    std::cout << "NeutralPFOClusterMLCorrectionTool: charged FE index " << charged_fe.index() << ", energy " << charged_fe.e() << ", this_cluster_subtracted_e " << this_cluster_subtracted_e << ", sum_all_linked_clusters_subtracted_e " << sum_all_linked_clusters_subtracted_e << " nlinks to charged fe: " << cl_links.size() << std::endl;
    return charged_fe.e() * this_cluster_subtracted_e / sum_all_linked_clusters_subtracted_e;
  }
  else {
    return 0.f;
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
