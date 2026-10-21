// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#ifndef ATHDEVICEUTILS_DETAILS_ACCESSORTRAITS_H
#define ATHDEVICEUTILS_DETAILS_ACCESSORTRAITS_H

// Framework include(s).
#include "AthContainers/Accessor.h"
#include "AthContainers/ConstAccessor.h"

// VecMem include(s).
#include <vecmem/edm/schema.hpp>

// System include(s).
#include <type_traits>

namespace AthDevice::Details {

/// Base class for the specialized @c AccessorType variants
template <typename T>
struct AccessorType;

/// Specialization for a 1D vector type
template <typename T>
struct AccessorType<vecmem::edm::type::vector<T>> {
  using const_type = SG::ConstAccessor<std::remove_cvref_t<T>>;
  using nc_type = SG::Accessor<std::remove_cvref_t<T>>;
};

/// Specialization for a jagged vector type
template <typename T>
struct AccessorType<vecmem::edm::type::jagged_vector<T>> {
  using const_type = SG::ConstAccessor<std::vector<std::remove_cvref_t<T>>>;
  using nc_type = SG::Accessor<std::vector<std::remove_cvref_t<T>>>;
};

/// Convenience alias for the @c AccessorType type
template <typename T>
using AccessorType_t = typename AccessorType<T>::nc_type;
/// Convenience alias for the @c AccessorType type
template <typename T>
using ConstAccessorType_t = typename AccessorType<T>::const_type;

}  // namespace AthDevice::Details

#endif  // ATHDEVICEUTILS_DETAILS_ACCESSORTRAITS_H
