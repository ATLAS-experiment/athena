/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// class header include
#include "SimpleFastKiller.h"
// G4 includes
#include "G4FastStep.hh"

void SimpleFastKiller::DoIt(const G4FastTrack&, G4FastStep& fastStep)
{
  fastStep.KillPrimaryTrack();
  fastStep.ProposePrimaryTrackPathLength(0.0);
}
