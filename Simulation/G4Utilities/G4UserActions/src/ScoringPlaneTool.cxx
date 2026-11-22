/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "ScoringPlaneTool.h"


namespace G4UA
{

  std::unique_ptr<ScoringPlane>
  ScoringPlaneTool::makeAndFillAction(G4AtlasUserActions& actionList)
  {
    ATH_MSG_DEBUG("Making a ScoringPlane action");
    ScoringPlane::Config config;
    config.plane = m_plane;
    config.pkill = m_pkill;
    config.fname = m_fname;
    auto action = std::make_unique<ScoringPlane>(config);
    actionList.runActions.push_back( action.get() );
    actionList.eventActions.push_back( action.get() );
    actionList.steppingActions.push_back( action.get() );
    return action;
  }

} // namespace G4UA
