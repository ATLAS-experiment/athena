// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#ifndef ACTSGPUEVENT_TESTEDCOLLECTIONTYPES_H
#define ACTSGPUEVENT_TESTEDCOLLECTIONTYPES_H

// Local include(s).
#include "ActsGPUEvent/TracccDetectorConditionsDescription.h"
#include "ActsGPUEvent/TracccDetectorDesignDescription.h"
#include "ActsGPUEvent/TracccMeasurementCollection.h"
#include "ActsGPUEvent/TracccSiliconCellCollection.h"
#include "ActsGPUEvent/TracccSiliconClusterCollection.h"

// System include(s).
#include <tuple>

/// Types to run unit tests on
using TestedCollectionTypes =
    std::tuple<traccc::detector_conditions_description,
               traccc::detector_design_description,
               traccc::edm::measurement_collection,
               traccc::edm::silicon_cell_collection,
               traccc::edm::silicon_cluster_collection>;

#endif  // ACTSGPUEVENT_TESTEDCOLLECTIONTYPES_H
