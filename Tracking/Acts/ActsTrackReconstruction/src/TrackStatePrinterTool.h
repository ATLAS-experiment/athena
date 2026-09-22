/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRACKRECONSTRUCTION_TRACKSTATEPRINTERTOOL_H
#define ACTSTRACKRECONSTRUCTION_TRACKSTATEPRINTERTOOL_H

// Base
#include "AthenaBaseComps/AthAlgTool.h"

// ATHENA
#include "GeoPrimitives/GeoPrimitives.h"
#include "GaudiKernel/EventContext.h"
#include "StoreGate/ReadHandleKeyArray.h"
#include "xAODInDetMeasurement/SpacePoint.h"
#include "xAODMeasurementBase/UncalibratedMeasurementContainer.h"

// ACTS CORE
#include "Acts/Geometry/TrackingGeometry.hpp"
#include "Acts/EventData/BoundTrackParameters.hpp"
#include "Acts/EventData/TrackStateType.hpp"

// PACKAGE
#include "src/detail/MeasurementIndex.h"
#include "ActsEvent/SeedContainer.h"
#include "ActsEvent/TrackContainer.h"
#include "ActsEvent/ContextUtility.h"
#include "ActsGeometryInterfaces/GeometryContext.h"
#include "ActsGeometryInterfaces/ITrackingGeometrySvc.h"
#include "xAODInDetMeasurement/SpacePointContainer.h"
#include "ActsCalibrators/xAODUncalibMeasSurfAcc.h"

#include "ITrackFindingMonitorTool.h"
// Other
#include <vector>
#include <memory>
#include <tuple>
#include <boost/container/small_vector.hpp>

namespace Acts
{
  class Surface;
}

namespace ActsTrk
{
  class TrackStatePrinterTool : public extends<AthAlgTool, ITrackFindingMonitorTool>
  {
  public:
    using base_class::base_class;

    virtual ~TrackStatePrinterTool() = default;

    // standard Athena methods
    virtual StatusCode initialize() override;

     virtual void newEvent(const EventContext &, const Acts::GeometryContext &) const override {}

    virtual void measurements(const EventContext &ctx,
                              const std::vector<const xAOD::UncalibratedMeasurementContainer *> &clusterContainers,
                              const std::vector<size_t> &offsets) const override {
       printMeasurements(ctx, clusterContainers, offsets);
    }

    virtual void
    newSeed(const Acts::GeometryContext &tgContext,
            const ActsTrk::Seed &seed,
            const Acts::BoundTrackParameters &initialParameters,
            const detail::MeasurementIndex &measurementIndexer,
            unsigned int iseed,
            bool isKF,
            const char *seedType,
            bool first_seed) const override {
       printSeed(tgContext, seed, initialParameters, measurementIndexer,  iseed,isKF, seedType, first_seed);
    }

    virtual void
    newTrack(const Acts::GeometryContext &tgContext,
             const detail::RecoTrackContainer &tracks,
             const typename detail::RecoTrackContainer::TrackProxy &track,
             const detail::MeasurementIndex &measurementIndexer,
             bool rejected = false) const override {
       printTrack(tgContext, tracks,track,measurementIndexer, rejected);
    }

    virtual bool
    newTrackState(const Acts::GeometryContext &tgContext,
                  [[maybe_unused]] const detail::RecoTrackContainer &track_container,
                  [[maybe_unused]] const typename detail::RecoTrackContainer::TrackProxy &track,
                  const detail::RecoTrackContainer::TrackStateProxy &state,
                  const detail::MeasurementIndex &measurementIndexer,
                  [[maybe_unused]] unsigned int status,
                  bool useFiltered = false,
                  bool newLine = true) const override {
       return printTrackState(tgContext, state, measurementIndexer, useFiltered, newLine);
    }

    virtual void finalizeEvent([[maybe_unused]] const EventContext &ctx) const override {}

    void printMeasurements(const EventContext &ctx,
                           const std::vector<const xAOD::UncalibratedMeasurementContainer *> &clusterContainers,
                           const std::vector<size_t> &offsets) const;

    void
    printSeed(const Acts::GeometryContext &tgContext,
              const ActsTrk::Seed &seed,
              const Acts::BoundTrackParameters &initialParameters,
              const detail::MeasurementIndex &measurementIndexer,
              unsigned int iseed,
              bool isKF,
              const char *seedType,
              bool first_seed) const;

    template <typename track_container_t>
    void
    printTrack(const Acts::GeometryContext &tgContext,
               const track_container_t &tracks,
               const typename track_container_t::TrackProxy &track,
               const detail::MeasurementIndex &measurementIndexer,
               bool rejected = false) const;

    template <typename track_state_proxy_t>
    bool
    printTrackState(const Acts::GeometryContext &tgContext,
                    const track_state_proxy_t &state,
                    const detail::MeasurementIndex &measurementIndexer,
                    bool useFiltered = false,
                    bool newLine = true) const;

    using MeasurementInfo = std::tuple<size_t,
                                       const xAOD::UncalibratedMeasurement* *,
                                       std::vector<const xAOD::SpacePoint *>>;

  private:
    // Handles
    SG::ReadHandleKeyArray<xAOD::SpacePointContainer> m_spacePointKey{this, "InputSpacePoints", {}, "Input Space Points for debugging"};

    // Tools
   ServiceHandle<ActsTrk::ITrackingGeometrySvc> m_trackingGeometrySvc{this, "TrackingGeometrySvc", "ActsTrackingGeometrySvc"};

    detail::xAODUncalibMeasSurfAcc m_surfAcc{};

    /** @brief Context provider for geometry, magnetic field and calibration contexts */
    ActsTrk::ContextUtility m_ctxProvider{this};

    // Configuration
    Gaudi::Property<bool> m_compareMeasurementTransforms{this, "compareMeasurementTransforms", false, "compare measurement coordinates transformed with Athena or ACTS"};
    Gaudi::Property<bool> m_printFilteredStates{this, "printFilteredStates", false, "print track states during filtering"};

    // most measurements are associated to only one SP, but allow some headroom to reduce number of allocations
    static constexpr unsigned int N_SP_PER_MEAS = 2;
    template <class T>
    using small_vector = boost::container::small_vector<T, N_SP_PER_MEAS>;

    std::vector<std::vector<small_vector<const xAOD::SpacePoint *>>>
    addSpacePoints(const EventContext &ctx,
                   const std::vector<const xAOD::UncalibratedMeasurementContainer *> &clusterContainers,
                   const std::vector<size_t> &offset) const;

    void printMeasurementAssociatedSpacePoint(const Acts::GeometryContext &tgContext,
                                              const xAOD::UncalibratedMeasurement *measurement,
                                              const std::vector<small_vector<const xAOD::SpacePoint *>> &measToSp,
                                              size_t offset) const;

    // static member functions used by TrackStatePrinter.icc
    static void printParameters(const Acts::Surface &surface, const Acts::GeometryContext &tgContext, const Acts::BoundVector &bound);
    static std::string actsSurfaceName(const Acts::Surface &surface);
    static std::string trackStateName(Acts::ConstTrackStateTypeMap trackStateType);

  };

} // namespace

#include "src/TrackStatePrinterTool.icc"

#endif
