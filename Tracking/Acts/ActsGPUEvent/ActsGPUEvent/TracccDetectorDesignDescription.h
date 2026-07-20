/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGPUEVENT_TRACCCDETECTORDESIGNDESCRIPTION_H
#define ACTSGPUEVENT_TRACCCDETECTORDESIGNDESCRIPTION_H

// Framework include(s).
#include "AthenaKernel/CLASS_DEF.h"

// Local include(s).
#include "ActsGPUEvent/VecMemSoASGHelpers.h"

// Acts/traccc include(s).
#include <traccc/geometry/detector_design_description.hpp>

// Declare identifiers for all the types.
CLASS_DEF(traccc::detector_design_description::buffer, 162125653, 1)
CLASS_DEF(traccc::detector_design_description::data, 254292033, 1)
CLASS_DEF(traccc::detector_design_description::const_data, 105724747, 1)
CLASS_DEF(traccc::detector_design_description::view, 78169602, 1)
CLASS_DEF(traccc::detector_design_description::const_view, 49399024, 1)
CLASS_DEF(traccc::detector_design_description::host, 72524399, 1)

// Declare all conversion rules for StoreGate.
SG_ADD_VECMEM_SOA_CONVERSIONS(traccc::detector_design_description,
                              DetectorDesignDescription);

#endif  // ACTSGPUEVENT_TRACCCDETECTORDESIGNDESCRIPTION_H
