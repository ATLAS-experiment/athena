/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRACKRECONSTRUCTION_TRACKFINDINGMEASUREMENTS_H
#define ACTSTRACKRECONSTRUCTION_TRACKFINDINGMEASUREMENTS_H

#include "src/detail/AtlasUncalibSourceLinkAccessor.h"

namespace ActsTrk {
  struct DetectorElementToActsGeometryIdMap;
}

namespace ActsTrk::detail {

  // Helper class to convert and store MeasurementContainer specializations to MeasurementRangeList
  class TrackFindingMeasurements {
  public:
    TrackFindingMeasurements(std::size_t nMeasurementContainerMax);
    TrackFindingMeasurements(const TrackFindingMeasurements &) = default;
    TrackFindingMeasurements &operator=(const TrackFindingMeasurements &) = default;
    TrackFindingMeasurements(TrackFindingMeasurements &&) noexcept = default;
    TrackFindingMeasurements &operator=(TrackFindingMeasurements &&) noexcept = default;
    ~TrackFindingMeasurements() = default;

    void addMeasurements(std::size_t typeIndex,
                         const xAOD::UncalibratedMeasurementContainer &clusterContainer,
                         const DetectorElementToActsGeometryIdMap &detectorElementToGeoid);
    MeasurementRange markSurfaceInsensitive(const Acts::GeometryIdentifier &identifier);

    inline const ActsTrk::detail::MeasurementRangeList &measurementRanges() const;
    inline std::size_t nMeasurements() const;
    inline const std::vector<std::size_t> &measurementOffsets() const;

  private:
    std::vector<std::size_t> m_measurementOffsets;
    // ActsTrk::detail::MeasurementRangeList is an std::unordered_map;
    ActsTrk::detail::MeasurementRangeList m_measurementRanges{};
    std::size_t m_measurementsTotal{0ul};
  };

}  // namespace ActsTrk::detail

#include "src/detail/TrackFindingMeasurements.icc"

#endif
