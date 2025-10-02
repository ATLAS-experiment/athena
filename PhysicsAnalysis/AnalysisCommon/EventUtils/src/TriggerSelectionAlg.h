/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EVENTUTILS_TRIGGERSELECTIONALG_H
#define EVENTUTILS_TRIGGERSELECTIONALG_H 1

// STL includes
#include <string>

// FrameWork includes
#include "AthenaBaseComps/AthFilterAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"

// EDM includes
#include "AthContainers/AuxElement.h"

// Forward declarations
namespace Trig{
  class TrigDecisionTool;
}


class TriggerSelectionAlg
  : public ::AthFilterAlgorithm
{
  ///////////////////////////////////////////////////////////////////
  // Public methods:
  ///////////////////////////////////////////////////////////////////
  public:

    /// Constructor with parameters:
    TriggerSelectionAlg( const std::string& name, ISvcLocator* pSvcLocator );

    /// Destructor:
    virtual ~TriggerSelectionAlg();

    /// Athena algorithm's initalize hook
    virtual StatusCode  initialize() override;

    /// Athena algorithm's execute hook
    virtual StatusCode  execute() override;

    /// Athena algorithm's finalize hook
    virtual StatusCode  finalize() override;


  private:

    /// @name The properties that can be defined via the python job options
    /// @{

    /// The ToolHandle for the TrigDecisionTool
    ToolHandle<Trig::TrigDecisionTool> m_trigDecisionTool{ this, "TrigDecisionTool", "Trig::TrigDecisionTool/TrigDecisionTool",
      "The TrigDecisionTool" };

    /// The list of triggers to cut on
    StringArrayProperty m_triggerList{ this, "TriggerList", {},
      "The list of triggers to cut on" };

    /// Decide if we also want to decorate the xAOD::EventInfo object with the pass/fail information
    BooleanProperty m_decoEvtInfo{ this, "DecorateEventInfo", true,
      "Decide if we also want to decorate the xAOD::EventInfo object with the pass/fail information" };

    /// Name of the xAOD::EventInfo object that we want to decorate
    StringProperty m_evtInfoName{ this, "EventInfoName", "EventInfo",
      "Name of the xAOD::EventInfo object that we want to decorate" };

    /// Prefix used for the decoration variables
    StringProperty m_varPrefix{ this, "VarNamePrefix", "pass_",
      "Prefix used for the decoration variables" };

    /// Decide if we also want to decorate the xAOD::EventInfo object with the (full-chain) prescale information
    BooleanProperty m_storePrescaleInfo{ this, "StorePrescaleInfo", false,
      "Decide if we also want to decorate the xAOD::EventInfo object with the full-chain prescale information" };

    /// @}

    /// @name Other private members
    /// @{

    /// The list of all variables names
    std::vector<std::string> m_varNameList;

    /// @}

};



#endif //> !EVENTUTILS_TRIGGERSELECTIONALG_H
