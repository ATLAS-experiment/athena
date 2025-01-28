/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRACKRECONSTRUCTION_SHAREDHITCOUNTER_H
#define ACTSTRACKRECONSTRUCTION_SHAREDHITCOUNTER_H

#include "src/detail/AtlasUncalibSourceLinkAccessor.h"
#include "src/detail/Definitions.h"

#include <utility>
#include <vector>

namespace ActsTrk {
 class MutableTrackContainer;
}

namespace ActsTrk::detail {

  // Helper class to keep track of measurement indices, used for shared hits and debug printing
  class SharedHitCounter {
  public:
    inline SharedHitCounter(std::size_t nMeasurementContainerMax);
    SharedHitCounter(const SharedHitCounter &) = default;
    SharedHitCounter &operator=(const SharedHitCounter &) = default;
    SharedHitCounter(SharedHitCounter &&) noexcept = default;
    SharedHitCounter &operator=(SharedHitCounter &&) noexcept = default;
    ~SharedHitCounter() = default;

    inline void addMeasurements(std::size_t typeIndex, const xAOD::UncalibratedMeasurementContainer &clusterContainer);
    inline std::pair<std::size_t, std::size_t> computeSharedHits(RecoTrackContainerProxy &track, ActsTrk::MutableTrackContainer &tracks);

    inline std::size_t measurementIndex(const xAOD::UncalibratedMeasurement &hit) const;
    inline std::size_t measurementIndexSize() const;

    inline const std::vector<std::size_t> &measurementOffsets() const;
    inline std::size_t nMeasurements() const;

  private:
    struct TrackStateIndex {
      std::size_t trackIndex;
      std::size_t stateIndex;
    };
    std::vector<std::size_t> m_measurementOffsets;
    std::vector<std::pair<const SG::AuxVectorData *, std::size_t>> m_measurementContainerOffsets;
    std::size_t m_measurementIndexSize{0ul};
    std::size_t m_measurementsTotal{0ul};
    std::vector<TrackStateIndex> m_firstTrackStateOnTheHit;
  };

}  // namespace ActsTrk::detail

#include "src/detail/SharedHitCounter.icc"

#endif
