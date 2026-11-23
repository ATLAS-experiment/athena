/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSEVENT_SEEDMEASUREMENTSCONTAINER_H
#define ACTSEVENT_SEEDMEASUREMENTSCONTAINER_H 1

#include "xAODMeasurementBase/UncalibratedMeasurement.h"

#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <vector>

namespace ActsTrk {

  /// @brief Measurements of each seed of a seed collection.
  ///
  /// Used together with an index-aligned BoundTrackParametersContainer to
  /// provide seeds to the track finding without space points, e.g. when the
  /// seeds and their initial parameters were made on an accelerator. Entry i
  /// holds the measurements of seed i. The measurements are not owned, they
  /// must be held by the measurement containers given to the track finding.
  ///
  /// The track finding forces the track onto all measurements of the seed.
  /// A seed is flagged as duplicate once its duplicate detection measurements
  /// are found on a track. These are all measurements of the seed, unless a
  /// subset is given, e.g. the measurements of the three space points used
  /// for the parameter estimation of a seed with more space points.
  class SeedMeasurementsContainer {
  public:
    using Measurement = const xAOD::UncalibratedMeasurement*;
    using MeasurementRange = std::span<const Measurement>;

    SeedMeasurementsContainer() = default;

    void reserve(std::size_t nSeeds, std::size_t nMeasurements) {
      m_seeds.reserve(nSeeds);
      m_measurements.reserve(nMeasurements);
    }

    /// Add the next seed with the given measurements, all used for the duplicate detection
    void push_back(MeasurementRange measurements) {
      const std::uint32_t begin = m_measurements.size();
      m_measurements.insert(m_measurements.end(), measurements.begin(), measurements.end());
      const std::uint32_t size = static_cast<std::uint32_t>(measurements.size());
      m_seeds.push_back({begin, size, begin, size});
    }

    /// Add the next seed with the given measurements and the subset of them used for the duplicate detection
    void push_back(MeasurementRange measurements, MeasurementRange duplicateDetectionMeasurements) {
      const std::uint32_t begin = m_measurements.size();
      m_measurements.insert(m_measurements.end(), measurements.begin(), measurements.end());
      const std::uint32_t duplicateBegin = m_measurements.size();
      m_measurements.insert(m_measurements.end(), duplicateDetectionMeasurements.begin(), duplicateDetectionMeasurements.end());
      m_seeds.push_back({begin, static_cast<std::uint32_t>(measurements.size()),
                         duplicateBegin, static_cast<std::uint32_t>(duplicateDetectionMeasurements.size())});
    }

    /// Number of seeds
    std::size_t size() const noexcept { return m_seeds.size(); }
    bool empty() const noexcept { return m_seeds.empty(); }

    /// All measurements of seed iseed
    MeasurementRange measurements(std::size_t iseed) const noexcept {
      const Entry &entry = m_seeds[iseed];
      return MeasurementRange(m_measurements.data() + entry.begin, entry.size);
    }

    /// Measurements of seed iseed used to detect whether it is a duplicate
    MeasurementRange duplicateDetectionMeasurements(std::size_t iseed) const noexcept {
      const Entry &entry = m_seeds[iseed];
      return MeasurementRange(m_measurements.data() + entry.duplicateBegin, entry.duplicateSize);
    }

    MeasurementRange operator[](std::size_t iseed) const noexcept { return measurements(iseed); }

    MeasurementRange at(std::size_t iseed) const {
      if (!(iseed < size())) {
        throw std::out_of_range("SeedMeasurementsContainer index out of range");
      }
      return measurements(iseed);
    }

  private:
    struct Entry {
      std::uint32_t begin;
      std::uint32_t size;
      std::uint32_t duplicateBegin;
      std::uint32_t duplicateSize;
    };
    std::vector<Entry> m_seeds;
    std::vector<Measurement> m_measurements;
  };

}  // namespace ActsTrk

// Set up a CLID for the type:
#include "AthenaKernel/CLASS_DEF.h"
CLASS_DEF( ActsTrk::SeedMeasurementsContainer , 1145633899 , 1 )

#endif
