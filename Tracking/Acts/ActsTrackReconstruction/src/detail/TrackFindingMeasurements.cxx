/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "src/detail/TrackFindingMeasurements.h"

#include "ActsGeometry/DetectorElementToActsGeometryIdMap.h"
#include "ActsGeometry/SurfaceOfMeasurementUtil.h"

namespace ActsTrk::detail {

  TrackFindingMeasurements::TrackFindingMeasurements(std::size_t nMeasurementContainerMax)
      : m_measurementOffsets(nMeasurementContainerMax, 0ul) {
    m_containers.reserve(nMeasurementContainerMax);
  }

  void TrackFindingMeasurements::addMeasurements(std::size_t typeIndex,
                                                const xAOD::UncalibratedMeasurementContainer &clusterContainer,
                                                const DetectorElementToActsGeometryIdMap &detectorElementToGeoid,
                                                const MeasurementIndex *measurementIndex) {
    unsigned int typeIndex32 = static_cast<unsigned int>(typeIndex);
    if (typeIndex < m_measurementOffsets.size())
      m_measurementOffsets[typeIndex] = m_measurementsTotal;
    if (!(typeIndex < m_containers.size()))
      m_containers.resize(typeIndex + 1);
    m_containers[typeIndex] = &clusterContainer;
    if (measurementIndex)
      m_surfaceIndices.resize(measurementIndex->size());

    if (m_measurementRanges.empty()) {
      // try to reserve needed space,
      // this however will reserve more than necessary not just the space needed for the surfaces of
      // all the measurements that are going to be added (e.g. pixel+strips).
      m_measurementRanges.reserve(detectorElementToGeoid.size());
    }
    m_measurementRanges.setContainer(typeIndex32, &clusterContainer);

    xAOD::UncalibMeasType lastMeasurementType = xAOD::UncalibMeasType::Other;
    xAOD::DetectorIDHashType lastIdHash = std::numeric_limits<xAOD::DetectorIDHashType>::max();
    MeasurementRange *currentRange = nullptr;

    std::size_t n_elements = clusterContainer.size();
    unsigned int sl_idx = 0;
    Acts::GeometryIdentifier measurementSurfaceId{};
    for (; sl_idx < n_elements; ++sl_idx) {
      const auto *measurement = clusterContainer[sl_idx];
      if (measurement->identifierHash() != lastIdHash or
          measurement->type() != lastMeasurementType) {
        if (currentRange) {
          currentRange->updateEnd(typeIndex, sl_idx);
        }
        lastIdHash = measurement->identifierHash();
        lastMeasurementType = measurement->type();

        measurementSurfaceId = ActsTrk::getSurfaceGeometryIdOfMeasurement(detectorElementToGeoid, *measurement);

        currentRange = addMeasurementToRange(m_measurementRanges, typeIndex32, sl_idx, sl_idx, measurement, measurementSurfaceId);
      }
      if (measurementIndex) {
        // sl_idx may be different from measurement->index() if we aren't passed the owning container (eg. with cached mode).
        // So we save the mapping here. This is only needed by setMeasurementRangesForced(), so measurementIndex is optional.
        // We also save the measurementSurfaceId for quick access. Note that this may have been calculated on a previous iteration for the same id hash.
        std::size_t idx = measurementIndex->index(*measurement);
        if (idx < m_surfaceIndices.size())
          m_surfaceIndices.at(idx) = {measurementSurfaceId, typeIndex32, sl_idx};
      }
    }

    if (currentRange) {
      currentRange->updateEnd(typeIndex, sl_idx);
    }

    m_measurementsTotal += clusterContainer.size();
  }

  template <typename MeasurementRangeList_t>
  MeasurementRange *
  TrackFindingMeasurements::addMeasurementToRange(MeasurementRangeList_t &measurementRanges,
                                                  unsigned int typeIndex,
                                                  unsigned int sl_idx,
                                                  unsigned int sl_idx_end,
                                                  const xAOD::UncalibratedMeasurement *measurement,
                                                  Acts::GeometryIdentifier measurementSurfaceId) {
    if (measurementSurfaceId.value() == 0u) {
      std::stringstream msg;
      msg << "No Acts surface associated to measurement "
          << sl_idx << " type = " << static_cast<unsigned int>(measurement->type())
          << " idHash=" << measurement->identifierHash();
      throw std::domain_error(msg.str());
    }

    // start with en empty range which is updated later.
    auto ret = measurementRanges.insert(std::make_pair(measurementSurfaceId.value(),
                                                      MeasurementRange(typeIndex, sl_idx, sl_idx_end)));
    if (!ret.second) {
      std::stringstream msg;
      msg << "Measurement not clustered by identifierHash / geometryId. New measurement "
          << sl_idx << " with geo Id " << measurementSurfaceId
          << " type = " << static_cast<unsigned int>(measurement->type())
          << " idHash=" << measurement->identifierHash()
          << " but already recorded for this geo ID the range : [" << ret.first->second.containerIndex() << "]"
          << ret.first->second.elementBeginIndex()
          << " .. " << ret.first->second.elementEndIndex()
          << (ret.first->second.isConsistentRange() ? "" : " !Container index inconsistent or not in increasing order!");
      throw std::runtime_error(msg.str());
    }
    return &ret.first->second;
  }

  MeasurementRange TrackFindingMeasurements::markSurfaceInsensitive(const Acts::GeometryIdentifier &identifier) {
    auto ret = m_measurementRanges.insert(std::make_pair(identifier.value(),
                                                        MeasurementRange::noMeasurementExpected()));
    if (!ret.second) {
      return ret.first->second;
    } else {
      return MeasurementRange();
    }
  }

  MeasurementRangeListFlat
  TrackFindingMeasurements::setMeasurementRangesForced(const ActsTrk::Seed &seed,
                                                      const MeasurementIndex &measurementIndex) const {
    ActsTrk::detail::MeasurementRangeListFlat measurementRangesForced;
    measurementRangesForced.reserve(seed.sp().size());  // wrong for strip seeds, but just means an extra allocation in this rare case
    for (const xAOD::SpacePoint *sp : seed.sp()) {
      for (const xAOD::UncalibratedMeasurement *measurement : sp->measurements()) {
        std::size_t idx = measurementIndex.index(*measurement);
        if (!(idx < m_surfaceIndices.size())) {
          // The measurement was not found in the measurementIndex.
          // This could happen if no measurements in the same owning container were provided to addMeasurements().
          continue;
        }
        auto [measurementSurfaceId, typeIndex, sl_idx] = m_surfaceIndices.at(idx);
        if (measurementSurfaceId.value() == 0ul) {
          // This measurement was not provided to addMeasurements(), so we can't force the track onto it.
          // This happens in secondary passes, which are given a reduced set of measurements.
          // Note that the test uses measurementSurfaceId=0, since 0 is valid for the other members.
          continue;
        }
        assert(container(typeIndex)->at(sl_idx) == measurement);
        measurementRangesForced.setContainer(typeIndex, container(typeIndex));
        addMeasurementToRange(measurementRangesForced,
                              typeIndex,
                              sl_idx,
                              sl_idx + 1,
                              measurement,
                              measurementSurfaceId);
      }
    }
    return measurementRangesForced;
  }

}  // namespace ActsTrk::detail
