/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PFMLNeutralFlowElementCreatorAlgorithm_H
#define PFMLNeutralFlowElementCreatorAlgorithm_H

////////////////////////////////////////////
/// \class PFMLNeutralFlowElementCreatorAlgorithm
///
/// Applies ML corrections to PFO
/// ML corrections are stored in linked CaloClusters in alternative state
/// (ALTCALIBRATED) and applied as a scale factor to the PFO
///
//////////////////////////////////////////////////

#include "eflowRec/eflowCaloObject.h"

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "StoreGate/DataHandle.h"

#include "xAODCaloEvent/CaloCluster.h"
#include "xAODPFlow/FlowElement.h"
#include "xAODPFlow/FlowElementContainer.h"
#include "xAODPFlow/PFODefs.h"

class PFMLNeutralFlowElementCreatorAlgorithm : public AthReentrantAlgorithm
{

public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  StatusCode initialize();
  StatusCode execute(const EventContext &ctx) const;

protected:
  void scaleEnergyToAlternativeSignalState(xAOD::FlowElement &pfo, const xAOD::CaloCluster &cls) const;
  ElementLink<xAOD::IParticleContainer> getClusterLink(const xAOD::FlowElement &pfo) const;

  /** ReadHandleKey for eflowCaloObjectContainer */
  SG::ReadHandleKey<xAOD::FlowElementContainer> m_neutralFEContainerReadHandleKey{this, "FEInputContainerName", "JetETMissNeutralParticleFlowObjects", "ReadHandleKey for neutral FlowElementContainer"};

  /** WriteHandleKey for neutral FE */
  SG::WriteHandleKey<xAOD::FlowElementContainer> m_neutralFEMLContainerWriteHandleKey{this, "FEMLOutputName", "JetETMissMLNeutralParticleFlowObjects", "WriteHandleKey for LC neutral FlowElementContainer"};
};
#endif
