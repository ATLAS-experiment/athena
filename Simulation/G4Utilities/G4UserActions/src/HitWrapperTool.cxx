/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "HitWrapperTool.h"

namespace G4UA
{

  //---------------------------------------------------------------------------
  std::unique_ptr<HitWrapper>
  HitWrapperTool::makeAndFillAction(G4AtlasUserActions& actionList)
  {
    ATH_MSG_DEBUG("Making a HitWrapper action");
    HitWrapper::Config config;
    config.time = m_time;
    auto action = std::make_unique<HitWrapper>(config);
    actionList.eventActions.push_back( action.get() );
    return action;
  }

} // namespace G4UA
