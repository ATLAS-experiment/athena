/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
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
#include "Acts/EventData/TrackParameters.hpp"
#include "Acts/EventData/TrackStateType.hpp"

// PACKAGE
#include "src/detail/SharedHitCounter.h"
#include "ActsGeometry/ATLASSourceLink.h"
#include "ActsEvent/Seed.h"
#include "ActsEvent/TrackContainer.h"
#include "ActsGeometryInterfaces/ActsGeometryContext.h"
#include "ActsGeometryInterfaces/IActsTrackingGeometryTool.h"
#include "xAODInDetMeasurement/SpacePointContainer.h"
#include "ActsGeometry/DetectorElementToActsGeometryIdMap.h"

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
  class TrackStatePrinterTool : virtual public AthAlgTool
  {
  public:
    TrackStatePrinterTool(const std::string &type,
			  const std::string &name,
			  const IInterface *parent);
    virtual ~TrackStatePrinterTool() = default;

    // standard Athena methods
    virtual StatusCode initialize() override;

    void
    printMeasurements(const EventContext &ctx,
                      const std::vector<const xAOD::UncalibratedMeasurementContainer *> &clusterContainers,
                      const DetectorElementToActsGeometryIdMap &detectorElementToGeometryIdMap,
                      const std::vector<size_t> &offsets) const;

    void
    printSeed(const Acts::GeometryContext &tgContext,
              const ActsTrk::Seed &seed,
              const Acts::BoundTrackParameters &initialParameters,
              const detail::SharedHitCounter &sharedHits,
              size_t iseed,
              bool isKF) const;

    template <typename track_container_t>
    void
    printTrack(const Acts::GeometryContext &tgContext,
               const track_container_t &tracks,
               const typename track_container_t::TrackProxy &track,
               const detail::SharedHitCounter &sharedHits) const;

    template <typename track_state_proxy_t>
    bool
    printTrackState(const Acts::GeometryContext &tgContext,
                    const track_state_proxy_t &state,
                    const detail::SharedHitCounter &sharedHits,
                    bool useFiltered = false,
                    bool newLine = true) const;

    using MeasurementInfo = std::tuple<size_t,
                                       const ATLASUncalibSourceLink *,
                                       std::vector<const xAOD::SpacePoint *>>;

  private:
    // Handles
    SG::ReadHandleKeyArray<xAOD::SpacePointContainer> m_spacePointKey{this, "InputSpacePoints", {}, "Input Space Points for debugging"};

    // Tools
    ToolHandle<IActsTrackingGeometryTool> m_trackingGeometryTool{this, "TrackingGeometryTool", ""};

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

    void
    printMeasurementAssociatedSpacePoint(const Acts::GeometryContext &tgContext,
                                         const Acts::TrackingGeometry &tracking_geometry,
                                         const DetectorElementToActsGeometryIdMap &detectorElementToGeometryIdMap,
                                         const xAOD::UncalibratedMeasurement *measurement,
                                         const std::vector<small_vector<const xAOD::SpacePoint *>> &measToSp,
                                         size_t offset) const;

    // static member functions used by TrackStatePrinter.icc
    static void printParameters(const Acts::Surface &surface, const Acts::GeometryContext &tgContext, const Acts::BoundVector &bound);
    static std::string actsSurfaceName(const Acts::Surface &surface);
    static std::string trackStateName(Acts::ConstTrackStateType trackStateType);

  };

} // namespace

#include "src/TrackStatePrinterTool.icc"

#endif
