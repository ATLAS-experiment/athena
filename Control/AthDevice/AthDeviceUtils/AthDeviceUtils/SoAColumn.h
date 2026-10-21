// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#ifndef ATHDEVICEUTILS_SOACOLUMN_H
#define ATHDEVICEUTILS_SOACOLUMN_H

// System include(s).
#include <concepts>
#include <cstddef>
#include <string_view>

namespace AthDevice {

/// Description of a single SoA column
template <std::size_t INDEX>
struct SoAColumn {
  /// Name of the auxiliary variable in the xAOD ontainer
  std::string_view name;
  /// Index of the column in the VecMem SoA container
  static constexpr std::size_t index = INDEX;
};

namespace Concepts {

/// Concept for an SoA column
template <typename T>
concept SoAColumn = requires {
  { T::name } -> std::convertible_to<std::string_view>;
  { T::index } -> std::convertible_to<std::size_t>;
};

}  // namespace Concepts

}  // namespace AthDevice

// Check that the type and concept are correctly defined.
static_assert(AthDevice::Concepts::SoAColumn<AthDevice::SoAColumn<0>>,
              "Implementation error in AthDevice::Concepts::SoAColumn.");

#endif  // ATHDEVICEUTILS_SOACOLUMN_H
