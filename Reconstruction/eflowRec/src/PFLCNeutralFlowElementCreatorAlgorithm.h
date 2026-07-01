/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EFLOWREC_PFLCNEUTRALFLOWELEMENTCREATORALGORITHM_H
#define EFLOWREC_PFLCNEUTRALFLOWELEMENTCREATORALGORITHM_H

#include "eflowCaloObject.h"

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"

#include "xAODCaloEvent/CaloCluster.h"
#include "xAODPFlow/FlowElement.h"
#include "xAODPFlow/FlowElementContainer.h"
#include "xAODPFlow/PFODefs.h"

class PFLCNeutralFlowElementCreatorAlgorithm : public AthReentrantAlgorithm {

public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  StatusCode initialize();
  StatusCode execute(const EventContext& ctx) const;

private:

  /** ReadHandleKey for eflowCaloObjectContainer */
  SG::ReadHandleKey<xAOD::FlowElementContainer> m_neutralFEContainerReadHandleKey{this,"FEInputContainerName","JetETMissNeutralParticleFlowObjects","ReadHandleKey for neutral FlowElementContainer"};
  
  /** WriteHandleKey for neutral FE */
  SG::WriteHandleKey<xAOD::FlowElementContainer> m_neutralFELCContainerWriteHandleKey{this,"FELCOutputName","JetETMissLCNeutralParticleFlowObjects","WriteHandleKey for LC neutral FlowElementContainer"};

};
#endif
