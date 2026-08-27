/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRK_IDEVICETRKPARAMALGPROVIDERTOOL_H
#define ACTSTRK_IDEVICETRKPARAMALGPROVIDERTOOL_H

#include "GaudiKernel/IAlgTool.h"
#include "GaudiKernel/EventContext.h"

#include <traccc/utils/algorithm.hpp>
#include <traccc/seeding/device/seed_parameter_estimation_algorithm.hpp>

namespace ActsTrk {

class IDeviceTrkParamAlgProviderTool : virtual public IAlgTool {
public:
  DeclareInterfaceID(IDeviceTrkParamAlgProviderTool, 1, 0);

  
  virtual std::pair<std::shared_ptr<const vecmem::copy>, std::shared_ptr<const traccc::device::seed_parameter_estimation_algorithm>> getTrkParamAlgorithm(const EventContext& ctx, const traccc::track_params_estimation_config& trkparam_config) const = 0;
  
};

} // namespace ActsTrk

#endif // ACTSTRK_IDEVICETRKPARAMALGPROVIDERTOOL_H