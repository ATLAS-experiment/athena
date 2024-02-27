/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRKFINDINGTOOL_TRACKSTATEPRINTER_H
#define ACTSTRKFINDINGTOOL_TRACKSTATEPRINTER_H 1

// Base
#include "src/ITrackStatePrinter.h"
#include "AthenaBaseComps/AthAlgTool.h"

// ATHENA
#include "StoreGate/ReadHandleKeyArray.h"
#include "xAODInDetMeasurement/SpacePoint.h"

// ACTS CORE
#include "Acts/Geometry/TrackingGeometry.hpp"

// PACKAGE
#include "ActsGeometryInterfaces/ActsGeometryContext.h"
#include "ActsGeometryInterfaces/IActsTrackingGeometryTool.h"
#include "xAODInDetMeasurement/SpacePointContainer.h"
#include "ActsEventCnv/IActsToTrkConverterTool.h"

// Other
#include <memory>
#include <tuple>
#include <boost/container/small_vector.hpp>

namespace InDetDD
{
  class SiDetectorElementCollection;
}

namespace ActsTrk
{
  class TrackStatePrinter : public extends<AthAlgTool, ActsTrk::ITrackStatePrinter>
  {
  public:
    TrackStatePrinter(const std::string &type, const std::string &name,
                      const IInterface *parent);
    virtual ~TrackStatePrinter() = default;

    // standard Athena methods
    virtual StatusCode initialize() override;

    void
    printMeasurements(const EventContext &ctx,
                      const std::vector<const xAOD::UncalibratedMeasurementContainer *> &clusterContainers,
                      const std::vector<const InDetDD::SiDetectorElementCollection *> &detectorElementCollections,
                      const std::vector<size_t> &offsets) const override;

    void
    printSeed(const Acts::GeometryContext &tgContext,
              const ActsTrk::Seed &seed,
              const Acts::BoundTrackParameters &initialParameters,
              size_t measurementOffset,
              size_t iseed,
              bool isKF) const override;

    void
    printTracks(const Acts::GeometryContext &tgContext,
                const ActsTrk::MutableTrackContainer &tracks,
                const std::vector<ActsTrk::MutableTrackContainer::TrackProxy> &fitResult,
                const std::vector<std::pair<const xAOD::UncalibratedMeasurementContainer *, size_t>> &offset) const override;

    void
    printTrackState(const Acts::GeometryContext &tgContext,
                    const ActsTrk::MutableTrackStateBackend::ConstTrackStateProxy &state,
                    const std::vector<std::pair<const xAOD::UncalibratedMeasurementContainer *, size_t>> &container_offset,
                    bool useFiltered = false) const override;

    using MeasurementInfo = std::tuple<size_t,
                                       const ATLASUncalibSourceLink *,
                                       std::vector<const xAOD::SpacePoint *>>;

  private:
    // Handles
    SG::ReadHandleKeyArray<xAOD::SpacePointContainer> m_spacePointKey{this, "InputSpacePoints", {}, "Input Space Points for debugging"};

    // Tools
    ToolHandle<IActsTrackingGeometryTool> m_trackingGeometryTool{this, "TrackingGeometryTool", "ActsTrackingGeometryTool"};
    ToolHandle<ActsTrk::IActsToTrkConverterTool> m_ATLASConverterTool{this, "ATLASConverterTool", "ActsToTrkConverterTool"};

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
                                         const xAOD::UncalibratedMeasurement *measurement,
                                         const std::vector<small_vector<const xAOD::SpacePoint *>> &measToSp,
                                         const InDetDD::SiDetectorElementCollection *detectorElements,
                                         size_t offset) const;
  };

} // namespace

#endif
