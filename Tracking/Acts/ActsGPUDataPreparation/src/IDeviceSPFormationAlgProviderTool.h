/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRK_IDEVICESPFORMATIONALGPROVIDERTOOL_H
#define ACTSTRK_IDEVICESPFORMATIONALGPROVIDERTOOL_H

// Framework include(s).
#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/IAlgTool.h"

// ActsGPU include(s).
#include "ActsGPUInterfaces/DeviceAlgorithmT.h"

// Traccc include(s).
#include <traccc/seeding/device/silicon_pixel_spacepoint_formation_algorithm.hpp>

namespace ActsTrk {

/// Interface for tools providing an Acts device/GPU spacepoint formation
/// algorithm
class IDeviceSPFormationAlgProviderTool : virtual public IAlgTool {
 public:
  /// Declare the interface ID for this tool.
  DeclareInterfaceID(IDeviceSPFormationAlgProviderTool, 1, 0);

  /// Get the device specific spacepoint formation algorithm.
  virtual DeviceAlgorithmT<
      traccc::device::silicon_pixel_spacepoint_formation_algorithm>
  getAlgorithm(const EventContext& ctx) const = 0;

};  // class IDeviceSPFormationAlgProviderTool

}  // namespace ActsTrk

#endif  // ACTSTRK_IDEVICESPFORMATIONALGPROVIDERTOOL_H
