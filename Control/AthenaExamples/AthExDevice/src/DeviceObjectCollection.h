// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#ifndef ATHEXDEVICE_DEVICECOLLECTION_H
#define ATHEXDEVICE_DEVICECOLLECTION_H

// Framework include(s).
#include "AthenaKernel/CLASS_DEF.h"

// VecMem include(s).
#include <vecmem/edm/container.hpp>

namespace AthExDevice {

/// Interface to the example SoA collection
template <typename BASE>
class DeviceObject : public BASE {

 public:
  /// @name Constructors
  /// @{

  /// Inherit the base class's constructor(s)
  using BASE::BASE;
  /// Use a default copy constructor
  DeviceObject(const DeviceObject& other) = default;
  /// Use a default move constructor
  DeviceObject(DeviceObject&& other) = default;

  /// @}

  /// @name Member accessor(s)
  /// @{

  auto& eta() { return BASE::template get<0>(); }
  const auto& eta() const { return BASE::template get<0>(); }

  auto& phi() { return BASE::template get<1>(); }
  const auto& phi() const { return BASE::template get<1>(); }

  auto& indices() { return BASE::template get<2>(); }
  const auto& indices() const { return BASE::template get<2>(); }

  /// @}

};  // class DeviceObject

/// VecMem based SoA container used for the example(s)
using DeviceObjectCollection =
    vecmem::edm::container<DeviceObject,
                           // eta
                           vecmem::edm::type::vector<float>,
                           // phi
                           vecmem::edm::type::vector<float>,
                           // indices
                           vecmem::edm::type::jagged_vector<unsigned int>>;

}  // namespace AthExDevice

// CLID definition(s).
CLASS_DEF(AthExDevice::DeviceObjectCollection::buffer, 1183318154, 1)

#endif  // ATHEXDEVICE_DEVICECOLLECTION_H
