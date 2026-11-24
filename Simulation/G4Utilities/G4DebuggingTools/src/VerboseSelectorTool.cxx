/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "VerboseSelectorTool.h"

namespace G4UA
{

  std::unique_ptr<VerboseSelector>
  VerboseSelectorTool::makeAndFillAction(G4AtlasUserActions& actionList)
  {
    ATH_MSG_DEBUG("Constructing a VerboseSelector");
    VerboseSelector::Config config;
    config.targetEvent = m_targetEvent;
    config.targetTrack = m_targetTrack;
    config.verboseLevel = m_verboseLevel;
    config.verb = m_verb;
    config.Xmin = m_Xmin;
    config.Xmax = m_Xmax;
    config.Ymin = m_Ymin;
    config.Ymax = m_Ymax;
    config.Zmin = m_Zmin;
    config.Zmax = m_Zmax;
    config.targetPdgIDs = m_targetPdgIDs;
    auto action = std::make_unique<VerboseSelector>(config);
    actionList.eventActions.push_back( action.get() );
    actionList.trackingActions.push_back( action.get() );
    actionList.steppingActions.push_back( action.get() );
    return action;
  }

} // namespace G4UA
