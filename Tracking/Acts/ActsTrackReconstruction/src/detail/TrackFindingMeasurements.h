/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRACKRECONSTRUCTION_TRACKFINDINGMEASUREMENTS_H
#define ACTSTRACKRECONSTRUCTION_TRACKFINDINGMEASUREMENTS_H

#include "ActsGeometry/SurfaceOfMeasurementUtil.h"
#include "ActsGeometry/DetectorElementToActsGeometryIdMap.h"

#include "src/detail/AtlasUncalibSourceLinkAccessor.h"

namespace ActsTrk::detail {

  // === TrackFindingMeasurements ============================================
  // Helper class to convert MeasurementContainer specializations to UncalibSourceLinkMultiset.
  class TrackFindingMeasurements {
  public:
    TrackFindingMeasurements(std::size_t nMeasurementContainerMax);
    TrackFindingMeasurements(const TrackFindingMeasurements &) = default;
    TrackFindingMeasurements& operator=(const TrackFindingMeasurements &) = default;
    TrackFindingMeasurements(TrackFindingMeasurements&&) noexcept = default;
    TrackFindingMeasurements& operator=(TrackFindingMeasurements&&) noexcept = default;
    ~TrackFindingMeasurements() = default;
    
    // NB. all addDetectorElements() must have been done before calling first addMeasurements().
    void addMeasurements(std::size_t typeIndex,
                         const xAOD::UncalibratedMeasurementContainer &clusterContainer,
                         const DetectorElementToActsGeometryIdMap &detectorElementToGeoid);

    std::vector<std::pair<const xAOD::UncalibratedMeasurementContainer *, std::size_t>> measurementContainerOffsets() const;


    inline std::size_t measurementOffset(std::size_t typeIndex) const;    
    inline const std::vector<std::size_t>& measurementOffsets() const;
    inline const ActsTrk::detail::MeasurementRangeList& measurementRanges() const;
    inline std::size_t nMeasurements() const;
    
  private:
    std::vector<std::size_t> m_measurementOffsets{};
    // ActsTrk::detail::MeasurementRangeList is an std::unordered_map;
    ActsTrk::detail::MeasurementRangeList m_measurementRanges{};
    std::size_t m_measurementsTotal{0ul};
  };

} // namespace ActsTrk::detail

#include "src/detail/TrackFindingMeasurements.icc"

#endif
