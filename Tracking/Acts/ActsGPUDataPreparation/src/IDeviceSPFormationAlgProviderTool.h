/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRK_IDEVICESPFORMATIONALGPROVIDERTOOL_H
#define ACTSTRK_IDEVICESPFORMATIONALGPROVIDERTOOL_H

#include "GaudiKernel/IAlgTool.h"
#include "GaudiKernel/EventContext.h"

#include <traccc/utils/algorithm.hpp>
#include <traccc/edm/measurement_collection.hpp>
#include <traccc/seeding/device/silicon_pixel_spacepoint_formation_algorithm.hpp>

namespace ActsTrk {

class IDeviceSPFormationAlgProviderTool : virtual public IAlgTool {
public:
  DeclareInterfaceID(IDeviceSPFormationAlgProviderTool, 1, 0);

   virtual std::pair<std::shared_ptr<const vecmem::copy>, std::shared_ptr<const traccc::device::silicon_pixel_spacepoint_formation_algorithm>> getPixelSPFormationAlgorithm(const EventContext& ctx) const = 0;

};

} // namespace ActsTrk

#endif // ACTSTRK_IDEVICESPFORMATIONALGPROVIDERTOOL_H