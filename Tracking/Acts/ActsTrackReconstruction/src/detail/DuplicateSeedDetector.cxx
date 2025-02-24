/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "src/detail/DuplicateSeedDetector.h"

#include "src/detail/MeasurementIndex.h"

namespace ActsTrk::detail {

  DuplicateSeedDetector::DuplicateSeedDetector(std::size_t numSeeds,
                                               bool enabled)
      : m_disabled(!enabled),
        m_nUsedMeasurements(enabled ? numSeeds : 0ul, 0ul),
        m_nSeedMeasurements(enabled ? numSeeds : 0ul, 0ul),
        m_isDuplicateSeed(enabled ? numSeeds : 0ul, false) {
    if (m_disabled)
      return;
    m_seedOffset.reserve(2ul);
  }

  void DuplicateSeedDetector::addSeeds(std::size_t typeIndex,
                                       const ActsTrk::SeedContainer &seeds,
                                       const MeasurementIndex& measurementIndex) {
    if (m_disabled)
      return;
    if (!(typeIndex < m_seedOffset.size()))
      m_seedOffset.resize(typeIndex + 1);
    m_seedOffset[typeIndex] = m_numSeeds;
    m_seedIndex.resize(measurementIndex.size());  // will resize for each seed container, but always with the same space

    for (const ActsTrk::Seed *seed : seeds) {
      if (!seed)
        continue;

      for (const xAOD::SpacePoint *sp : seed->sp()) {
        const std::vector<const xAOD::UncalibratedMeasurement *> &els = sp->measurements();
        for (const xAOD::UncalibratedMeasurement *meas : els) {
          std::size_t hitIndex = measurementIndex.index(*meas);
          if (!(hitIndex < m_seedIndex.size())) {
            std::cout << "ERROR hit index " << hitIndex << " past end of " << measurementIndex.size() << " hit indices\n";
            continue;
          }
          m_seedIndex[hitIndex].push_back(m_numSeeds);
          ++m_nSeedMeasurements[m_numSeeds];
        }
      }
      ++m_numSeeds;
    }
  }

}  // namespace ActsTrk::detail
