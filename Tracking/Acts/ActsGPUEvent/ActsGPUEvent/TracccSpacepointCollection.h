/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGPUEVENT_TRACCCSPACEPOINTCOLLECTION_H
#define ACTSGPUEVENT_TRACCCSPACEPOINTCOLLECTION_H

// Framework include(s).
#include "AthenaKernel/CLASS_DEF.h"

// Local include(s).
#include "ActsGPUEvent/VecMemSoASGHelpers.h"

// Acts/traccc include(s).
#include <traccc/edm/spacepoint_collection.hpp>

// Declare identifiers for all the types.
CLASS_DEF(traccc::edm::spacepoint_collection::buffer, 215537287, 1)
CLASS_DEF(traccc::edm::spacepoint_collection::data, 26064339, 1)
CLASS_DEF(traccc::edm::spacepoint_collection::const_data, 261732737, 1)
CLASS_DEF(traccc::edm::spacepoint_collection::view, 55916572, 1)
CLASS_DEF(traccc::edm::spacepoint_collection::const_view, 16390910, 1)
CLASS_DEF(traccc::edm::spacepoint_collection::host, 214478193, 1)

// Declare all conversion rules for StoreGate.
SG_ADD_VECMEM_SOA_CONVERSIONS(traccc::edm::spacepoint_collection,
                              SpacepointCollection);

#endif  // ACTSGPUEVENT_TRACCCSPACEPOINTCOLLECTION_H
