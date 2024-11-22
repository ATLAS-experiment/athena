/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGINDETTOOLINTERFACES_ITRIGINDETTRACKSEEDINGTOOL_H
#define TRIGINDETTOOLINTERFACES_ITRIGINDETTRACKSEEDINGTOOL_H

#include <vector>
#include "GaudiKernel/IAlgTool.h"

#include "IRegionSelector/IRoiDescriptor.h"
#include "TrigInDetPattRecoEvent/TrigInDetTracklet.h"
#include "TrigInDetPattRecoEvent/TrigInDetTrackSeedingResult.h"

/** @class ITrigInDetTrackSeedingTool    
provides the abstract interface for track seeding tool for TrigFastTrackFinder
*/

class ITrigInDetTrackSeedingTool : virtual public IAlgTool {
 public:
  DeclareInterfaceID( ITrigInDetTrackSeedingTool, 1, 0 );
  
  virtual TrigInDetTrackSeedingResult findSeeds(const IRoiDescriptor&, std::vector<TrigInDetTracklet>&, const EventContext&) const = 0;
};

#endif
