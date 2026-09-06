/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGPUEVENT_TRACCCTRACKCONTAINER_H
#define ACTSGPUEVENT_TRACCCTRACKCONTAINER_H

// Framework include(s).
#include "AthenaKernel/CLASS_DEF.h"

// Acts/traccc include(s).
#include <traccc/edm/track_container.hpp>

#include <detray/definitions/algebra.hpp>
using traccc_track_container = traccc::edm::track_container<traccc::default_algebra>;

// Declare identifiers for all the types.
CLASS_DEF(traccc_track_container::buffer, 204507380, 1)
CLASS_DEF(traccc_track_container::data, 226390216, 1)
CLASS_DEF(traccc_track_container::const_data, 31673186, 1)
CLASS_DEF(traccc_track_container::view, 44592051, 1)
CLASS_DEF(traccc_track_container::const_view, 210100209, 1)
CLASS_DEF(traccc_track_container::host, 74495182, 1)

#endif  // ACTSGPUEVENT_TRACCCTRACKCONTAINER_H
