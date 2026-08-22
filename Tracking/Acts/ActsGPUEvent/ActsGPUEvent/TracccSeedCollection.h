/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGPUEVENT_TRACCCSEEDCOLLECTION_H
#define ACTSGPUEVENT_TRACCCSEEDCOLLECTION_H

// Framework include(s).
#include "AthenaKernel/CLASS_DEF.h"

// Local include(s).
#include "ActsGPUEvent/VecMemSoASGHelpers.h"

// Acts/traccc include(s).
#include <traccc/edm/seed_collection.hpp>

// Declare identifiers for all the types.
CLASS_DEF(traccc::edm::seed_collection::buffer, 97293304, 1)
CLASS_DEF(traccc::edm::seed_collection::data, 230501228, 1)
CLASS_DEF(traccc::edm::seed_collection::const_data, 62168198, 1)
CLASS_DEF(traccc::edm::seed_collection::view, 193037299, 1)
CLASS_DEF(traccc::edm::seed_collection::const_view, 24704281, 1)
CLASS_DEF(traccc::edm::seed_collection::host, 194066398, 1)

// Declare all conversion rules for StoreGate.
SG_ADD_VECMEM_SOA_CONVERSIONS(traccc::edm::seed_collection,
                              SeedCollection);

#endif  // ACTSGPUEVENT_TRACCCSEEDCOLLECTION_H
