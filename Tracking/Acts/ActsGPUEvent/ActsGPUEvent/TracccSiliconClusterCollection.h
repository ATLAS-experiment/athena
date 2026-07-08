/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGPUEVENT_TRACCCSILICONCLUSTERCOLLECTION_H
#define ACTSGPUEVENT_TRACCCSILICONCLUSTERCOLLECTION_H

// Framework include(s).
#include "AthenaKernel/CLASS_DEF.h"

// Local include(s).
#include "ActsGPUEvent/VecMemSoASGHelpers.h"

// Acts/traccc include(s).
#include <traccc/edm/silicon_cluster_collection.hpp>

// Declare identifiers for all the types.
CLASS_DEF(traccc::edm::silicon_cluster_collection::buffer, 112563938, 1)
CLASS_DEF(traccc::edm::silicon_cluster_collection::data, 5293110, 1)
CLASS_DEF(traccc::edm::silicon_cluster_collection::const_data, 36910396, 1)
CLASS_DEF(traccc::edm::silicon_cluster_collection::view, 35145593, 1)
CLASS_DEF(traccc::edm::silicon_cluster_collection::const_view, 72438531, 1)
CLASS_DEF(traccc::edm::silicon_cluster_collection::host, 193707092, 1)

// Declare all conversion rules for StoreGate.
SG_ADD_VECMEM_SOA_CONVERSIONS(traccc::edm::silicon_cluster_collection,
                              SiliconClusterCollection);

#endif  // ACTSGPUEVENT_TRACCCSILICONCLUSTERCOLLECTION_H
