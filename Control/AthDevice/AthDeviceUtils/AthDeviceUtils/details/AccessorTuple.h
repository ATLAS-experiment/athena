// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#ifndef ATHDEVICEUTILS_DETAILS_ACCESSORTUPLE_H
#define ATHDEVICEUTILS_DETAILS_ACCESSORTUPLE_H

// Local include(s).
#include "AthDeviceUtils/SoAColumn.h"

// VecMem include(s).
#include <vecmem/edm/schema.hpp>

// System include(s).
#include <tuple>

namespace AthDevice::Details {

/// Base type for the schema specific specialization of @c AccessorTuple
template <typename T>
struct AccessorTuple;

/// Helper utility constructing a tuple of accessors used for memory copies
template <typename... VARTYPES>
struct AccessorTuple<vecmem::edm::schema<VARTYPES...>> {

  /// Get a tuple of (const) accessors for the given column definitions
  ///
  /// @param columns The column definitions
  /// @return A tuple of (const) accessors for the given column definitions
  ///
  template <Concepts::SoAColumn... COLUMNS>
  static auto get_const(const std::tuple<COLUMNS...>& columns);

  /// Get a tuple of (non-const) accessors for the given column definitions
  ///
  /// @param columns The column definitions
  /// @return A tuple of (non-const) accessors for the given column definitions
  ///
  template <Concepts::SoAColumn... COLUMNS>
  static auto get_nc(const std::tuple<COLUMNS...>& columns);

};  // struct AccessorTuple

}  // namespace AthDevice::Details

// Include the implementation.
#include "AthDeviceUtils/impl/AccessorTuple.icc"

#endif  // ATHDEVICEUTILS_DETAILS_ACCESSORTUPLE_H
