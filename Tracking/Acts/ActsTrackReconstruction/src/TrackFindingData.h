/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSTRACKRECONSTRUCTION_TRACKFINDINGDATA_H
#define ACTSTRACKRECONSTRUCTION_TRACKFINDINGDATA_H 1

// ACTS
#include "Acts/EventData/VectorTrackContainer.hpp"
#include "Acts/EventData/TrackContainer.hpp"
#include "Acts/EventData/TrackProxy.hpp"
#include "Acts/Definitions/Common.hpp"
#include "Acts/Definitions/Algebra.hpp"
#include "Acts/Propagator/SympyStepper.hpp"
#include "Acts/Propagator/Navigator.hpp"
#include "Acts/Propagator/Propagator.hpp"
#include "Acts/TrackFitting/GainMatrixSmoother.hpp"
#include "Acts/TrackFitting/GainMatrixUpdater.hpp"
#include "Acts/TrackFinding/MeasurementSelector.hpp"
#include "Acts/TrackFinding/CombinatorialKalmanFilter.hpp"
#include "Acts/TrackFinding/TrackSelector.hpp"
#include "Acts/Surfaces/Surface.hpp"

// Athena
#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
#include "xAODMeasurementBase/UncalibratedMeasurement.h"
#include "xAODInDetMeasurement/PixelClusterContainer.h"
#include "xAODInDetMeasurement/StripClusterContainer.h"

// ActsTrk
#include "ActsGeometry/ATLASSourceLink.h"
#include "ActsGeometry/TrackingSurfaceHelper.h"
#include "ActsEventCnv/IActsToTrkConverterTool.h"
#include "src/TrackStatePrinter.h"

// STL
#include <unordered_map>
#include <utility>
#include <vector>
#include <variant>

#include "AtlasUncalibSourceLinkAccessor.h"

/// =========================================================================
/// Include all sorts of stuff needed to interface with the Acts Core classes.
/// =========================================================================
namespace ActsTrk::detail {
  // container used during the reconstruction
  using RecoTrackContainer = Acts::TrackContainer<Acts::VectorTrackContainer,
                                                  Acts::VectorMultiTrajectory>;
  using RecoTrackContainerProxy = RecoTrackContainer::TrackProxy;
  using RecoTrackStateContainer = Acts::VectorMultiTrajectory;
  using RecoTrackStateContainerProxy = RecoTrackStateContainer::TrackStateProxy;



  /// Adapted from Acts Examples/Algorithms/TrackFinding/src/TrackFindingAlgorithmFunction.cpp

  using Stepper = Acts::SympyStepper;
  using Navigator = Acts::Navigator;
  using Propagator = Acts::Propagator<Stepper, Navigator>;
  using CKF = Acts::CombinatorialKalmanFilter<Propagator, RecoTrackContainer>;
  using Extrapolator = Propagator;

  // Small holder class to keep CKF and related objects.
  // Keep a unique_ptr<CKF_pimpl> in TrackFindingAlg, so we don't have to expose the
  // Acts class definitions in TrackFindingAlg.h.
  // ActsTrk::TrackFindingAlg::CKF_pimpl inherits from CKF_config to prevent -Wsubobject-linkage warning.
  struct CKF_config
  {
    // Extrapolator
    Extrapolator extrapolator;
    // CKF algorithm
    CKF ckf;
    // CKF configuration
    Acts::MeasurementSelector measurementSelector;
    Acts::CombinatorialKalmanFilterExtensions<RecoTrackContainer> ckfExtensions;
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
      for (auto [iiseed, eiseed] = m_seedIndex.equal_range(&(ActsTrk::getUncalibratedMeasurement(sl))); iiseed != eiseed; ++iiseed)
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
    std::unordered_multimap<const xAOD::UncalibratedMeasurement *, size_t> m_seedIndex;
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
  class TrackFindingMeasurements {
  public:
    TrackFindingMeasurements(std::size_t measTotal) {
      m_orderedGeoIds.reserve(measTotal);
      m_measurementOffsets.reserve(2);  // pixels+strips
    }

    TrackFindingMeasurements() = delete;
    TrackFindingMeasurements(const TrackFindingMeasurements &) = default;
    TrackFindingMeasurements &operator=(const TrackFindingMeasurements &) = delete;

    void addDetectorElements(xAOD::UncalibMeasType measType,
                             const InDetDD::SiDetectorElementCollection &detElems,
                             const ToolHandle<ActsTrk::IActsToTrkConverterTool> &ATLASConverterTool) {
      assert (m_sorted == false);  // should not call this again after addMeasurements()

      if (!(static_cast<std::size_t>(measType) < TrackingSurfaceHelper::s_NMeasTypes)) {
        std::stringstream msg;
        msg << "Measurements of type " << static_cast<std::size_t>(measType) << " larger than " << TrackingSurfaceHelper::s_NMeasTypes - 1;
        throw std::runtime_error(msg.str());
      }

      if (measType != xAOD::UncalibMeasType::Other) {
        m_trackingSurfaceHelper.setSiDetectorElements(measType, &detElems);
      }

      auto &actsSurfaces = m_trackingSurfaceHelper.actsSurfaces(measType);
      actsSurfaces.reserve(actsSurfaces.size() + detElems.size());  // may extend previous data, but usually starts from empty
      for (const auto *det_el : detElems)
      {
        const Acts::Surface &surface = ATLASConverterTool->trkSurfaceToActsSurface(det_el->surface());
        m_orderedGeoIds.push_back(surface.geometryId());
        actsSurfaces.push_back(&surface);
      }
      m_sorted = false;
    }

    // NB. all addDetectorElements() must have been done before calling first addMeasurements().
    void addMeasurements(size_t typeIndex,
                         const xAOD::UncalibratedMeasurementContainer &clusterContainer,
                         const InDetDD::SiDetectorElementCollection &detElems,
                         const ToolHandle<ActsTrk::IActsToTrkConverterTool> &ATLASConverterTool) {
      if (!m_sorted) {
        std::sort(m_orderedGeoIds.begin(), m_orderedGeoIds.end());
        m_sorted = true;
        m_measurementRanges.resize(m_orderedGeoIds.size());
      }

      // m_measurementOffsets only needed for TrackStatePrinter, but it is trivial overhead to save it for each event
      if (!(typeIndex < m_measurementOffsets.size()))
        m_measurementOffsets.resize(typeIndex + 1);
      m_measurementOffsets[typeIndex] = m_measurementsTotal;

      m_measurementRanges.setContainer(typeIndex, &clusterContainer);

      xAOD::UncalibMeasType last_measurement_type = xAOD::UncalibMeasType::Other;
      xAOD::DetectorIDHashType last_id_hash = std::numeric_limits<xAOD::DetectorIDHashType>::max();
      unsigned int range_idx = m_measurementRanges.size();
      std::size_t sl_idx = 0;
      for (auto *measurement : clusterContainer)
      {
        const InDetDD::SiDetectorElement *elem =
            detElems.getDetectorElement(measurement->identifierHash());
        if (!elem)
        {
          throw std::domain_error("No detector element for measurement");
        }

        if (measurement->identifierHash() != last_id_hash || measurement->type() != last_measurement_type)
        {
          const Acts::Surface &surface = ATLASConverterTool->trkSurfaceToActsSurface(elem->surface());
          std::vector<Acts::GeometryIdentifier>::const_iterator
              geo_iter = std::lower_bound(m_orderedGeoIds.begin(), m_orderedGeoIds.end(), surface.geometryId());
          if (geo_iter == m_orderedGeoIds.end() || *geo_iter != surface.geometryId())
          {
            std::stringstream msg;
            msg << "Measurement with unexpected Acts geometryId: " << surface.geometryId()
                << " type = " << static_cast<unsigned int>(measurement->type())
                << " idHash=" << measurement->identifierHash()
                << (geo_iter == m_orderedGeoIds.end() ? " not" :"") << " found GeoId in orderedGeoIds, that are of size " << m_orderedGeoIds.size();
            throw std::runtime_error(msg.str());
          }
          range_idx = geo_iter - m_orderedGeoIds.begin();
          if (m_measurementRanges[range_idx].first != std::numeric_limits<unsigned int>::max())
          {
            std::stringstream msg;
            msg << "Measurement not clustered by identifierHash / geometryId. New measurement "
                << sl_idx << " with geo Id " << surface.geometryId()
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
        ++sl_idx;
      }
      m_measurementsTotal += clusterContainer.size();
    }

    std::vector<std::pair<const xAOD::UncalibratedMeasurementContainer *, size_t>> measurementContainerOffsets() const
    {
      std::vector<std::pair<const xAOD::UncalibratedMeasurementContainer *, size_t>> offsets;
      if (m_measurementRanges.numContainers() == 0) return offsets;
      offsets.reserve(m_measurementRanges.numContainers() - 1); // first one usually 0
      for (std::size_t typeIndex = 0; typeIndex < m_measurementRanges.numContainers(); ++typeIndex)
      {
        const xAOD::UncalibratedMeasurementContainer *the_container
            = std::visit( [] (const auto &a) -> const xAOD::UncalibratedMeasurementContainer * { return a.containerPtr();} ,
                          m_measurementRanges.container(typeIndex));
        if (measurementOffset(typeIndex) > 0 && the_container != nullptr)
        {
          offsets.emplace_back(the_container, measurementOffset(typeIndex));
        }
      }
      return offsets;
    }

    size_t measurementOffset(size_t typeIndex) const { return typeIndex < m_measurementOffsets.size() ? m_measurementOffsets[typeIndex] : 0u; }
    const std::vector<size_t>& measurementOffsets() const { return m_measurementOffsets; }
    const std::vector<Acts::GeometryIdentifier> &orderedGeoIds() const { return m_orderedGeoIds; }
    const ActsTrk::MeasurementRangeList &measurementRanges() const { return m_measurementRanges; }
    const TrackingSurfaceHelper &trackingSurfaceHelper() const { return m_trackingSurfaceHelper; }

  private:

    std::vector<size_t> m_measurementOffsets;
    std::vector<Acts::GeometryIdentifier> m_orderedGeoIds;
    TrackingSurfaceHelper m_trackingSurfaceHelper;
    ActsTrk::MeasurementRangeList m_measurementRanges;
    std::size_t m_measurementsTotal = 0;
    bool m_sorted = false;
  };
}
#endif
