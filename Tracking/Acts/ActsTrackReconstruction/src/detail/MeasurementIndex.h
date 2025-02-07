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
    inline MeasurementIndex(std::size_t nMeasurementContainerMax);
    MeasurementIndex(const MeasurementIndex &) = default;
    MeasurementIndex &operator=(const MeasurementIndex &) = default;
    MeasurementIndex(MeasurementIndex &&) noexcept = default;
    MeasurementIndex &operator=(MeasurementIndex &&) noexcept = default;
    ~MeasurementIndex() = default;

    inline void addMeasurements(const xAOD::UncalibratedMeasurementContainer &clusterContainer);
    inline std::size_t nMeasurements() const;

    inline std::size_t index(const xAOD::UncalibratedMeasurement &hit);
    inline std::size_t size() const;

  private:
    using ContainerPtr = const SG::AuxVectorData *;
    std::vector<std::pair<ContainerPtr, std::size_t>> m_measurementContainerOffsets;
    std::size_t m_size{};
    std::size_t m_nMeasurements{};
    ContainerPtr m_lastContainer{nullptr};
    std::size_t m_lastContainerOffset{};
    std::size_t m_lastContainerSize{};
  };

}  // namespace ActsTrk::detail

#include "src/detail/MeasurementIndex.icc"

#endif
