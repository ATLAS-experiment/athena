/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRACKRECONSTRUCTION_DEFINITIONS_H
#define ACTSTRACKRECONSTRUCTION_DEFINITIONS_H

// ACTS
#include "Acts/EventData/VectorTrackContainer.hpp"
#include "Acts/EventData/TrackContainer.hpp"
#include "Acts/EventData/TrackProxy.hpp"
#include "Acts/Propagator/SympyStepper.hpp"
#include "Acts/Propagator/Navigator.hpp"
#include "Acts/Propagator/Propagator.hpp"
#include "Acts/TrackFinding/MeasurementSelector.hpp"
#include "Acts/TrackFinding/CombinatorialKalmanFilter.hpp"
#include "Acts/TrackFinding/TrackSelector.hpp"

namespace ActsTrk::detail {
  
  // container used during the reconstructions
  using RecoTrackContainer = Acts::TrackContainer<Acts::VectorTrackContainer,
                                                  Acts::VectorMultiTrajectory>;
  using RecoTrackContainerProxy = RecoTrackContainer::TrackProxy;
  using RecoTrackStateContainer = Acts::VectorMultiTrajectory;
  using RecoTrackStateContainerProxy = RecoTrackStateContainer::TrackStateProxy;
  using RecoConstTrackStateContainerProxy = RecoTrackStateContainer::ConstTrackStateProxy;

  /// Adapted from Acts Examples/Algorithms/TrackFinding/src/TrackFindingAlgorithmFunction.cpp

  using Stepper = Acts::SympyStepper;
  using Navigator = Acts::Navigator;
  using Propagator = Acts::Propagator<Stepper, Navigator>;
  using CKF = Acts::CombinatorialKalmanFilter<Propagator, RecoTrackContainer>;
  using Extrapolator = Propagator;

  // Small holder class to keep CKF and related objects.
  // Keep a unique_ptr<CKF_pimpl> in TrackFindingAlg, so we don't have to expose the
  // Acts class definitions in TrackFindingAlg.h.
  // ActsTrk::TrackFindingAlg::CKF_pimpl inherits from CKF_config to prevent -Wsubobject-linkage warning.
  struct CKF_config {
    // Extrapolator
    Extrapolator extrapolator;
    // CKF algorithm
    CKF ckf;
    // CKF configuration
    Acts::MeasurementSelector measurementSelector;
    Acts::CombinatorialKalmanFilterExtensions<RecoTrackContainer> ckfExtensions;
    // Track selection
    Acts::TrackSelector trackSelector;
  };
  
} // namespace ActsTrk::detail

#endif
