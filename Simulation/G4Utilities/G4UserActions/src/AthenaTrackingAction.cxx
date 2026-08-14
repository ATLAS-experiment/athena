/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "AthenaTrackingAction.h"

#include <iostream>

#include "G4EventManager.hh"

#include "MCTruth/TrackHelper.h"
#include "MCTruth/TrackInformation.h"
#include "MCTruthBase/AtlasTrajectory.h"

namespace G4UA
{

  //---------------------------------------------------------------------------
  // Constructor
  //---------------------------------------------------------------------------
  AthenaTrackingAction::AthenaTrackingAction(MSG::Level lvl,
                                             int secondarySavingLevel)
    : AthMessaging("AthenaTrackingAction")
    , m_secondarySavingLevel(secondarySavingLevel)
  {
    setLevel(lvl);
  }

  //---------------------------------------------------------------------------
  // Pre-tracking action.
  //---------------------------------------------------------------------------
  void AthenaTrackingAction::PreUserTrackingAction(const G4Track* track)
  {
    ATH_MSG_DEBUG("Starting to track a new particle");

    // Use the TrackHelper code to identify the kind of particle.
    TrackHelper trackHelper(track);

    // The G4Trajectory is currently attached to the TrackingManager. Therefore, only one Trajectory can
    // be handled at the time, so it must be disabled for parallel tracking on the GPU
#ifndef ATHSIMULATION_USE_ADEPT

    // Condition for creating a trajectory object to store truth.
    if (trackHelper.IsPrimary() ||
        (trackHelper.IsRegisteredSecondary() && m_secondarySavingLevel>1) ||
        (trackHelper.IsSecondary() && m_secondarySavingLevel>2))
    {
      ATH_MSG_DEBUG("Preparing an AtlasTrajectory for saving truth");

      // Create a new AtlasTrajectory for this particle
      AtlasTrajectory* trajectory = new AtlasTrajectory(track);

      // Assign the trajectory to the tracking manager.
      // TODO: consider caching the tracking manager once to reduce overhead.
      auto trkMgr = G4EventManager::GetEventManager()->GetTrackingManager();
      //trajectory->setTrackingManager(trkMgr);
      trkMgr->SetStoreTrajectory(true);
      trkMgr->SetTrajectory(trajectory);
    }
#endif
  }

  //---------------------------------------------------------------------------
  // Post-tracking action.
  //---------------------------------------------------------------------------
  void AthenaTrackingAction::PostUserTrackingAction(const G4Track* /*track*/)
  {
    ATH_MSG_DEBUG("Finished tracking a particle");

    // The G4Trajectory is currently attached to the TrackingManager. Therefore, only one Trajectory can
    // be handled at the time, so it must be disabled for parallel tracking on the GPU
#ifndef ATHSIMULATION_USE_ADEPT
    // We are done tracking this particle, so reset the trajectory.
    // TODO: consider caching the tracking manager once to reduce overhead.
    G4EventManager::GetEventManager()->GetTrackingManager()->
      SetStoreTrajectory(false);
#endif
  }

} // namespace G4UA
