/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRK_IDEVICETRKPARAMALGPROVIDERTOOL_H
#define ACTSTRK_IDEVICETRKPARAMALGPROVIDERTOOL_H

// Framework include(s).
#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/IAlgTool.h"

// ActsGPU include(s).
#include "ActsGPUInterfaces/DeviceAlgorithmT.h"

// Traccc include(s).
#include <traccc/seeding/device/seed_parameter_estimation_algorithm.hpp>

namespace ActsTrk {

/// Interface for tools providing an Acts device/GPU track parameter estimation
/// algorithm
class IDeviceTrkParamAlgProviderTool : virtual public IAlgTool {
 public:
  /// Declare the interface ID for this tool.
  DeclareInterfaceID(IDeviceTrkParamAlgProviderTool, 1, 0);

  /// Get the device specific track parameter estimation algorithm.
  virtual DeviceAlgorithmT<traccc::device::seed_parameter_estimation_algorithm>
  getAlgorithm(
      const EventContext& ctx,
      const traccc::track_params_estimation_config& trkparam_config) const = 0;

};  // class IDeviceTrkParamAlgProviderTool

}  // namespace ActsTrk

#endif  // ACTSTRK_IDEVICETRKPARAMALGPROVIDERTOOL_H
