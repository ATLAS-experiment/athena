/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PFOClusterMLCorrectionAlgorithm_H
#define PFOClusterMLCorrectionAlgorithm_H

////////////////////////////////////////////
/// \class PFOClusterMLCorrectionAlgorithm
///
/// Applies ML corrections to PFO
/// The correction is implemented in the correction tool derived from the IPFOContainerCorrectionTool (NeutralPFOClusterMLCorrectionTool by default)).
/// The corrected PFOs are stored in a shallow copy of the input container.
///
//////////////////////////////////////////////////

#include "eflowRec/IPFOContainerCorrectionTool.h"

#include "xAODPFlow/FlowElement.h"
#include "xAODPFlow/FlowElementContainer.h"
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "StoreGate/DataHandle.h"

class PFOClusterMLCorrectionAlgorithm : public AthReentrantAlgorithm
{

public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  StatusCode initialize();
  StatusCode execute(const EventContext &ctx) const;

protected:
  StatusCode shallowCopyChargedFEContainer(const EventContext &ctx) const;
  StatusCode shallowCopyAndModifyNeutralFEContainer(const EventContext &ctx) const;
    
  ToolHandle<IPFOContainerCorrectionTool> m_correctionTool{this, "PFOContainerCorrectionTool", "NeutralPFOClusterMLCorrectionTool"};
    
  /** ReadHandleKey for eflowCaloObjectContainer */
  SG::ReadHandleKey<xAOD::FlowElementContainer> m_neutralFEContainerReadHandleKey{this, "NeutralPFlowInputContainer", "JetETMissNeutralParticleFlowObjects", "ReadHandleKey for neutral FlowElementContainer"};
  SG::ReadHandleKey<xAOD::FlowElementContainer> m_chargedFEContainerReadHandleKey{this, "ChargedPFlowInputContainer", "JetETMissChargedParticleFlowObjects", "ReadHandleKey for neutral FlowElementContainer"};
   /** WriteHandleKey for neutral FE */
  SG::WriteHandleKey<xAOD::FlowElementContainer> m_neutralFEMLContainerWriteHandleKey{this, "NeutralPFlowOutputContainer", "JetETMissClusterMLCorrectedNeutralParticleFlowObjects", "WriteHandleKey for ML neutral FlowElementContainer"};
  SG::WriteHandleKey<xAOD::FlowElementContainer> m_chargedFEMLContainerWriteHandleKey{this, "ChargedPFlowOutputContainer", "JetETMissClusterMLCorrectedChargedParticleFlowObjects", "WriteHandleKey for shallow copy of charged FlowElementContainer"};

};
#endif
