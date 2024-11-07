/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "src/detail/DuplicateSeedDetector.h"

namespace ActsTrk::detail {

  DuplicateSeedDetector::DuplicateSeedDetector(std::size_t numSeeds,
					       bool enabled)
    : m_disabled(!enabled),
      m_nUsedMeasurements(enabled ? numSeeds : 0u, 0u),
      m_nSeedMeasurements(enabled ? numSeeds : 0u, 0u),
      m_isDuplicateSeeds(enabled ? numSeeds : 0u, false)
  {
    if (m_disabled)
      return;
    m_seedIndexes.reserve(6 * numSeeds); // 6 hits/seed for strips (3 for pixels)
    m_seedOffsets.reserve(2);
  }
  
  void DuplicateSeedDetector::addSeeds(std::size_t typeIndex,
				       const ActsTrk::SeedContainer &seeds)
  {
    if (m_disabled) return;
    if (!(typeIndex < m_seedOffsets.size()))
      m_seedOffsets.resize(typeIndex + 1);
    m_seedOffsets[typeIndex] = m_numSeeds;
    
    for (const ActsTrk::Seed *seed : seeds) {
      if (!seed) continue;

      for (const xAOD::SpacePoint *sp : seed->sp()) {
	const std::vector< const xAOD::UncalibratedMeasurement* > &els = sp->measurements();
	for (const xAOD::UncalibratedMeasurement *meas : els) {
	  m_seedIndexes.insert({meas, m_numSeeds});
	  ++m_nSeedMeasurements[m_numSeeds];
	}
      }
      ++m_numSeeds;
    }
  }
  
  void DuplicateSeedDetector::newTrajectory() {
    if (m_disabled or m_foundSeeds == 0 or m_nextSeeds == m_nUsedMeasurements.size())
      return;
    
    auto beg = m_nUsedMeasurements.begin();
    if (m_nextSeeds < m_nUsedMeasurements.size()) {
      std::advance(beg, m_nextSeeds);
    }
    
    std::fill(beg, m_nUsedMeasurements.end(), 0ul);
  }
  
  void DuplicateSeedDetector::addMeasurement(const ActsTrk::ATLASUncalibSourceLink &sl) {
    if (m_disabled or m_nextSeeds == m_nUsedMeasurements.size()) return;

    for (auto [iiseed, eiseed] = m_seedIndexes.equal_range(&(ActsTrk::getUncalibratedMeasurement(sl)));
	 iiseed != eiseed; ++iiseed) {
	std::size_t iseed = iiseed->second;
        assert(iseed < m_nUsedMeasurements.size());
	
        if (iseed < m_nextSeeds or m_isDuplicateSeeds[iseed]) continue;

        if (++m_nUsedMeasurements[iseed] >= m_nSeedMeasurements[iseed]) {
	  assert(m_nUsedMeasurements[iseed] == m_nSeedMeasurements[iseed]); // shouldn't ever find more
	  m_isDuplicateSeeds[iseed] = true;
	}
        ++m_foundSeeds;
    }
  }
  
  // For complete removal of duplicate seeds, assumes isDuplicate(iseed) is called for monotonically increasing iseed.
  bool DuplicateSeedDetector::isDuplicate(std::size_t typeIndex,
					  std::size_t iseed) {
    if (m_disabled) return false;

    if (typeIndex < m_seedOffsets.size()) {
      iseed += m_seedOffsets[typeIndex];
    }
    
    assert(iseed < m_isDuplicateSeeds.size());
    // If iseed not increasing, we will miss some duplicate seeds, but won't exclude needed seeds.
    if (iseed >= m_nextSeeds) {
      m_nextSeeds = iseed + 1;
    }
    
    return m_isDuplicateSeeds[iseed];
  }

} // namespace ActsTrk::detail
