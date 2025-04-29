/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file /GSFTsos.h
 * @begin         September 2023
 * @author        Christos Anastopoulos
 * @brief         Simplified TSos for internal GSF use
 */

#ifndef TrkGsfTsos_H
#define TrkGsfTsos_H

#include "TrkTrack/MultiComponentStateOnSurface.h"

struct GSFTsos {

  // Full state
  Trk::MultiComponentState multiComponentState{};
  // Collapsed to single Parameters state
  std::unique_ptr<Trk::TrackParameters> trackParameters{};
  // Measurement
  std::unique_ptr<Trk::MeasurementBase> measurementOnTrack{};
  // FitQuality
  Trk::FitQualityOnSurface fitQualityOnSurface{};
  // Type of TSOS
  std::bitset<Trk::TrackStateOnSurface::NumberOfTrackStateOnSurfaceTypes>
      typeFlags{};

  // default move, move assignment and dtor
  GSFTsos() = default;
  GSFTsos& operator=(GSFTsos&&) = default;
  GSFTsos(GSFTsos&&) = default;
  ~GSFTsos() = default;

  // implement copy and copy assignment
  GSFTsos(const GSFTsos& rhs) = delete;
  GSFTsos& operator=(const GSFTsos& rhs) = delete;

  // Full constructor with passing of type flags
  GSFTsos(
      const Trk::FitQualityOnSurface& inFitQualityOnSurface,
      std::unique_ptr<Trk::MeasurementBase> inMeasurementBase,
      std::unique_ptr<Trk::TrackParameters> inTrackParameters,
      Trk::MultiComponentState&& inMultiComponentState,
      const std::bitset<Trk::TrackStateOnSurface::NumberOfTrackStateOnSurfaceTypes>& inTypeFlags)
      : multiComponentState(std::move(inMultiComponentState)),
        trackParameters(std::move(inTrackParameters)),
        measurementOnTrack(std::move(inMeasurementBase)),
        fitQualityOnSurface(inFitQualityOnSurface),
        typeFlags(inTypeFlags) {}

  // constructor with automatic setting of type flags
  GSFTsos(const Trk::FitQualityOnSurface& inFitQualityOnSurface,
          std::unique_ptr<Trk::MeasurementBase> inMeasurementBase,
          std::unique_ptr<Trk::TrackParameters> inTrackParameters,
          Trk::MultiComponentState&& inMultiComponentState)
      : multiComponentState(std::move(inMultiComponentState)),
        trackParameters(std::move(inTrackParameters)),
        measurementOnTrack(std::move(inMeasurementBase)),
        fitQualityOnSurface(inFitQualityOnSurface) {
    if (measurementOnTrack) {
      typeFlags.set(Trk::TrackStateOnSurface::Measurement);
    }
    if (!multiComponentState.empty()) {
      typeFlags.set(Trk::TrackStateOnSurface::Parameter);
    }
    if (fitQualityOnSurface) {
      typeFlags.set(Trk::TrackStateOnSurface::FitQuality);
    }
  }
  // convert pass ownership to MTSOS
  std::unique_ptr<const Trk::MultiComponentStateOnSurface> convert(bool slim) {
    if (!trackParameters) {
      trackParameters = multiComponentState.front().params->uniqueClone();
    }
    if (slim) {
      multiComponentState.clear();
    }
    return std::make_unique<const Trk::MultiComponentStateOnSurface>(
        fitQualityOnSurface, std::move(measurementOnTrack),
        std::move(trackParameters), std::move(multiComponentState), nullptr,
        typeFlags);
  }
};

#endif
