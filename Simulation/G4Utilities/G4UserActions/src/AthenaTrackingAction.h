/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4AtlasAlg_AthenaTrackingAction_H
#define G4AtlasAlg_AthenaTrackingAction_H

#include "AthenaBaseComps/AthMessaging.h"

#include "G4UserTrackingAction.hh"

namespace G4UA
{

  /// @class AthenaTrackingAction
  /// @brief User action for pre/post tracking truth handling.
  ///
  class AthenaTrackingAction : public G4UserTrackingAction, public AthMessaging
  {

    public:

      /// Constructor
      AthenaTrackingAction(MSG::Level lvl, int secondarySavingLevel);

      /// @brief Called before tracking a new particle.
      ///
      /// If the track meets certain conditions, we construct a new
      /// AtlasTrajectory which will be used for writing out truth particles
      /// later.
      virtual void PreUserTrackingAction(const G4Track*) override final;

      /// @brief Called after tracking a particle.
      ///
      /// Here we reset the AtlasTrajectory if it was created.
      virtual void PostUserTrackingAction(const G4Track*) override final;

    private:

      /// The saving level for secondaries.
      int m_secondarySavingLevel;

  }; // class AthenaTrackingAction

} // namespace G4UA

#endif
