/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#pragma once

#include <system_error>
#include <type_traits>

namespace MuonExperimental {

/// Error codes for global chi-square fitter operations
/// @enum GlobalChiSquareFitterError
/// @ingroup errors
enum class GlobalChiSquareFitterError {
  // ensure all values are non-zero
  /// aMatrix is not invertible.
  AIsNotInvertible = 1,
  /// Did not converge in 'nUpdateMax' updates.
  DidNotConverge = 2,
  /// Not enough measurements.
  NotEnoughMeasurements = 3,
  /// Update pushed the parameters to a new volume.
  UpdatePushedToNewVolume = 4,
  /// Momentum below the cut-off during a propagation.
  MomentumBelowCutOff = 5,
  /// Too many consecutive direction flips during a propagation.
  DirectionFlipLimitReached = 6,

};

/// Converts error code to std::error_code
/// @param e Error code to convert
/// @return std::error_code for the given error
std::error_code make_error_code(
    MuonExperimental::GlobalChiSquareFitterError e);

}  // namespace MuonExperimental

namespace std {
// register with STL
template <>
struct is_error_code_enum<MuonExperimental::GlobalChiSquareFitterError>
    : std::true_type {};
}  // namespace std
