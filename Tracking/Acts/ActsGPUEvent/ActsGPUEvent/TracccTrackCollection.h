/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGPUEVENT_TRACCCTRACKCOLLECTION_H
#define ACTSGPUEVENT_TRACCCTRACKCOLLECTION_H

// Framework include(s).
#include "AthenaKernel/CLASS_DEF.h"

// Local include(s).
#include "ActsGPUEvent/VecMemSoASGHelpers.h"

// Acts/traccc include(s).
#include <traccc/edm/track_collection.hpp>

#include <detray/definitions/algebra.hpp>
using traccc_track_collection = traccc::edm::track_collection<traccc::default_algebra>;

// Declare identifiers for all the types.
CLASS_DEF(traccc_track_collection::buffer, 250325755, 1)
CLASS_DEF(traccc_track_collection::data, 223152875, 1)
CLASS_DEF(traccc_track_collection::const_data, 98120493, 1)
CLASS_DEF(traccc_track_collection::view, 57354664, 1)
CLASS_DEF(traccc_track_collection::const_view, 52119126, 1)
CLASS_DEF(traccc_track_collection::host, 41385093, 1)

// Declare all conversion rules for StoreGate.
SG_ADD_VECMEM_SOA_CONVERSIONS(traccc_track_collection,
                              TrackCollection);

#endif  // ACTSGPUEVENT_TRACCCTRACKCOLLECTION_H
