/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ASSOCIATIONUTILS_OVERLAPREMOVALTESTALG_H
#define ASSOCIATIONUTILS_OVERLAPREMOVALTESTALG_H

// Framework includes
#include "GaudiKernel/ToolHandle.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "AthenaBaseComps/AthAlgorithm.h"

// EDM includes
#include "xAODEventInfo/EventInfo.h"
#include "xAODJet/JetContainer.h"

// Local includes
#include "JetSelectorTools/IEventCleaningTool.h"

/// A testing algorithm for the dual-use event cleaning tool in Athena
///
/// @author Julia Gonski <j.gonski@cern.ch>
///
class EventCleaningTestAlg : public AthAlgorithm
{

  public:

    /// Standard algorithm constructor
    EventCleaningTestAlg(const std::string& name, ISvcLocator* svcLoc);

    /// Initialize the algorithm
    virtual StatusCode initialize() override;

    /// Execute the algorithm
    virtual StatusCode execute() override;

  private:

    ToolHandle<ECUtils::IEventCleaningTool> m_ecTool{this, "EventCleaningTool","ECUtils::EventCleaningTool/EventCleaningTool" };

    SG::ReadHandleKey<xAOD::JetContainer> m_jetKey{this, "JetCollectionName", "AntiKt4EMPFlowJets",
                                                   "Jet collection name"};
    SG::ReadHandleKey<xAOD::EventInfo> m_evtKey{this, "EventInfoKey",
                                                "EventInfo"};
    /// Configuration
    Gaudi::Property<std::string> m_prefix{this, "EventCleanPrefix" , "",
                                  "Input name of event cleaning decorator prefix" };
    Gaudi::Property<std::string> m_cleaningLevel{this, "CleaningLevel" , "LooseBad",
                                                "Input cleaning level"};
    Gaudi::Property<bool> m_doEvent{this, "doEvent" , true, "Decorate the EventInfo"};

    SG::WriteDecorHandleKey<xAOD::EventInfo> m_evtInfoDecor{this, "EvtDecorKey", "" , "Will be overwritten in initialze"};    
};

#endif
