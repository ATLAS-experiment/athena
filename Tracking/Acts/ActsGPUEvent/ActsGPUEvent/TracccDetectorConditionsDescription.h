/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGPUEVENT_TRACCCDETECTORCONDITIONSDESCRIPTION_H
#define ACTSGPUEVENT_TRACCCDETECTORCONDITIONSDESCRIPTION_H

// Framework include(s).
#include "AthenaKernel/CLASS_DEF.h"
#include "AthenaKernel/CondCont.h"

// Local include(s).
#include "ActsGPUEvent/VecMemSoASGHelpers.h"

// Acts/traccc include(s).
#include <traccc/geometry/detector_conditions_description.hpp>

// Declare identifiers for all the types.
CLASS_DEF(traccc::detector_conditions_description::buffer, 21218053, 1)
CLASS_DEF(traccc::detector_conditions_description::data, 60224057, 1)
CLASS_DEF(traccc::detector_conditions_description::const_data, 262230451, 1)
CLASS_DEF(traccc::detector_conditions_description::view, 148971970, 1)
CLASS_DEF(traccc::detector_conditions_description::const_view, 159787296, 1)
CLASS_DEF(traccc::detector_conditions_description::host, 189199167, 1)


CONDCONT_DEF(traccc::detector_conditions_description::host, 9999001);
CONDCONT_DEF(traccc::detector_conditions_description::buffer, 9999002);

// Declare all conversion rules for StoreGate.
SG_ADD_VECMEM_SOA_CONVERSIONS(traccc::detector_conditions_description,
                              DetectorConditionsDescription);

#endif  // ACTSGPUEVENT_TRACCCDETECTORCONDITIONSDESCRIPTION_H
