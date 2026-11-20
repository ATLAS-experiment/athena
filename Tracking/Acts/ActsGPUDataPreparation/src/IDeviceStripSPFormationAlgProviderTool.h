/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRK_IDEVICESTRIPSPFORMATIONALGPROVIDERTOOL_H
#define ACTSTRK_IDEVICESTRIPSPFORMATIONALGPROVIDERTOOL_H

// Framework include(s).
#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/IAlgTool.h"

// ActsGPU include(s).
#include "ActsGPUInterfaces/DeviceAlgorithmT.h"
#include "StripSpacepointConditions.h"

// Traccc include(s).
#include <traccc/seeding/device/silicon_strip_spacepoint_formation_algorithm.hpp>

namespace ActsTrk {
struct StripSpacepointInputs;

/// Interface for tools providing an Acts device/GPU spacepoint formation
/// algorithm
class IDeviceStripSPFormationAlgProviderTool : virtual public IAlgTool {
 public:
  /// Declare the interface ID for this tool.
  DeclareInterfaceID(IDeviceStripSPFormationAlgProviderTool, 2, 0);

  /// Upload IOV inputs to independently owned device storage. Return only after
  /// the transfer has completed; the host staging may then be destroyed.
  virtual std::unique_ptr<StripSpacepointConditions> uploadInputs(
      const StripSpacepointInputs& inputs, const EventContext& ctx) const = 0;

  /// Get the device specific spacepoint formation algorithm.
  virtual DeviceAlgorithmT<
      traccc::device::silicon_strip_spacepoint_formation_algorithm>
  getAlgorithm(const EventContext& ctx) const = 0;

};  // class IDeviceStripSPFormationAlgProviderTool

}  // namespace ActsTrk

#endif  // ACTSTRK_IDEVICESTRIPSPFORMATIONALGPROVIDERTOOL_H
