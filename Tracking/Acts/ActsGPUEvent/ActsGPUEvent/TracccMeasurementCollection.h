/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGPUEVENT_TRACCCMEASUREMENTCOLLECTION_H
#define ACTSGPUEVENT_TRACCCMEASUREMENTCOLLECTION_H

// Framework include(s).
#include "AthenaKernel/CLASS_DEF.h"

// Local include(s).
#include "ActsGPUEvent/VecMemSoASGHelpers.h"

// Acts/traccc include(s).
#include <traccc/edm/measurement_collection.hpp>

// Declare identifiers for all the types.
CLASS_DEF(traccc::edm::measurement_collection::buffer, 219498114, 1)
CLASS_DEF(traccc::edm::measurement_collection::data, 36039726, 1)
CLASS_DEF(traccc::edm::measurement_collection::const_data, 144720388, 1)
CLASS_DEF(traccc::edm::measurement_collection::view, 193707825, 1)
CLASS_DEF(traccc::edm::measurement_collection::const_view, 245792427, 1)
CLASS_DEF(traccc::edm::measurement_collection::host, 127310188, 1)

// Declare all conversion rules for StoreGate.
SG_ADD_VECMEM_SOA_CONVERSIONS(traccc::edm::measurement_collection,
                              MeasurementCollection);

#endif  // ACTSGPUEVENT_TRACCCMEASUREMENTCOLLECTION_H
