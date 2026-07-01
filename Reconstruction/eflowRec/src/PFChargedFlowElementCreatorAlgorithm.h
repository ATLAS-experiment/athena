/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EFLOWREC_PFCHARGEDFLOWELEMENTCREATORALGORITHM_H
#define EFLOWREC_PFCHARGEDFLOWELEMENTCREATORALGORITHM_H

#include "eflowCaloObject.h"

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"

#include "xAODPFlow/FlowElementContainer.h"


class PFChargedFlowElementCreatorAlgorithm : public  AthReentrantAlgorithm {

  public:
  
    using AthReentrantAlgorithm::AthReentrantAlgorithm;

    StatusCode initialize();
    StatusCode execute(const EventContext&) const;

  private:

    /** Create the charged PFO */ 
    void createChargedFlowElements(const eflowCaloObject& energyFlowCaloObject, bool addClusters, SG::WriteHandle<xAOD::FlowElementContainer>& chargedFlowElementContainerWriteHandle) const;

    /** Toggle EOverP algorithm mode, whereby no charged shower subtraction is performed */
    Gaudi::Property<bool> m_eOverPMode{this,"EOverPMode",false,"Toggle EOverP algorithm mode, whereby no charged shower subtraction is performed"};

    /** Toggle whether to decorate FlowElements with addutional data for Combined Performance studies */
    Gaudi::Property<bool> m_addCPData{this,"addCPData",false,"Toggle whether to decorate FlowElements with addutional data for Combined Performance studies "};

    /** ReadHandleKey for eflowCaloObjectContainer */
    SG::ReadHandleKey<eflowCaloObjectContainer> m_eflowCaloObjectContainerReadHandleKey{this,"eflowCaloObjectContainerName","eflowCaloObjects","ReadHandleKey for eflowCaloObjectContainer"};
  
    /** WriteHandleKey for charged PFO */
    SG::WriteHandleKey<xAOD::FlowElementContainer> m_chargedFlowElementContainerWriteHandleKey{this,"FlowElementOutputName","JetETMissChargedParticleFlowObjects","WriteHandleKey for charged FlowElements"};
    
};

#endif
