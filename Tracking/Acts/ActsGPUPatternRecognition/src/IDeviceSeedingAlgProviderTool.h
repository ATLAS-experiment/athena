/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRK_IDEVICESEEDINGALGPROVIDERTOOL_H
#define ACTSTRK_IDEVICESEEDINGALGPROVIDERTOOL_H

#include "GaudiKernel/IAlgTool.h"
#include "GaudiKernel/EventContext.h"

#include <traccc/utils/algorithm.hpp>
#include <traccc/edm/measurement_collection.hpp>
#include <traccc/gbts_seeding/device/gbts_seeding_algorithm.hpp>
#include <traccc/seeding/device/triplet_seeding_algorithm.hpp>

namespace ActsTrk {

class IDeviceSeedingAlgProviderTool : virtual public IAlgTool {
public:
  DeclareInterfaceID(IDeviceSeedingAlgProviderTool, 1, 0);

  
  virtual std::pair<std::shared_ptr<const vecmem::copy>, std::shared_ptr<const traccc::device::triplet_seeding_algorithm>> getTripletSeedingAlgorithm(const EventContext& ctx, const traccc::seedfinder_config& seedfinder, const traccc::seedfilter_config& seedfilter) const = 0;
  virtual std::pair<std::shared_ptr<const vecmem::copy>, std::shared_ptr<const traccc::device::gbts_seeding_algorithm>> getGBTSAlgorithm(const EventContext& ctx, const traccc::gbts_seedfinder_config& gbts_config) const = 0;

};

} // namespace ActsTrk

#endif // ACTSTRK_IDEVICESEEDINGALGPROVIDERTOOL_H