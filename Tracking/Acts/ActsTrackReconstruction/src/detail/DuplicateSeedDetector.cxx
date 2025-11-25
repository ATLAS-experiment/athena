/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "src/detail/DuplicateSeedDetector.h"

#include "src/detail/MeasurementIndex.h"
#include <stdexcept>
#include <array>

namespace ActsTrk::detail {

  DuplicateSeedDetector::DuplicateSeedDetector(std::size_t numSeeds,
					       unsigned int measOffset,
                                               bool enabled)
      : m_disabled(!enabled),
	m_measOffset(measOffset),
        m_nUsedMeasurements(enabled ? numSeeds : 0ul, 0u),
        m_nSeedMeasurements(enabled ? numSeeds : 0ul, 0u),
        m_isDuplicateSeed(enabled ? numSeeds : 0ul, false) {
    if (m_disabled)
      return;
    m_seedOffset.reserve(2ul);
  }

  void DuplicateSeedDetector::addSeeds(std::size_t typeIndex,
                                       const ActsTrk::SeedContainer &seeds,
                                       const MeasurementIndex& measurementIndex) {
    addSeeds(typeIndex, seeds, measurementIndex,
             [](std::size_t) -> std::array<std::size_t, 3> { return {0, 1, 2}; },
             [](const ActsTrk::Seed&) -> bool { return false; });
  }

  void DuplicateSeedDetector::addSeeds(std::size_t typeIndex,
                                       const ActsTrk::SeedContainer &seeds,
                                       const MeasurementIndex& measurementIndex,
                                       SpacePointIndicesFun_t spacePointIndicesFun,
                                       UseTopSpFun_t useTopSpFun) {
    if (m_disabled)
      return;
    if (!(typeIndex < m_seedOffset.size()))
      m_seedOffset.resize(typeIndex + 1);
    m_seedOffset[typeIndex] = m_numSeeds;
    m_seedIndex.resize(measurementIndex.size());  // will resize for each seed container, but always with the same space

    for (const ActsTrk::Seed seed : seeds) {
      std::size_t nSP = 0;
      bool useTopSp = useTopSpFun(seed);
      const auto& sps = seed.sp();
      for (std::size_t isp : spacePointIndicesFun(sps.size())) {
        const xAOD::SpacePoint *sp = sps.at(useTopSp ? sps.size() - isp - 1 : isp);
        const std::vector<const xAOD::UncalibratedMeasurement *> &els = sp->measurements();
        for (const xAOD::UncalibratedMeasurement *meas : els) {
          std::size_t hitIndex = measurementIndex.index(*meas);
          if (!(hitIndex < m_seedIndex.size())) {
            // std::cout << "ERROR hit index " << hitIndex << " past end of " << m_seedIndex.size() << " hit indices\n";
            continue;
          }
          m_seedIndex[hitIndex].push_back(m_numSeeds);
          ++m_nSeedMeasurements[m_numSeeds];
        }
        ++nSP;
        if (nSP >= 3) break;
      }
      ++m_numSeeds;
    }
  }

}  // namespace ActsTrk::detail
