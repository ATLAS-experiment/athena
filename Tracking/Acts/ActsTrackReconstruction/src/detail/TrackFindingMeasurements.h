/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRACKRECONSTRUCTION_TRACKFINDINGMEASUREMENTS_H
#define ACTSTRACKRECONSTRUCTION_TRACKFINDINGMEASUREMENTS_H

#include <vector>

#include "ActsEvent/SeedContainer.h"
#include "src/detail/AtlasUncalibSourceLinkAccessor.h"
#include "src/detail/MeasurementIndex.h"

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
                        const DetectorElementToActsGeometryIdMap &detectorElementToGeoid,
                        const MeasurementIndex *measurementIndex = nullptr);
    MeasurementRangeListFlat setMeasurementRangesForced(const ActsTrk::Seed &seed,
                                                        const MeasurementIndex &measurementIndex) const;
    MeasurementRange markSurfaceInsensitive(const Acts::GeometryIdentifier &identifier);

    inline const MeasurementRangeList &measurementRanges() const;
    inline std::size_t nMeasurements() const;
    inline const std::vector<std::size_t> &measurementOffsets() const;
    inline const xAOD::UncalibratedMeasurementContainer *container(std::size_t typeIndex) const;

  private:
    struct MeasurementSurfaceIndex {
      Acts::GeometryIdentifier measurementSurfaceId;
      unsigned int typeIndex;
      unsigned int sl_idx;
    };

    template <typename MeasurementRangeList_t>
    static MeasurementRange *addMeasurementToRange(MeasurementRangeList_t &measurementRanges,
                                                  unsigned int typeIndex,
                                                  unsigned int sl_idx,
                                                  unsigned int sl_idx_end,
                                                  const xAOD::UncalibratedMeasurement *measurement,
                                                  Acts::GeometryIdentifier measurementSurfaceId);

    std::vector<std::size_t> m_measurementOffsets;
    // ActsTrk::detail::MeasurementRangeList is an std::unordered_map;
    MeasurementRangeList m_measurementRanges{};
    std::vector<const xAOD::UncalibratedMeasurementContainer *> m_containers{};
    std::vector<MeasurementSurfaceIndex> m_surfaceIndices;

    std::size_t m_measurementsTotal{0ul};
  };

}  // namespace ActsTrk::detail

#include "src/detail/TrackFindingMeasurements.icc"

#endif
