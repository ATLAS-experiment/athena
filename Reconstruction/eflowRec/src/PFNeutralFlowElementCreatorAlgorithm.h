/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EFLOWREC_PFNEUTRALFLOWELEMENTCREATORALGORITHM_H
#define EFLOWREC_PFNEUTRALFLOWELEMENTCREATORALGORITHM_H

#include "eflowCaloObject.h"

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"

#include "xAODCaloEvent/CaloCluster.h"
#include "xAODPFlow/FlowElement.h"
#include "xAODPFlow/FlowElementContainer.h"
#include "xAODPFlow/PFODefs.h"

class PFNeutralFlowElementCreatorAlgorithm : public AthReentrantAlgorithm {

public:
  
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  StatusCode initialize();
  StatusCode execute(const EventContext& ctx) const;

private:
  /** Create the chargedneutral FE */
  StatusCode createNeutralFlowElement(
    const eflowCaloObject& energyFlowCaloObject,
    xAOD::FlowElementContainer* neutralFEContainer) const;
  
  
                 
  /** Toggle usage of calibration hit truth - false by default */
  Gaudi::Property<bool> m_useCalibHitTruth{
    this,
    "UseCalibHitTruth",
    false,
    "Toggle usage of calibration hit truth - false by default"
  };

  /** Toggle whether to decorate FlowElements with addutional data for Combined Performance studies */
  Gaudi::Property<bool> m_addCPData{this,"addCPData",false,"Toggle whether to decorate FlowElements with addutional data for Combined Performance studies "};

  /** ReadHandleKey for eflowCaloObjectContainer */
  SG::ReadHandleKey<eflowCaloObjectContainer>
    m_eflowCaloObjectContainerReadHandleKey{
      this,
      "eflowCaloObjectContainerName",
      "eflowCaloObjects",
      "ReadHandleKey for eflowCaloObjectContainer"
    };

  /** WriteHandleKey for neutral FE */
  SG::WriteHandleKey<xAOD::FlowElementContainer>
    m_neutralFEContainerWriteHandleKey{
      this,
      "FlowElementOutputName",
      "JetETMissNeutralParticleFlowObjects",
      "WriteHandleKey for neutral FlowElements"
    };  
};
#endif
