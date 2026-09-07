/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRK_IDEVICESEEDINGALGPROVIDERTOOL_H
#define ACTSTRK_IDEVICESEEDINGALGPROVIDERTOOL_H

// Framework include(s).
#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/IAlgTool.h"

// ActsGPU include(s).
#include "ActsGPUInterfaces/DeviceAlgorithmT.h"

// Traccc include(s).
#include <traccc/gbts_seeding/device/gbts_seeding_algorithm.hpp>
#include <traccc/seeding/device/triplet_seeding_algorithm.hpp>

namespace ActsTrk {

/// Interface for tools providing Acts device/GPU track seeding algorithms
class IDeviceSeedingAlgProviderTool : virtual public IAlgTool {
 public:
  /// Declare the interface ID for this tool.
  DeclareInterfaceID(IDeviceSeedingAlgProviderTool, 1, 0);

  /// Get the device specific triplet seeding algorithm.
  virtual DeviceAlgorithmT<traccc::device::triplet_seeding_algorithm>
  getTripletSeedingAlgorithm(
      const EventContext& ctx, const traccc::seedfinder_config& seedfinder,
      const traccc::seedfilter_config& seedfilter) const = 0;
  /// Get the device specific GBTS seeding algorithm.
  virtual DeviceAlgorithmT<traccc::device::gbts_seeding_algorithm>
  getGBTSAlgorithm(const EventContext& ctx,
                   const traccc::gbts_seedfinder_config& gbts_config) const = 0;

};  // class IDeviceSeedingAlgProviderTool

}  // namespace ActsTrk

#endif  // ACTSTRK_IDEVICESEEDINGALGPROVIDERTOOL_H
