/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRK_IDEVICECLUSTERIZATIONALGPROVIDERTOOL_H
#define ACTSTRK_IDEVICECLUSTERIZATIONALGPROVIDERTOOL_H

#include "GaudiKernel/IAlgTool.h"
#include "GaudiKernel/EventContext.h"

#include <traccc/utils/algorithm.hpp>
#include <traccc/edm/silicon_cell_collection.hpp>
#include <traccc/edm/measurement_collection.hpp>
#include <traccc/clusterization/device/clusterization_algorithm.hpp>
#include <traccc/cuda/clusterization/measurement_sorting_algorithm.hpp>

namespace ActsTrk {

class IDeviceClusterizationAlgProviderTool : virtual public IAlgTool {
public:
  DeclareInterfaceID(IDeviceClusterizationAlgProviderTool, 1, 0);

   virtual std::pair<std::shared_ptr<const vecmem::copy>, std::shared_ptr<const traccc::device::clusterization_algorithm>> getClusterizationAlgorithm(const EventContext& ctx) const = 0;

   using sorting_algorithm_type = traccc::algorithm<traccc::edm::measurement_collection::buffer(const traccc::edm::measurement_collection::const_view&)>;
   virtual std::pair<std::shared_ptr<const vecmem::copy>, std::shared_ptr<const sorting_algorithm_type>> getSortingAlgorithm(const EventContext& ctx) const = 0;

};

} // namespace ActsTrk

#endif // ACTSTRK_IDEVICECLUSTERIZATIONALGPROVIDERTOOL_H