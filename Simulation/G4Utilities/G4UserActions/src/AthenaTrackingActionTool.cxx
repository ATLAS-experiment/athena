/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "AthenaTrackingActionTool.h"

namespace G4UA
{

  //---------------------------------------------------------------------------
  // Constructor
  //---------------------------------------------------------------------------
  AthenaTrackingActionTool::
  AthenaTrackingActionTool(const std::string& type, const std::string& name,
                           const IInterface* parent)
    : UserActionToolBase<AthenaTrackingAction>(type, name, parent)
    , m_secondarySavingLevel(2)
  {
    declareProperty("SecondarySavingLevel", m_secondarySavingLevel,
      "Three valid options: 1 - Primaries; 2 - StoredSecondaries(default); 3 - All");
  }

  //---------------------------------------------------------------------------
  // Initialize - temporarily here for debugging
  //---------------------------------------------------------------------------
  StatusCode AthenaTrackingActionTool::initialize()
  {
    ATH_MSG_DEBUG( "Initializing " << name() );
    return StatusCode::SUCCESS;
  }

  //---------------------------------------------------------------------------
  // Create the action on request
  //---------------------------------------------------------------------------
  std::unique_ptr<AthenaTrackingAction>
  AthenaTrackingActionTool::makeAndFillAction(G4AtlasUserActions& actionLists)
  {
    ATH_MSG_DEBUG("Constructing an AthenaTrackingAction");
    // Create and configure the action plugin.
    auto action = std::make_unique<AthenaTrackingAction>(
        msg().level(), m_secondarySavingLevel );
    actionLists.trackingActions.push_back( action.get() );
    return action;
  }

}
