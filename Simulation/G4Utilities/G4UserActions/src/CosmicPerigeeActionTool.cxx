/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "CosmicPerigeeActionTool.h"

namespace G4UA
{

  //---------------------------------------------------------------------------
  // Create the action on request
  //---------------------------------------------------------------------------
  std::unique_ptr<CosmicPerigeeAction>
  CosmicPerigeeActionTool::makeAndFillAction(G4AtlasUserActions& actionList)
  {
    ATH_MSG_DEBUG("Creating CosmicPerigeeAction");
    CosmicPerigeeAction::Config config;
    config.pMinPrimary = m_pMinPrimary;
    auto action = std::make_unique<CosmicPerigeeAction>(config);
    actionList.eventActions.push_back( action.get() );
    actionList.trackingActions.push_back( action.get() );
    actionList.steppingActions.push_back( action.get() );
    return action;
  }

}
