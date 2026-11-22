/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "HyperspaceCatcherTool.h"

namespace G4UA
{

  std::unique_ptr<HyperspaceCatcher>
  HyperspaceCatcherTool::makeAndFillAction(G4AtlasUserActions& actionList)
  {
    ATH_MSG_DEBUG("Constructing a HyperspaceCatcher");
    HyperspaceCatcher::Config config;
    config.treatmentLevel = m_treatmentLevel;
    config.killAfter = m_killAfter;
    auto action = std::make_unique<HyperspaceCatcher>(config);
    actionList.runActions.push_back( action.get() );
    actionList.steppingActions.push_back( action.get() );
    return action;
  }

} // namespace G4UA
