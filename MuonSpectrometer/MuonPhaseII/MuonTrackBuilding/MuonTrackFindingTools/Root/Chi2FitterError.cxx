/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/


#include "MuonTrackFindingTools/Chi2FitterError.h"

#include <string>

namespace {

class GlobalChiSquareFitterErrorCategory : public std::error_category {
 public:
  // Return a short descriptive name for the category.
  const char* name() const noexcept final {
    return "GlobalChiSquareFitterError";
  }

  // Return what each enum means in text.
  std::string message(int c) const final {
    using MuonExperimental::GlobalChiSquareFitterError;

    switch (static_cast<GlobalChiSquareFitterError>(c)) {
      case GlobalChiSquareFitterError::AIsNotInvertible:
        return "Gx2f: aMatrix is not invertible.";
      case GlobalChiSquareFitterError::DidNotConverge:
        return "Gx2f: Did not converge in 'nUpdateMax' updates.";
      case GlobalChiSquareFitterError::NotEnoughMeasurements:
        return "Gx2f: Not enough measurements.";
      case GlobalChiSquareFitterError::UpdatePushedToNewVolume:
        return "Gx2f: Update pushed the parameters to a new volume.";
      case GlobalChiSquareFitterError::MomentumBelowCutOff:
        return "Gx2f: Momentum below the cut-off during a propagation.";
      case GlobalChiSquareFitterError::DirectionFlipLimitReached:
        return "Gx2f: Too many consecutive direction flips during a "
               "propagation.";
      default:
        return "unknown";
    }
  }
};

}  // namespace

std::error_code MuonExperimental::make_error_code(
    MuonExperimental::GlobalChiSquareFitterError e) {
  static const GlobalChiSquareFitterErrorCategory c;
  return {static_cast<int>(e), c};
}
