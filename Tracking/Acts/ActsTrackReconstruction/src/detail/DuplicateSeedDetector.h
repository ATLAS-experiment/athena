/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRACKRECONSTRUCTION_DUPLICATESEEDDETECTOR_H
#define ACTSTRACKRECONSTRUCTION_DUPLICATESEEDDETECTOR_H

#include "ActsGeometry/ATLASSourceLinkSurfaceAccessor.h"
#include "xAODMeasurementBase/UncalibratedMeasurementContainer.h"
#include "ActsEvent/SeedContainer.h"

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

    DuplicateSeedDetector(std::size_t numSeeds, bool enabled);
    DuplicateSeedDetector(const DuplicateSeedDetector &) = delete;
    DuplicateSeedDetector &operator=(const DuplicateSeedDetector &) = delete;
    DuplicateSeedDetector(DuplicateSeedDetector &&) noexcept = default;
    DuplicateSeedDetector &operator=(DuplicateSeedDetector &&) noexcept = default;
    ~DuplicateSeedDetector() = default;

    // add seeds from an associated measurements collection.
    void addSeeds(std::size_t typeIndex, const ActsTrk::SeedContainer &seeds, const MeasurementIndex &measurementIndex);
    inline void newTrajectory();
    inline void addMeasurement(const ActsTrk::ATLASUncalibSourceLink &sl, const MeasurementIndex &measurementIndex);

    // For complete removal of duplicate seeds, assumes isDuplicate(typeIndex,iseed) is called for monotonically increasing typeIndex,iseed.
    inline bool isDuplicate(std::size_t typeIndex, index_t iseed);

  private:
    friend struct DuplicateSeedDetectorTest;  // allow unit test access to internals

    bool m_disabled{false};
    std::vector<boost::container::small_vector<index_t, 4>> m_seedIndex;  // m_seedIndex[measurementIndex][usedBySeedNumber]
    std::vector<std::size_t> m_nUsedMeasurements;
    std::vector<std::size_t> m_nSeedMeasurements;
    std::vector<bool> m_isDuplicateSeed;
    std::vector<index_t> m_seedOffset;
    index_t m_numSeeds{0u};         // count of number of seeds so-far added with addSeeds()
    index_t m_nextSeed{0u};         // index of next seed expected with isDuplicate()
    std::size_t m_foundSeeds{0ul};  // count of found seeds for this/last trajectory
  };

}  // namespace ActsTrk::detail

#include "src/detail/DuplicateSeedDetector.icc"

#endif
