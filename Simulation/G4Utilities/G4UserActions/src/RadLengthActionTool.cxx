/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "RadLengthActionTool.h"

namespace G4UA{


  std::unique_ptr<RadLengthAction>
  RadLengthActionTool::makeAndFillAction(G4AtlasUserActions& actionList)
  {
    ATH_MSG_DEBUG("Constructing a RadLengthAction action");
    RadLengthAction::Config config;
    config.VolumeDepthLevel = m_volumeDepthLevel;
    auto action = std::make_unique<RadLengthAction>(config);
    actionList.runActions.push_back( action.get() );
    actionList.eventActions.push_back( action.get() );
    actionList.steppingActions.push_back( action.get() );
    return action;
  }

} // namespace G4UA
