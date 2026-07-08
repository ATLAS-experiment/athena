/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGPUEVENT_TRACCCSILICONCELLCOLLECTION_H
#define ACTSGPUEVENT_TRACCCSILICONCELLCOLLECTION_H

// Framework include(s).
#include "AthenaKernel/CLASS_DEF.h"

// Local include(s).
#include "ActsGPUEvent/VecMemSoASGHelpers.h"

// Acts/traccc include(s).
#include <traccc/edm/silicon_cell_collection.hpp>

// Declare identifiers for all the types.
CLASS_DEF(traccc::edm::silicon_cell_collection::buffer, 37225663, 1)
CLASS_DEF(traccc::edm::silicon_cell_collection::data, 37320835, 1)
CLASS_DEF(traccc::edm::silicon_cell_collection::const_data, 156860361, 1)
CLASS_DEF(traccc::edm::silicon_cell_collection::view, 256228120, 1)
CLASS_DEF(traccc::edm::silicon_cell_collection::const_view, 25788242, 1)
CLASS_DEF(traccc::edm::silicon_cell_collection::host, 157215173, 1)

// Declare all conversion rules for StoreGate.
SG_ADD_VECMEM_SOA_CONVERSIONS(traccc::edm::silicon_cell_collection,
                              SiliconCellCollection);

#endif  // ACTSGPUEVENT_TRACCCSILICONCELLCOLLECTION_H
