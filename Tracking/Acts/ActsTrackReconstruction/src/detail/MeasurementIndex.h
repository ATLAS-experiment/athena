/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRACKRECONSTRUCTION_MEASUREMENTINDEX_H
#define ACTSTRACKRECONSTRUCTION_MEASUREMENTINDEX_H

#include "xAODMeasurementBase/UncalibratedMeasurementContainer.h"

#include <utility>
#include <vector>

namespace ActsTrk::detail {

  // Helper class to keep track of measurement indices, used for shared hits and debug printing
  class MeasurementIndex {
  public:
    inline MeasurementIndex(std::size_t nMeasurementContainerMax = 0ul);  // nMeasurementContainerMax is only a hint
    MeasurementIndex(const MeasurementIndex &) = default;
    MeasurementIndex &operator=(const MeasurementIndex &) = default;
    MeasurementIndex(MeasurementIndex &&) noexcept = default;
    MeasurementIndex &operator=(MeasurementIndex &&) noexcept = default;
    ~MeasurementIndex() = default;

    // addMeasurement(s)/newMeasurementIndex can include previously added hits, as long as they are in the same owning container.
    inline bool addMeasurement(const xAOD::UncalibratedMeasurement &hit);
    inline void addMeasurements(const xAOD::UncalibratedMeasurementContainer &clusterContainer);
    inline std::size_t nMeasurements() const;

    inline std::size_t index(const xAOD::UncalibratedMeasurement &hit) const;
    inline std::pair<std::size_t,bool> newMeasurementIndex(const xAOD::UncalibratedMeasurement &hit);
    inline std::size_t size() const;

  private:
    using ContainerPtr = const SG::AuxVectorData *;
    std::vector<std::pair<ContainerPtr, std::size_t>> m_measurementContainerOffsets;
    std::size_t m_size{};
    std::size_t m_nMeasurements{};
    // MeasurementIndex is only ever used as a local stack variable, so doesn't need to be thread safe.
    // Is there a way to confirm this but still disable "non-thread-safe" warnings?
    // We could make index() non-const, but it is only needed for these cached values.
    mutable ContainerPtr m_lastContainer ATLAS_THREAD_SAFE{nullptr};
    mutable std::size_t m_lastContainerOffset ATLAS_THREAD_SAFE{};
    mutable std::size_t m_lastContainerSize ATLAS_THREAD_SAFE{};
  };

}  // namespace ActsTrk::detail

#include "src/detail/MeasurementIndex.icc"

#endif
