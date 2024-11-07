/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRACKRECONSTRUCTION_DUPLICATESEEDDETECTOR_H
#define ACTSTRACKRECONSTRUCTION_DUPLICATESEEDDETECTOR_H

#include "ActsGeometry/ATLASSourceLinkSurfaceAccessor.h"
#include "xAODMeasurementBase/UncalibratedMeasurementContainer.h"
#include "ActsEvent/SeedContainer.h"

#include <unordered_map>
#include <vector>

namespace ActsTrk::detail {
  
  // === DuplicateSeedDetector ===============================================
  // Identify duplicate seeds: seeds where all measurements were already located in a previously followed trajectory.
  class DuplicateSeedDetector {
  public:
    DuplicateSeedDetector(std::size_t numSeeds, bool enabled);
    DuplicateSeedDetector(const DuplicateSeedDetector &) = delete;
    DuplicateSeedDetector &operator=(const DuplicateSeedDetector &) = delete;
    DuplicateSeedDetector(DuplicateSeedDetector&&) noexcept = default;
    DuplicateSeedDetector& operator=(DuplicateSeedDetector&&) noexcept = default;
    ~DuplicateSeedDetector() = default;
    
    // add seeds from an associated measurements collection.
    // measurementOffset non-zero is only needed if measurements holds more than one collection (eg. kept for TrackStatePrinter).
    void addSeeds(std::size_t typeIndex, const ActsTrk::SeedContainer &seeds);
    void newTrajectory();
    void addMeasurement(const ActsTrk::ATLASUncalibSourceLink &sl);

    // For complete removal of duplicate seeds, assumes isDuplicate(iseed) is called for monotonically increasing iseed.
    bool isDuplicate(std::size_t typeIndex, std::size_t iseed);

    // Getters
    inline bool isEnabled() const;
    inline const std::unordered_multimap<const xAOD::UncalibratedMeasurement *, std::size_t>& seedIndexes() const;
    inline const std::vector<std::size_t>& nUsedMeasurements() const;
    inline const std::vector<std::size_t>& nSeedMeasurements() const;
    inline const std::vector<bool>& isDuplicateSeeds() const;
    inline const std::vector<std::size_t>& seedOffsets() const;
    inline std::size_t numSeeds() const;
    inline std::size_t nextSeeds() const;
    inline std::size_t foundSeeds() const;
    
  private:
    bool m_disabled {false};
    std::unordered_multimap<const xAOD::UncalibratedMeasurement *, std::size_t> m_seedIndexes {};
    std::vector<std::size_t> m_nUsedMeasurements {};
    std::vector<std::size_t> m_nSeedMeasurements {};
    std::vector<bool> m_isDuplicateSeeds {};
    std::vector<std::size_t> m_seedOffsets {};
    std::size_t m_numSeeds {0ul};  // count of number of seeds so-far added with addSeeds()
    std::size_t m_nextSeeds {0ul}; // index of next seed expected with isDuplicate()
    std::size_t m_foundSeeds {0ul};    // count of found seeds for this/last trajectory
  };

} // namespace ActsTrk::detail

#include "src/detail/DuplicateSeedDetector.icc"

#endif
