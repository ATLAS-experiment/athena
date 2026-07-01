/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#pragma once

#include "GaudiKernel/IService.h"
#include "Identifier/Identifier.h"
#include "traccc/geometry/detector_design_description.hpp"
#include "traccc/geometry/detector_conditions_description.hpp"
#include <unordered_map>
#include <cstdint>

namespace ActsTrk {

class IActsDeviceDetectorDescriptionProviderSvc : virtual public IService {
public:
    DeclareInterfaceID(IActsDeviceDetectorDescriptionProviderSvc, 1, 0);

    virtual ~IActsDeviceDetectorDescriptionProviderSvc() = default;

    virtual const std::unordered_map<uint64_t, Identifier>&
        detrayToAthenaMap() const = 0;

    virtual const std::unordered_map<Identifier, uint64_t>&
        athenaToDetrayMap() const = 0;
    virtual const traccc::detector_design_description::buffer&
      designDescriptionBuffer() const = 0;

    virtual const traccc::detector_conditions_description::buffer&
      conditionsDescriptionBuffer() const = 0;

    virtual const traccc::detector_conditions_description::host& hostConditions() const = 0;
};

} // namespace ActsTrk