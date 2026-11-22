/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "Geant4SetupCheckerTool.h"

namespace G4UA
{

  std::unique_ptr<Geant4SetupChecker>
  Geant4SetupCheckerTool::makeAndFillAction(G4AtlasUserActions& actionList)
  {
    ATH_MSG_DEBUG("Constructing a Geant4SetupChecker");
    auto action = std::make_unique<Geant4SetupChecker>(m_file_location,m_test);
    actionList.runActions.push_back( action.get() );
    return action;
  }

} // namespace G4UA
