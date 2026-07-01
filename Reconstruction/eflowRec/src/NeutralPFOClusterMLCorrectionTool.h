/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EFLOWREC_NeutralPFOClusterMLCorrectionTool_H
#define EFLOWREC_NeutralPFOClusterMLCorrectionTool_H

////////////////////////////////////////////
/// \class NeutralPFOClusterMLCorrectionTool
///
/// Applies ML corrections to PFO
/// ML corrections are stored in linked CaloClusters as decorations.
/// The correction is applied as a scale factor to the PFO.
///
//////////////////////////////////////////////////

#include "IPFOContainerCorrectionTool.h"

#include "xAODCaloEvent/CaloCluster.h"
#include "xAODCaloEvent/CaloClusterContainer.h"
#include "xAODPFlow/FlowElement.h"
#include "xAODPFlow/FlowElementContainer.h"
#include "AthLinks/ElementLink.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadDecorHandleKey.h"
#include "StoreGate/ReadDecorHandle.h"

class NeutralPFOClusterMLCorrectionTool final : public extends<AthAlgTool, IPFOContainerCorrectionTool>
{
public:
  NeutralPFOClusterMLCorrectionTool(const std::string &type, const std::string &name, const IInterface *parent);
  virtual ~NeutralPFOClusterMLCorrectionTool() = default;

  virtual StatusCode initialize() override;
  virtual void correctContainer(xAOD::FlowElementContainer& neutral_pfos, xAOD::FlowElementContainer& charged_pfos, const EventContext& ctx) const override;

private:
  // Property to configure the ML energy decoration key
  SG::ReadDecorHandleKey<xAOD::CaloClusterContainer> m_clusterMLCorrectedEnergyKey{
      this,
      "ClusterMLCorrectedEnergyDecorationKey",
      "CaloCalTopoClusters.clusterE_ML",
      "Decoration storing ML-corrected cluster energy"
  };

  Gaudi::Property<float> m_max_allowed_charged_correction_fraction{this, "MaxAllowedChargedCorrectionFraction", 0.001, "ClusterML correction will be applied only if |npfo_E - cls_EM_E| <= |MaxAllowedChargedCorrectionFraction * cls_EM_E|"};
  Gaudi::Property<float> m_min_allowed_em_energy{this,"MinAllowedEMEnergyMeV", 300, "Minimum allowed energy in MeV of matched cluster at EM scale. ClusterML correction will not be applied below this limit."};

  void correctNeutralFlowElement(xAOD::FlowElement &pfo, const SG::ReadDecorHandle<xAOD::CaloClusterContainer, double>& clusterMLReadHandle) const;
  const xAOD::CaloCluster* getLinkedCluster(const xAOD::FlowElement &pfo) const;
};
#endif
