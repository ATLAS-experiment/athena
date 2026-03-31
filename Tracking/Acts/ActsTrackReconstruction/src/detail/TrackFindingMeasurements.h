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

namespace InDet {
   class SiDetectorElementStatus;
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
    std::unique_ptr<MeasurementRangeListFlat> createMeasurementRangesForced(const ActsTrk::Seed &seed,
                                                                            const MeasurementIndex &measurementIndex) const;
     //    MeasurementRange markSurfaceInsensitive(const Acts::GeometryIdentifier &identifier);
    void setDetectorElementStatus(const std::array< const InDet::SiDetectorElementStatus *,
                                                   static_cast<unsigned int>(ActsTrk::DetectorType::UnDefined)+1u> &det_el_status_per_arr);
     

    inline const MeasurementRangeList &measurementRanges() const;
     
  private:
    struct MeasurementSurfaceIndex {
      Acts::GeometryIdentifier measurementSurfaceId;
      unsigned int typeIndex;
      unsigned int sl_idx;
    };

    MeasurementRangeList m_measurementRanges{};
    std::array< const InDet::SiDetectorElementStatus *,
                static_cast<unsigned int>(ActsTrk::DetectorType::UnDefined)+1u> m_detectorElementStatusPerDetectorType;
  };

}  // namespace ActsTrk::detail

#include "src/detail/TrackFindingMeasurements.icc"

#endif
