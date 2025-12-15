/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include <G4Event.hh>

// Local includes
#include "G4AtlasEventAction.h"

namespace G4UA
{

  //---------------------------------------------------------------------------
  // Constructor
  //---------------------------------------------------------------------------
  G4AtlasEventAction::G4AtlasEventAction()
  {
  }

  //---------------------------------------------------------------------------
  // Begin-event action
  //---------------------------------------------------------------------------
  void G4AtlasEventAction::BeginOfEventAction(const G4Event* event)
  {
    // Loop over my pre-actions and apply each one in turn
    for(auto action : m_eventActions){
      action->BeginOfEventAction(event);
      if(event->IsAborted()) {
        if(event->GetUserInformation()) {
          // no user information means an empty event, skip message
          G4cout << "G4AtlasEventAction: Event was aborted in BeginOfEventAction, skipping further actions" << G4endl;
        }
        break;
      }
    }
  }

  //---------------------------------------------------------------------------
  // End-event action
  //---------------------------------------------------------------------------
  void G4AtlasEventAction::EndOfEventAction(const G4Event* event)
  {
    // Loop over my post-actions and apply each one in turn
    for(auto action : m_eventActions){
      action->EndOfEventAction(event);
      if(event->IsAborted()) {
        if(event->GetUserInformation()) {
          // no user information means an empty event, skip message
          G4cout << "G4AtlasEventAction: Event was aborted in EndOfEventAction, skipping further actions" << G4endl;
        }
        break;
      }
    }
  }

  //---------------------------------------------------------------------------
  // Add one action to the list
  //---------------------------------------------------------------------------
  void G4AtlasEventAction::addEventAction(G4UserEventAction* action)
  {
    m_eventActions.push_back(action);
  }

} // namespace G4UA
