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
  std::cout << "NeutralPFOClusterMLCorrectionTool: Starting correction of " << neutral_pfos.size() << " neutral PFOs using " << charged_pfos.size() << " charged PFOs." << std::endl;
  for (xAOD::FlowElement *neutral_pfo : neutral_pfos)
  { correctFlowElement(*neutral_pfo, charged_pfos); }
}

void NeutralPFOClusterMLCorrectionTool::correctFlowElement(xAOD::FlowElement &neutral_pfo, const xAOD::FlowElementContainer &charged_pfos) const
{
  if (neutral_pfo.isCharged())
  {
    ATH_MSG_WARNING("NeutralPFOClusterMLCorrectionTool: Charged FlowElement found in neutral FlowElementContainer with index " + std::to_string(neutral_pfo.index()));
    return;
  }
  std::cout << "NeutralPFOClusterMLCorrectionTool: Get linked cluster for neutral PFO with index " << neutral_pfo.index() << std::endl;
  const xAOD::CaloCluster *cls = getLinkedCluster(neutral_pfo);
  if (cls == nullptr)
  { return; }
  
  std::cout << "NeutralPFOClusterMLCorrectionTool: Found linked cluster with index " << cls->index() << " for neutral PFO with index " << neutral_pfo.index() << std::endl;
  std::pair<float,float> charged_corrections = getChargedCorrectionsToCluster(cls, charged_pfos);
  scaleEnergyToAlternativeSignalState(neutral_pfo, *cls, charged_corrections);
}

std::pair<float,float>
NeutralPFOClusterMLCorrectionTool::getChargedCorrectionsToCluster(
    const xAOD::CaloCluster* cls_ptr,
    const xAOD::FlowElementContainer& charged_pfos) const
{
    std::pair<float, float> corr_total  = {0.f,0.f};
    for (const xAOD::FlowElement* fe : charged_pfos) {
      const std::pair<float,float> corr = getChargedCorrectionsToClusterFromSingleFe(cls_ptr, *fe);
      corr_total.first += corr.first;
      corr_total.second += corr.second;
    }
    std::cout << "NeutralPFOClusterMLCorrectionTool: Total charged correction to cluster with index " << cls_ptr->index() << " is corr_pfo_e: " << corr_total.first << " and corr_weight_e: " << corr_total.second << std::endl;
    return corr_total;
}

std::pair<float,float>
NeutralPFOClusterMLCorrectionTool::getChargedCorrectionsToClusterFromSingleFe(
    const xAOD::CaloCluster* cls_ptr,
    const xAOD::FlowElement& fe) const
{
  float corr_pfo_e  = 0.f;
  float corr_weight_e   = 0.f;
  // FlowElements store links to clusters with weights
  const auto& cl_links = fe.otherObjectLinks();
  const auto& cl_weights = fe.otherObjectWeights();

  // Loop over all cluster links of this FE
  bool has_matched_cluster = false;
  for (size_t i = 0; i < cl_links.size(); ++i) {

    const ElementLink<xAOD::IParticleContainer>& link = cl_links[i];
    float weight = cl_weights[i];

    // Skip invalid links
    if (!link.isValid()) continue;

    // Check if this link points to the same cluster
    const xAOD::IParticle* ip = *link;
    const xAOD::CaloCluster* linked_cluster =
      dynamic_cast<const xAOD::CaloCluster*>(ip);

    if (!linked_cluster) continue;

    // Compare pointer identity
    if (linked_cluster == cls_ptr) {
      has_matched_cluster = true;
      corr_weight_e  += weight;
    }
  }
  
  if (has_matched_cluster)
  { corr_pfo_e = fe.e(); }

  return {corr_pfo_e, corr_weight_e};
}


void NeutralPFOClusterMLCorrectionTool::scaleEnergyToAlternativeSignalState(xAOD::FlowElement &neutral_pfo, const xAOD::CaloCluster &cls, const std::pair<float,float> &charged_corrections) const
{
  // Scale factor is defined as the ratio of the cluster energy stored in decoration
  // to the energy in the EM calibration state (UNCALIBRATED). This scale factor is then applied to the PFO energy.
  // If the EM energy is zero or if decoration is not found, no scaling is applied.
  const SG::Accessor<double> clusterMLCorrectedEnergyAccessor(m_clusterMLCorrectedEnergyKey.value());
  const double clusterEMEnergy = cls.rawE();
  if (!clusterMLCorrectedEnergyAccessor.isAvailable(cls))
  {
    ATH_MSG_WARNING("NeutralPFOClusterMLCorrectionTool: No ML energy decoration '" << m_clusterMLCorrectedEnergyKey.value() 
              << "' found for cluster with index " << cls.index()
              << ". PFO energy will not be scaled.");
  }
  const double clusterDecorEnergy = clusterMLCorrectedEnergyAccessor.isAvailable(cls) ? clusterMLCorrectedEnergyAccessor(cls) : clusterEMEnergy;
  const double scaleFactor = clusterEMEnergy > FLT_MIN ? clusterDecorEnergy / clusterEMEnergy : 1.0;

  const float corr_charged_pfo_e = charged_corrections.first;
  const float corr_weight_e = charged_corrections.second;

  const double neutral_pfo_e_corrected = (neutral_pfo.e() + corr_weight_e) * scaleFactor - corr_charged_pfo_e;


  std::cout <<"NeutralPFOClusterMLCorrectionTool: Scaling PFO with index: " << neutral_pfo.index()
            << " charged pfo energy: " << corr_charged_pfo_e << " weighted charged pfo energy: " << corr_weight_e << " =?= (clusterEMEnergy - neutral_pfo.e()): " << clusterEMEnergy - neutral_pfo.e() 
            << " scale factor: " << scaleFactor
            << " energy from: " << neutral_pfo.e() << " to: " << neutral_pfo_e_corrected
            << " (neutral_pfo.e() + corr_weight_e) / clusterEMEnergy: " << (neutral_pfo.e() + corr_weight_e) / clusterEMEnergy << " =?= 1"
            << " using cluster index: " << cls.index()
            << " EM energy: " << clusterEMEnergy
            << " Decor energy: " << clusterDecorEnergy << std::endl;

  neutral_pfo.setP4(neutral_pfo_e_corrected / cosh(neutral_pfo.eta()), neutral_pfo.eta(), neutral_pfo.phi(), neutral_pfo.m());
}

const xAOD::CaloCluster *NeutralPFOClusterMLCorrectionTool::getLinkedCluster(const xAOD::FlowElement &neutral_pfo) const
{
  // Returns xAOD::Type::CaloCluster type link. There should be at most one such link.
  // It can happen that no such link exists. This can happen due to negative cells subtraction.
  // Empty link is returned if no valid cluster link is found.
  const std::vector<ElementLink<xAOD::IParticleContainer>> &otherObjectLinks = neutral_pfo.otherObjectLinks();
  if (otherObjectLinks.size() > 1)
    ATH_MSG_ERROR("NeutralPFOClusterMLCorrectionTool: Multiple links found for neutral FlowElement with index " + std::to_string(neutral_pfo.index()));

  bool hasValidLink = !otherObjectLinks.empty() && otherObjectLinks[0].isValid();
  if (hasValidLink)
  {
    if ((**otherObjectLinks[0]).type() != xAOD::Type::CaloCluster)
      ATH_MSG_ERROR("NeutralPFOClusterMLCorrectionTool: Link for neutral FlowElement with index " + std::to_string(neutral_pfo.index()) + " is not of type CaloCluster");

    return static_cast<const xAOD::CaloCluster *>(*otherObjectLinks[0]);
  }
  else
    return nullptr;
}
