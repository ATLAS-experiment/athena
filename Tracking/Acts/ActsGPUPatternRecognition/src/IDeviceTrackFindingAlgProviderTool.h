/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRK_IDEVICETRACKFINDINGALGPROVIDERTOOL_H
#define ACTSTRK_IDEVICETRACKFINDINGALGPROVIDERTOOL_H

#include "GaudiKernel/IAlgTool.h"
#include "GaudiKernel/EventContext.h"

// ActsGPU include(s).
#include "ActsGPUInterfaces/DeviceAlgorithmT.h"

#include <traccc/utils/algorithm.hpp>
#include <traccc/edm/seed_collection.hpp>
#include <traccc/finding/device/combinatorial_kalman_filter_algorithm.hpp>

namespace ActsTrk {

class IDeviceTrackFindingAlgProviderTool : virtual public IAlgTool {
public:
  DeclareInterfaceID(IDeviceTrackFindingAlgProviderTool, 1, 0);

  
    virtual DeviceAlgorithmT<traccc::device::combinatorial_kalman_filter_algorithm> getAlgorithm(const EventContext& ctx, const traccc::finding_config& trkfinding_config) const = 0;
  
};

} // namespace ActsTrk

#endif // ACTSTRK_IDEVICETRACKFINDINGALGPROVIDERTOOL_H