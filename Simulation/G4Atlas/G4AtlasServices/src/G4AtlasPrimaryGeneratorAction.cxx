/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// Local includes
#include "G4AtlasPrimaryGeneratorAction.h"

namespace G4UA
{

  //---------------------------------------------------------------------------
  // Generate primaries action
  //---------------------------------------------------------------------------
  void G4AtlasPrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent)
  {
    // Loop over my pre-actions and apply each one in turn
    for(auto action : m_actions){
      action->GeneratePrimaries(anEvent);
    }
  }

  //---------------------------------------------------------------------------
  // Add one action to the list
  //---------------------------------------------------------------------------
  void G4AtlasPrimaryGeneratorAction::addPrimaryGeneratorAction(G4VUserPrimaryGeneratorAction* action)
  {
    m_actions.push_back(action);
  }

} // namespace G4UA
