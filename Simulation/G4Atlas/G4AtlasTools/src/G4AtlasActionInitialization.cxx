/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#include "G4AtlasTools/G4AtlasActionInitialization.h"
#if G4VERSION_NUMBER >= 1070
#include "G4Exception.hh"
#else
#include "globals.hh"
#endif

G4AtlasActionInitialization::G4AtlasActionInitialization(G4UA::IUserActionSvc* userActionSvc)
  : G4VUserActionInitialization()
  , m_userActionSvc(userActionSvc)
{}


G4AtlasActionInitialization::~G4AtlasActionInitialization()
{}


void G4AtlasActionInitialization::BuildForMaster() const
{

  // Here, can only add run actions
  if (!m_userActionSvc) {
    G4ExceptionDescription description;
    description << "BuildForMaster: UserActionSvc is NULL.";
    G4Exception("G4AtlasActionInitialization", "NoUserActionSvc", FatalException, description);
    abort(); // to keep Coverity happy
  }
  if (m_userActionSvc->initializeActionsMaster().isFailure()) { //Consider renaming to buildActions()?
    G4ExceptionDescription description;
    description << "BuildForMaster: Failed to create UserActions on main thread.";
    G4Exception("G4AtlasActionInitialization", "CouldNotBuildActions", FatalException, description);
    abort(); // to keep Coverity happy
  }
  G4VUserActionInitialization::BuildForMaster();
}


void G4AtlasActionInitialization::Build() const
{
  if (!m_userActionSvc) {
    G4ExceptionDescription description;
    description << "Build: UserActionSvc is NULL.";
    G4Exception("G4AtlasActionInitialization", "NoUserActionSvc", FatalException, description);
    abort(); // to keep Coverity happy
  }
  if (m_userActionSvc->initializeActions().isFailure()) { //Consider renaming to buildActions()?
    G4ExceptionDescription description;
    description << "Build: Failed to create UserActions.";
    G4Exception("G4AtlasActionInitialization", "CouldNotBuildActions", FatalException, description);
    abort(); // to keep Coverity happy
  }
}
