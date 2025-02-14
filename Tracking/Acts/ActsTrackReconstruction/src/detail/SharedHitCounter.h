/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRACKRECONSTRUCTION_SHAREDHITCOUNTER_H
#define ACTSTRACKRECONSTRUCTION_SHAREDHITCOUNTER_H

#include "src/detail/MeasurementIndex.h"

#include "Acts/EventData/TrackContainerFrontendConcept.hpp"

#include <utility>
#include <vector>

namespace ActsTrk {
 struct MutableTrackContainer;
}

namespace ActsTrk::detail {

  // Helper class to keep track of measurement indices, used for shared hits and debug printing
  class SharedHitCounter {
  public:
    SharedHitCounter() = default;
    SharedHitCounter(const SharedHitCounter &) = default;
    SharedHitCounter &operator=(const SharedHitCounter &) = default;
    SharedHitCounter(SharedHitCounter &&) noexcept = default;
    SharedHitCounter &operator=(SharedHitCounter &&) noexcept = default;
    ~SharedHitCounter() = default;

    template <Acts::TrackContainerFrontend track_container_t>
    inline std::pair<std::size_t, std::size_t> computeSharedHits(typename track_container_t::TrackProxy &track, track_container_t &tracks, const MeasurementIndex& measurementIndex);

  private:
    struct TrackStateIndex {
      std::size_t trackIndex;
      std::size_t stateIndex;
    };
    std::vector<TrackStateIndex> m_firstTrackStateOnTheHit;
  };

}  // namespace ActsTrk::detail

#include "src/detail/SharedHitCounter.icc"

#endif
