/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
#ifndef TRIGCOSTMONITOR_TRIGCOSTFINALIZEALG_H
#define TRIGCOSTMONITOR_TRIGCOSTFINALIZEALG_H 1

#include "TrigCompositeUtils/TrigCompositeUtils.h"
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "TrigCostMonitor/ITrigCostSvc.h"


/**
 * @class TrigCostFinalizeAlg
 * @brief Simple class to end the Cost Monitoring at the end of acceptedEventTopSeq
 **/
class TrigCostFinalizeAlg : public AthReentrantAlgorithm
{ 
 public: 

    TrigCostFinalizeAlg( const std::string& name, ISvcLocator* pSvcLocator );

    virtual StatusCode  initialize() override;
    virtual StatusCode  execute( const EventContext& context ) const override;

  private:
    ServiceHandle<ITrigCostSvc> m_trigCostSvcHandle {
        this, "TrigCostSvc", "TrigCostSvc", "The trigger cost service" };

    SG::WriteHandleKey<xAOD::TrigCompositeContainer> m_costWriteHandleKey { this, "CostWriteHandleKey", "HLT_TrigCostContainer",
        "TrigComposite collections summarising the HLT execution" };

};

#endif //> !TRIGCOSTMONITOR_TRIGCOSTFINALIZEALG_H

