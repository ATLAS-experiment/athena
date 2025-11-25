/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRACKRECONSTRUCTION_DUPLICATESEEDDETECTOR_H
#define ACTSTRACKRECONSTRUCTION_DUPLICATESEEDDETECTOR_H

#include "xAODMeasurementBase/UncalibratedMeasurementContainer.h"
#include "ActsEvent/SeedContainer.h"
#include "ActsGeometry/ATLASSourceLink.h"

#include <unordered_map>
#include <vector>
#include <boost/container/small_vector.hpp>

namespace ActsTrk::detail {
  class MeasurementIndex;

  // === DuplicateSeedDetector ===============================================
  // Identify duplicate seeds: seeds where all measurements were already located in a previously followed trajectory.
  class DuplicateSeedDetector {
  public:
    using index_t = unsigned int;
    using SpacePointIndicesFun_t = std::function<std::array<std::size_t, 3>(std::size_t)>; // copied from ITrackParamsEstimationTool
    using UseTopSpFun_t = std::function<bool(const ActsTrk::Seed&)>;

    DuplicateSeedDetector(std::size_t numSeeds, index_t measOffset, bool enabled);
    DuplicateSeedDetector(const DuplicateSeedDetector &) = delete;
    DuplicateSeedDetector &operator=(const DuplicateSeedDetector &) = delete;
    DuplicateSeedDetector(DuplicateSeedDetector &&) noexcept = default;
    DuplicateSeedDetector &operator=(DuplicateSeedDetector &&) noexcept = default;
    ~DuplicateSeedDetector() = default;

    // add seeds from an associated measurements collection.
    void addSeeds(std::size_t typeIndex, const ActsTrk::SeedContainer &seeds, const MeasurementIndex &measurementIndex);
    void addSeeds(std::size_t typeIndex, const ActsTrk::SeedContainer &seeds, const MeasurementIndex &measurementIndex,
                  SpacePointIndicesFun_t spacePointIndicesFun, UseTopSpFun_t useTopSpFun);
    inline void newTrajectory();
    inline void addMeasurement(const ActsTrk::ATLASUncalibSourceLink &sl, const MeasurementIndex &measurementIndex);

    // For complete removal of duplicate seeds, assumes isDuplicate(typeIndex,iseed) is called for monotonically increasing typeIndex,iseed.
    inline bool isDuplicate(std::size_t typeIndex, index_t iseed);

  private:
    friend struct DuplicateSeedDetectorTest;  // allow unit test access to internals
    
    bool m_disabled{false};
    index_t m_measOffset{0u}; // if a seed has N hits, only N - m_measOffset are needed to mark it as duplicate
    std::vector<boost::container::small_vector<index_t, 2>> m_seedIndex;  // m_seedIndex[measurementIndex][usedBySeedNumber]
    std::vector<index_t> m_nUsedMeasurements;
    std::vector<index_t> m_nSeedMeasurements;
    std::vector<bool> m_isDuplicateSeed;
    std::vector<index_t> m_seedOffset;
    index_t m_numSeeds{0u};         // count of number of seeds so-far added with addSeeds()
    std::size_t m_nextSeed{0ul};    // index of next seed expected with isDuplicate()
    index_t m_foundSeeds{0u};       // count of found seeds for this/last trajectory
    
  };

}  // namespace ActsTrk::detail

#include "src/detail/DuplicateSeedDetector.icc"

#endif
