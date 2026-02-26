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
#include "xAODPFlow/FlowElement.h"
#include "xAODPFlow/FlowElementContainer.h"
#include "AthLinks/ElementLink.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "StoreGate/ReadHandleKey.h"

class NeutralPFOClusterMLCorrectionTool final : public extends<AthAlgTool, IPFOContainerCorrectionTool>
{
public:
  NeutralPFOClusterMLCorrectionTool(const std::string &type, const std::string &name, const IInterface *parent);
  virtual ~NeutralPFOClusterMLCorrectionTool() = default;

  virtual StatusCode initialize() override;
  virtual void correctContainer(xAOD::FlowElementContainer& neutral_pfos, xAOD::FlowElementContainer& charged_pfos) const override;

private:
  // Property to configure the ML energy decoration key
  Gaudi::Property<std::string> m_clusterMLCorrectedEnergyKey{this, "ClusterMLCorrectedEnergyDecorationKey", "clusterE_ML", 
    "Name of the decoration storing the ML-corrected cluster energy"};

  void correctFlowElement(xAOD::FlowElement &pfo, const xAOD::FlowElementContainer &charged_pfos) const;
  void scaleEnergyToAlternativeSignalState(xAOD::FlowElement &pfo, const xAOD::CaloCluster &cls, const std::pair<float,float> &charged_corrections) const;
  std::pair<float,float> getChargedCorrectionsToClusterFromSingleFe(const xAOD::CaloCluster* cls_ptr, const xAOD::FlowElement& fe) const;
  std::pair<float,float> getChargedCorrectionsToCluster(const xAOD::CaloCluster* cls_ptr, const xAOD::FlowElementContainer& charged_pfos) const;
  const xAOD::CaloCluster* getLinkedCluster(const xAOD::FlowElement &pfo) const;
};
#endif
