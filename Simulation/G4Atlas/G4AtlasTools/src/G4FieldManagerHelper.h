/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4FieldManagerHelper_H
#define G4FieldManagerHelper_H

/** @file G4FieldManagerHelper.h
 *
 *  @brief Helper functions for G4FieldManager to adapt to changes in Geant4 interfaces since 10.6
 *
 *  @authors  John Apostolakis, Ben Morgan
 *  @date   2024-08-29
 */

#include "G4FieldManager.hh"

namespace G4FieldManagerHelper
{
  /**
   * Set epsilon step range for a G4FieldManager instance accounting for Geant4 sanity checks
   */
  inline G4bool SetMinAndMaxEpsilonStep( G4FieldManager *fieldMgr, double eps_min, double eps_max ) 
  {
    if( fieldMgr == nullptr ) { return false; }

    if( eps_min > fieldMgr->GetMaximumEpsilonStep() )
    { 
      fieldMgr->SetMaximumEpsilonStep( eps_max );
      fieldMgr->SetMinimumEpsilonStep( eps_min );
    }
    else
    {
      fieldMgr->SetMinimumEpsilonStep( eps_min );
      fieldMgr->SetMaximumEpsilonStep( eps_max );
    }
    return eps_min <= eps_max;
  }
}

#endif
