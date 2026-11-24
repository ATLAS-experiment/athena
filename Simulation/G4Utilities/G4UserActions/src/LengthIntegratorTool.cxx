/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "LengthIntegratorTool.h"

namespace G4UA
{

  //---------------------------------------------------------------------------
  // Initialize - temporarily here for debugging
  //---------------------------------------------------------------------------
  StatusCode LengthIntegratorTool::initialize()
  {
    ATH_MSG_DEBUG("initialize");

    ATH_CHECK( m_hSvc.retrieve() );

    return StatusCode::SUCCESS;
  }

  //---------------------------------------------------------------------------
  // Create the action on request
  //---------------------------------------------------------------------------
  std::unique_ptr<LengthIntegrator>
  LengthIntegratorTool::makeAndFillAction(G4AtlasUserActions& actionList)
  {
    ATH_MSG_DEBUG("Making a LengthIntegrator action");
    auto action = std::make_unique<LengthIntegrator>( m_hSvc.name(), m_doHistos );
    actionList.eventActions.push_back( action.get() );
    actionList.steppingActions.push_back( action.get() );
    return action;
  }

}
