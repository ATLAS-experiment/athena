/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRK_IDEVICECLUSTERIZATIONALGPROVIDERTOOL_H
#define ACTSTRK_IDEVICECLUSTERIZATIONALGPROVIDERTOOL_H

// Framework include(s).
#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/IAlgTool.h"

// ActsGPU include(s).
#include "ActsGPUInterfaces/DeviceAlgorithmT.h"

// TracCC include(s).
#include <traccc/clusterization/device/clusterization_algorithm.hpp>
#include <traccc/edm/measurement_collection.hpp>
#include <traccc/edm/silicon_cell_collection.hpp>
#include <traccc/utils/algorithm.hpp>

namespace ActsTrk {

/// Interface for tools providing an Acts device/GPU clusterization and
/// measurement sorting algorithm.
class IDeviceClusterizationAlgProviderTool : virtual public IAlgTool {
 public:
  /// Declare the interface ID for this tool.
  DeclareInterfaceID(IDeviceClusterizationAlgProviderTool, 1, 0);

  /// Get the device specific clusterization algorithm.
  virtual DeviceAlgorithmT<traccc::device::clusterization_algorithm>
  getClusterizationAlgorithm(const EventContext& ctx) const = 0;

  /// Generic type for the measurement sorting algorithms.
  using sorting_algorithm_type =
      traccc::algorithm<traccc::edm::measurement_collection::buffer(
          const traccc::edm::measurement_collection::const_view&)>;

  /// Get the device specific measurement sorting algorithm.
  virtual DeviceAlgorithmT<sorting_algorithm_type> getSortingAlgorithm(
      const EventContext& ctx) const = 0;

};  // class IDeviceClusterizationAlgProviderTool

}  // namespace ActsTrk

#endif  // ACTSTRK_IDEVICECLUSTERIZATIONALGPROVIDERTOOL_H
