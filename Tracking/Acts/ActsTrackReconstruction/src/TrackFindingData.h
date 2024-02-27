/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSTRACKRECONSTRUCTION_TRACKFINDINGDATA_H
#define ACTSTRACKRECONSTRUCTION_TRACKFINDINGDATA_H 1

// ACTS
#include "Acts/Definitions/Common.hpp"
#include "Acts/Definitions/Algebra.hpp"
#include "Acts/Propagator/EigenStepper.hpp"
#include "Acts/Propagator/Navigator.hpp"
#include "Acts/Propagator/Propagator.hpp"
#include "Acts/TrackFitting/GainMatrixSmoother.hpp"
#include "Acts/TrackFitting/GainMatrixUpdater.hpp"
#include "Acts/TrackFinding/MeasurementSelector.hpp"
#include "Acts/TrackFinding/CombinatorialKalmanFilter.hpp"
#include "Acts/TrackFinding/TrackSelector.hpp"
#include "InDetReadoutGeometry/SiDetectorElement.h"

#include "ActsGeometry/ATLASSourceLink.h"
#include "ActsEventCnv/IActsToTrkConverterTool.h"
#include "src/ITrackStatePrinter.h"

#include <boost/container/flat_set.hpp>
#include <boost/container/flat_map.hpp>
#include <utility>
#include <vector>
#include <variant>

namespace
{
  /// =========================================================================
  /// Include all sorts of stuff needed to interface with the Acts Core classes.
  /// This is only required by code in TrackFindingAlg.cxx, so we keep it in the anonymous namespace.
  /// =========================================================================

  /// Borrowed from Athena Tracking/Acts/ActsTrkTools/ActsTrkFittingTools/src/ActsKalmanFitter.ipp
  /// We could also access them directly from there, but that would pull inline a lot of other stuff we
  /// don't need.

  static Acts::Result<void>
  gainMatrixUpdate(const Acts::GeometryContext &gctx,
                   typename ActsTrk::MutableTrackStateBackend::TrackStateProxy trackState,
                   Acts::Direction direction,
                   const Acts::Logger &logger)
  {
    Acts::GainMatrixUpdater updater;
    return updater.template operator()<ActsTrk::MutableTrackStateBackend>(gctx, trackState, direction, logger);
  }

  static Acts::Result<void>
  gainMatrixSmoother(const Acts::GeometryContext &gctx,
                     ActsTrk::MutableTrackStateBackend &trajectory,
                     size_t entryIndex,
                     const Acts::Logger &logger)
  {
    Acts::GainMatrixSmoother smoother;
    return smoother.template operator()<ActsTrk::MutableTrackStateBackend>(gctx, trajectory, entryIndex, logger);
  }

  // Helper class to describe ranges of measurements
  // the range provides the measurement collection index and  element index range (begin, end)
  struct MeasurementRange : public std::pair<unsigned int, unsigned int>
  {
    MeasurementRange() : std::pair<unsigned int, unsigned int>(std::numeric_limits<unsigned int>::max(), std::numeric_limits<unsigned int>::max()) {}
    static constexpr unsigned int CONTAINER_IDX_SHIFT = 28;
    static constexpr unsigned int CONTAINER_IDX_MASK = (1u << 31) | (1u << 30) | (1u << 29) | (1u << 28);
    static constexpr unsigned int ELEMENT_IDX_MASK = ~CONTAINER_IDX_MASK;
    static unsigned int createRangeValue(unsigned int container_idx, unsigned int index)
    {
      assert(container_idx < (1u << (32 - CONTAINER_IDX_SHIFT)));
      assert((index & CONTAINER_IDX_MASK) == 0u);
      return (container_idx << CONTAINER_IDX_SHIFT) | index;
    }
    void setRangeBegin(std::size_t container_idx, unsigned int element_idx)
    {
      assert(container_idx < (1u << (32 - CONTAINER_IDX_SHIFT)));
      this->first = MeasurementRange::createRangeValue(container_idx, element_idx);
    }
    void setRangeEnd(std::size_t container_idx, unsigned int element_idx)
    {
      this->second = MeasurementRange::createRangeValue(container_idx, element_idx);
    }
    unsigned int containerIndex() const
    {
      assert((this->first & CONTAINER_IDX_MASK) == (this->second & CONTAINER_IDX_MASK));
      return (this->first & CONTAINER_IDX_MASK) >> CONTAINER_IDX_SHIFT;
    }
    unsigned int elementBeginIndex() const
    {
      assert((this->first & CONTAINER_IDX_MASK) == (this->second & CONTAINER_IDX_MASK));
      return this->first & ELEMENT_IDX_MASK;
    }
    unsigned int elementEndIndex() const
    {
      assert((this->first & CONTAINER_IDX_MASK) == (this->second & CONTAINER_IDX_MASK));
      return this->second & ELEMENT_IDX_MASK;
    }
    bool empty() const { return this->first == this->second; }
  };

  // List of measurement ranges and the measurement container targeted by the ranges.
  struct MeasurementRangeList : public std::vector<MeasurementRange>
  {
    const xAOD::UncalibratedMeasurementContainer *container(unsigned int container_index) const
    {
      assert(container_index < m_measurementContainer.size());
      return m_measurementContainer[container_index];
    }
    std::vector<const xAOD::UncalibratedMeasurementContainer *> m_measurementContainer;
  };

  /// Accessor for the above source link container
  ///
  /// It wraps up a few lookup methods to be used in the Combinatorial Kalman
  /// Filter
  class UncalibSourceLinkAccessor
  {
  private:
    const EventContext *m_eventContext;
    const std::vector<Acts::GeometryIdentifier> *m_orderedGeoIds;
    const MeasurementRangeList *m_measurementRanges;

  public:
    class BaseIterator
    {
    public:
      BaseIterator(const EventContext &ctx,
                   const xAOD::UncalibratedMeasurementContainer *container,
                   unsigned int element_index,
                   const Acts::GeometryIdentifier &geometry_id)
          : m_container(container),
            m_index(element_index),
            m_refElementLink(*container,
                             static_cast<ActsTrk::ATLASUncalibSourceLink::index_type>(0),
                             Atlas::getExtendedEventContext(ctx).proxy()),
            m_geometryId(geometry_id)

      {
        if (m_refElementLink.isValid() && !container->empty())
        {
          m_refElementLink.getStorableObjectPointer();
        }
      }
      BaseIterator &operator++()
      {
        ++m_index;
        return *this;
      }
      bool operator==(const BaseIterator &a) const { return m_index == a.m_index && m_container == a.m_container; }

      Acts::SourceLink operator*() const
      {
        assert(m_container && m_index < m_container->size());
        return Acts::SourceLink(ActsTrk::ATLASUncalibSourceLink(m_refElementLink, m_index));
      }
      using value_type = unsigned int;
      using difference_type = unsigned int;
      using pointer = const xAOD::UncalibratedMeasurementContainer **;
      using reference = const xAOD::UncalibratedMeasurementContainer *;
      using iterator_category = std::input_iterator_tag;

    private:
      const xAOD::UncalibratedMeasurementContainer *m_container;
      unsigned int m_index;
      ActsTrk::ATLASUncalibSourceLink m_refElementLink;
      Acts::GeometryIdentifier m_geometryId;
    };

    using Iterator = Acts::SourceLinkAdapterIterator<BaseIterator>;
    UncalibSourceLinkAccessor(const EventContext &ctx,
                              const std::vector<Acts::GeometryIdentifier> &ordered_geoIds,
                              const MeasurementRangeList &measurement_ranges)
        : m_eventContext(&ctx),
          m_orderedGeoIds(&ordered_geoIds),
          m_measurementRanges(&measurement_ranges)
    {
    }
    // get the range of elements with requested geoId
    std::pair<Iterator, Iterator> range(const Acts::Surface &surface) const
    {
      std::vector<Acts::GeometryIdentifier>::const_iterator
          geo_iter = std::lower_bound(m_orderedGeoIds->begin(), m_orderedGeoIds->end(), surface.geometryId());
      if (geo_iter == m_orderedGeoIds->end() || *geo_iter != surface.geometryId() || (*m_measurementRanges).at(geo_iter - m_orderedGeoIds->begin()).empty())
      {
        return {Iterator(BaseIterator(*m_eventContext, nullptr, 0u, surface.geometryId())),
                Iterator(BaseIterator(*m_eventContext, nullptr, 0u, surface.geometryId()))};
      }

      assert(static_cast<std::size_t>(geo_iter - m_orderedGeoIds->begin()) < m_measurementRanges->size());
      const MeasurementRange &range = (*m_measurementRanges).at(geo_iter - m_orderedGeoIds->begin());
      const xAOD::UncalibratedMeasurementContainer *container = m_measurementRanges->container(range.containerIndex());
      return {Iterator(BaseIterator(*m_eventContext, container, range.elementBeginIndex(), surface.geometryId())),
              Iterator(BaseIterator(*m_eventContext, container, range.elementEndIndex(), surface.geometryId()))};
    }
  };

  void gatherGeoIds(const ActsTrk::IActsToTrkConverterTool &converter_tool,
                    const InDetDD::SiDetectorElementCollection &detectorElements,
                    std::vector<Acts::GeometryIdentifier> &geo_ids,
                    std::vector<const Acts::Surface *> &acts_surfaces)
  {
    for (const auto *det_el : detectorElements)
    {
      const Acts::Surface &surface =
          converter_tool.trkSurfaceToActsSurface(det_el->surface());
      geo_ids.push_back(surface.geometryId());
      acts_surfaces.push_back(&surface);
    }
  }

  /// Adapted from Acts Examples/Algorithms/TrackFinding/src/TrackFindingAlgorithmFunction.cpp

  using Stepper = Acts::EigenStepper<>;
  using Navigator = Acts::Navigator;
  using Propagator = Acts::Propagator<Stepper, Navigator>;
  using CKF = Acts::CombinatorialKalmanFilter<Propagator, ActsTrk::MutableTrackStateBackend>;

  // Small holder class to keep CKF and related objects.
  // Keep a unique_ptr<CKF_pimpl> in TrackFindingAlg, so we don't have to expose the
  // Acts class definitions in TrackFindingAlg.h.
  // ActsTrk::TrackFindingAlg::CKF_pimpl inherits from CKF_config to prevent -Wsubobject-linkage warning.
  struct CKF_config
  {
    // CKF algorithm
    CKF ckf;
    // CKF configuration
    Acts::MeasurementSelector measurementSelector;
    Acts::PropagatorPlainOptions pOptions;
    Acts::CombinatorialKalmanFilterExtensions<ActsTrk::MutableTrackStateBackend> ckfExtensions;
    // Track selection
    Acts::TrackSelector trackSelector;
  };

  // === DuplicateSeedDetector ===============================================

  // Identify duplicate seeds: seeds where all measurements were already located in a previously followed trajectory.
  class DuplicateSeedDetector
  {
  public:
    DuplicateSeedDetector(size_t numSeeds, bool enabled)
        : m_disabled(!enabled),
          m_nUsedMeasurements(enabled ? numSeeds : 0u, 0u),
          m_nSeedMeasurements(enabled ? numSeeds : 0u, 0u),
          m_isDuplicateSeed(enabled ? numSeeds : 0u, false)
    {
      if (m_disabled)
        return;
      m_seedIndex.reserve(6 * numSeeds); // 6 hits/seed for strips (3 for pixels)
      m_seedOffset.reserve(2);
    }

    DuplicateSeedDetector() = delete;
    DuplicateSeedDetector(const DuplicateSeedDetector &) = delete;
    DuplicateSeedDetector &operator=(const DuplicateSeedDetector &) = delete;

    // add seeds from an associated measurements collection.
    // measurementOffset non-zero is only needed if measurements holds more than one collection (eg. kept for TrackStatePrinter).
    void addSeeds(size_t typeIndex, const ActsTrk::SeedContainer &seeds)
    {
      if (m_disabled)
        return;
      if (!(typeIndex < m_seedOffset.size()))
        m_seedOffset.resize(typeIndex + 1);
      m_seedOffset[typeIndex] = m_numSeed;

      for (const auto *seed : seeds)
      {
        if (!seed)
          continue;
        for (const auto *sp : seed->sp())
        {
          const auto &els = sp->measurements();
          for (const xAOD::UncalibratedMeasurement *meas : els)
          {
            m_seedIndex.insert({meas, m_numSeed});
            ++m_nSeedMeasurements[m_numSeed];
          }
        }
        ++m_numSeed;
      }
    }

    void newTrajectory()
    {
      if (m_disabled || m_found == 0 || m_nextSeed == m_nUsedMeasurements.size())
        return;
      auto beg = m_nUsedMeasurements.begin();
      if (m_nextSeed < m_nUsedMeasurements.size())
        std::advance(beg, m_nextSeed);
      std::fill(beg, m_nUsedMeasurements.end(), 0u);
    }

    void addMeasurement(const ActsTrk::ATLASUncalibSourceLink &sl)
    {
      if (m_disabled || m_nextSeed == m_nUsedMeasurements.size())
        return;
      for (auto [iiseed, eiseed] = m_seedIndex.equal_range(&(**sl)); iiseed != eiseed; ++iiseed)
      {
        size_t iseed = iiseed->second;
        assert(iseed < m_nUsedMeasurements.size());
        if (iseed < m_nextSeed || m_isDuplicateSeed[iseed])
          continue;
        if (++m_nUsedMeasurements[iseed] >= m_nSeedMeasurements[iseed])
        {
          assert(m_nUsedMeasurements[iseed] == m_nSeedMeasurements[iseed]); // shouldn't ever find more
          m_isDuplicateSeed[iseed] = true;
        }
        ++m_found;
      }
    }

    // For complete removal of duplicate seeds, assumes isDuplicate(iseed) is called for monotonically increasing iseed.
    bool isDuplicate(size_t typeIndex, size_t iseed)
    {
      if (m_disabled)
        return false;
      if (typeIndex < m_seedOffset.size())
        iseed += m_seedOffset[typeIndex];
      assert(iseed < m_isDuplicateSeed.size());
      // If iseed not increasing, we will miss some duplicate seeds, but won't exclude needed seeds.
      if (iseed >= m_nextSeed)
        m_nextSeed = iseed + 1;
      return m_isDuplicateSeed[iseed];
    }

  private:
    bool m_disabled = false;
    boost::container::flat_multimap<const xAOD::UncalibratedMeasurement *, size_t> m_seedIndex;
    std::vector<size_t> m_nUsedMeasurements;
    std::vector<size_t> m_nSeedMeasurements;
    std::vector<bool> m_isDuplicateSeed;
    std::vector<size_t> m_seedOffset;
    size_t m_numSeed = 0u;  // count of number of seeds so-far added with addSeeds()
    size_t m_nextSeed = 0u; // index of next seed expected with isDuplicate()
    size_t m_found = 0u;    // count of found seeds for this/last trajectory
  };

  // === TrackFindingMeasurements ============================================

  // Helper class to convert xAOD::PixelClusterContainer or xAOD::StripClusterContainer to UncalibSourceLinkMultiset.
  class TrackFindingMeasurements
  {
  public:
    TrackFindingMeasurements(const std::vector<Acts::GeometryIdentifier> &ordered_geo_ids)
        : m_orderedGeoIds(&ordered_geo_ids)
    {
      m_measurementRanges.resize(m_orderedGeoIds->size(), MeasurementRange());
      m_measurementOffset.reserve(2); // pixels+strips
    }

    TrackFindingMeasurements() = delete;
    TrackFindingMeasurements(const TrackFindingMeasurements &) = delete;
    TrackFindingMeasurements &operator=(const TrackFindingMeasurements &) = delete;

    void addMeasurements(size_t typeIndex,
                         const xAOD::UncalibratedMeasurementContainer &clusterContainer,
                         const InDetDD::SiDetectorElementCollection &detElems,
                         const ToolHandle<ActsTrk::IActsToTrkConverterTool> &ATLASConverterTool)
    {
      // m_measurementOffset only needed for TrackStatePrinter, but it is trivial overhead to save it for each event
      if (!(typeIndex < m_measurementOffset.size()))
        m_measurementOffset.resize(typeIndex + 1);
      m_measurementOffset[typeIndex] = m_measurementsTotal;

      // the following is just in case we call addMeasurements out of order or not for 2 types of measurements
      if (typeIndex >= m_measurementRanges.m_measurementContainer.size())
      {
        m_measurementRanges.m_measurementContainer.resize(typeIndex + 1, nullptr);
      }
      m_measurementRanges.m_measurementContainer[typeIndex] = &clusterContainer;

      xAOD::UncalibMeasType last_measurement_type = xAOD::UncalibMeasType::Other;
      xAOD::DetectorIDHashType last_id_hash = std::numeric_limits<xAOD::DetectorIDHashType>::max();
      unsigned int range_idx = m_measurementRanges.size();
      std::size_t max_measurement_index = 0;
      for (auto *measurement : clusterContainer)
      {
        max_measurement_index = std::max(max_measurement_index, measurement->index());
        const InDetDD::SiDetectorElement *elem =
            detElems.getDetectorElement(measurement->identifierHash());
        if (!elem)
        {
          throw std::domain_error("No detector element for measurement");
        }

        unsigned int sl_idx = measurement->index();
        if (measurement->identifierHash() != last_id_hash || measurement->type() != last_measurement_type)
        {
          const Acts::Surface &surface = ATLASConverterTool->trkSurfaceToActsSurface(elem->surface());
          std::vector<Acts::GeometryIdentifier>::const_iterator
              geo_iter = std::lower_bound(m_orderedGeoIds->begin(), m_orderedGeoIds->end(), surface.geometryId());
          if (geo_iter == m_orderedGeoIds->end() || *geo_iter != surface.geometryId())
          {
            std::stringstream msg;
            msg << "Measurement with unexpected Acts geometryId: " << surface.geometryId()
                << " type = " << static_cast<unsigned int>(measurement->type())
                << " idHash=" << measurement->identifierHash();
            throw std::runtime_error(msg.str());
          }
          range_idx = geo_iter - m_orderedGeoIds->begin();
          if (m_measurementRanges[range_idx].first != std::numeric_limits<unsigned int>::max())
          {
            std::stringstream msg;
            msg << "Measurement not clustered by identifierHash / geometryId. New measurement "
                << measurement->index() << " with geo Id " << surface.geometryId()
                << " type = " << static_cast<unsigned int>(measurement->type())
                << " idHash=" << measurement->identifierHash()
                << " but already recorded for this geo ID the range : " << m_measurementRanges[range_idx].first
                << " .. " << m_measurementRanges[range_idx].second;
            throw std::runtime_error(msg.str());
          }
          m_measurementRanges[range_idx].setRangeBegin(typeIndex, sl_idx);
          last_id_hash = measurement->identifierHash();
          last_measurement_type = measurement->type();
        }
        m_measurementRanges[range_idx].setRangeEnd(typeIndex, sl_idx + 1);
      }
      m_measurementsTotal = std::max(max_measurement_index + 1, clusterContainer.size());
    }

    std::vector<std::pair<const xAOD::UncalibratedMeasurementContainer *, size_t>> measurementOffsets() const
    {
      std::vector<std::pair<const xAOD::UncalibratedMeasurementContainer *, size_t>> offsets;
      offsets.reserve(m_measurementRanges.m_measurementContainer.size() - 1); // first one usually 0
      for (std::size_t typeIndex = 0; typeIndex < m_measurementRanges.m_measurementContainer.size(); ++typeIndex)
      {
        if (measurementOffset(typeIndex) > 0 && m_measurementRanges.m_measurementContainer[typeIndex] != nullptr)
        {
          offsets.emplace_back(m_measurementRanges.m_measurementContainer[typeIndex], measurementOffset(typeIndex));
        }
      }
      return offsets;
    }

    size_t measurementOffset(size_t typeIndex) const { return typeIndex < m_measurementOffset.size() ? m_measurementOffset[typeIndex] : 0u; }
    std::vector<size_t> measurementOffsetVector() const { return m_measurementOffset; }

    const std::vector<Acts::GeometryIdentifier> &orderedGeoIds() const { return *m_orderedGeoIds; }
    const MeasurementRangeList &measurementRanges() const { return m_measurementRanges; }

  private:
    std::vector<size_t> m_measurementOffset;
    const std::vector<Acts::GeometryIdentifier> *m_orderedGeoIds;

    MeasurementRangeList m_measurementRanges;
    std::size_t m_measurementsTotal = 0;
  };

} // anonymous namespace

#endif
