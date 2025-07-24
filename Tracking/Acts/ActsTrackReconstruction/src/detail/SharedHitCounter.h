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

    using ReturnSharedAndBad = std::pair<std::size_t, std::size_t>;

    template <Acts::TrackContainerFrontend track_container_t>
    inline auto computeSharedHitsDynamic(typename track_container_t::TrackProxy &track,
                                         track_container_t &tracks,
                                         MeasurementIndex &measurementIndex,
                                         bool removeSharedHits = false)
        -> ReturnSharedAndBad;

    template <Acts::TrackContainerFrontend track_container_t>
    inline auto computeSharedHits(typename track_container_t::TrackProxy &track,
                                  track_container_t &tracks,
                                  const MeasurementIndex &measurementIndex,
                                  bool removeSharedHits = false)
        -> ReturnSharedAndBad;

    template <Acts::TrackContainerFrontend track_container_t, typename IndexFun>
    inline auto computeSharedHits(typename track_container_t::TrackProxy &track,
                                  track_container_t &tracks,
                                  std::size_t indexSize,
                                  IndexFun &&indexFun,
                                  bool removeSharedHits = false)
        -> ReturnSharedAndBad;

  private:
  struct TrackStateIndex {
      std::size_t trackIndex;
      std::size_t stateIndex;
    };
    static constexpr TrackStateIndex s_noTrackState{std::numeric_limits<std::size_t>::max(), std::numeric_limits<std::size_t>::max()};
    std::vector<TrackStateIndex> m_firstTrackStateOnTheHit;
  };

}  // namespace ActsTrk::detail

#include "src/detail/SharedHitCounter.icc"

#endif
