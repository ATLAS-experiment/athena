/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGPUEVENT_TRACCCTRACKSTATECOLLECTION_H
#define ACTSGPUEVENT_TRACCCTRACKSTATECOLLECTION_H

// Framework include(s).
#include "AthenaKernel/CLASS_DEF.h"

// Local include(s).
#include "ActsGPUEvent/VecMemSoASGHelpers.h"

// Acts/traccc include(s).
#include <traccc/edm/track_state_collection.hpp>

#include <detray/definitions/algebra.hpp>
using traccc_track_state_collection = traccc::edm::track_state_collection<traccc::default_algebra>;

// Declare identifiers for all the types.
CLASS_DEF(traccc_track_state_collection::buffer, 114559123, 1)
CLASS_DEF(traccc_track_state_collection::data, 39511663, 1)
CLASS_DEF(traccc_track_state_collection::const_data, 242882901, 1)
CLASS_DEF(traccc_track_state_collection::view, 18418060, 1)
CLASS_DEF(traccc_track_state_collection::const_view, 140245398, 1)
CLASS_DEF(traccc_track_state_collection::host, 157967921, 1)

// Declare all conversion rules for StoreGate.
SG_ADD_VECMEM_SOA_CONVERSIONS(traccc_track_state_collection,
                              TrackStateCollection);

#endif  // ACTSGPUEVENT_TRACCCTRACKSTATECOLLECTION_H
