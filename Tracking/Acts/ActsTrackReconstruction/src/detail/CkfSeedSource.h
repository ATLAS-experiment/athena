/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRACKRECONSTRUCTION_CKFSEEDSOURCE_H
#define ACTSTRACKRECONSTRUCTION_CKFSEEDSOURCE_H

#include "ActsEvent/SeedContainer.h"
#include "ActsEvent/SeedMeasurementsContainer.h"
#include "ActsEvent/TrackParametersContainer.h"
#include "ActsToolInterfaces/ITrackParamsEstimationTool.h"
#include "src/detail/SeedMeasurements.h"

#include "Acts/EventData/BoundTrackParameters.hpp"
#include "Acts/Geometry/GeometryContext.hpp"
#include "Acts/MagneticField/MagneticFieldContext.hpp"
#include "Acts/Utilities/CalibrationContext.hpp"

#include <cstddef>
#include <functional>
#include <optional>
#include <utility>

namespace Acts {
  class TrackingGeometry;
}

namespace ActsTrk {
  struct DetectorElementToActsGeometryIdMap;
}

namespace ActsTrk::detail {

  // === ICkfSeedSource ======================================================
  // Everything the CKF needs to know about one collection of seeds:
  // the initial track parameters, the measurements on each seed (used for the
  // duplicate seed removal and to force the track onto the seed) and an eta
  // for the statistics tables, and whether the CKF starts with an inward search.
  // This decouples TrackFindingAlg from the space point seed EDM, so that the
  // initial parameters can also be provided without space points.
  class ICkfSeedSource {
  public:
    using EstimationResult = std::pair<std::optional<Acts::BoundTrackParameters>,
                                       ITrackParamsEstimationTool::EstimationStatus>;

    virtual ~ICkfSeedSource() = default;

    /// Number of seeds in the collection
    virtual std::size_t size() const = 0;

    /// Measurements used to flag the seed as duplicate once they are found on a track
    virtual void duplicateMeasurements(std::size_t iseed, SeedMeasurements &out) const = 0;

    /// All measurements on the seed, used to force the track onto the seed
    virtual void seedMeasurements(std::size_t iseed, SeedMeasurements &out) const = 0;

    /// Seed eta, used for the statistics when no track parameters are available
    virtual double statEta(std::size_t iseed) const = 0;

    /// True if the initial parameters are not at the innermost seed point, so that
    /// the CKF first searches inward to the perigee and then outward
    virtual bool searchInward(std::size_t iseed) const = 0;

    /// Initial track parameters of the seed
    virtual EstimationResult initialParameters(std::size_t iseed,
                                               const Acts::GeometryContext &geoContext,
                                               const Acts::MagneticFieldContext &magFieldContext,
                                               const Acts::CalibrationContext &calibContext) const = 0;

    /// Underlying space point seeds, if any (only used for debug printouts)
    virtual const ActsTrk::SeedContainer *seedContainer() const { return nullptr; }
  };

  // === SpacePointSeedSource ================================================
  // Space point seeds, with the initial track parameters estimated on the fly
  // by an ITrackParamsEstimationTool at the space point chosen by searchStartSpFun.
  class SpacePointSeedSource final : public ICkfSeedSource {
  public:
    /// Seed space point where the search starts (0: bottom SP, search forward)
    using SearchStartSpFun_t = std::function<std::size_t(const ActsTrk::Seed&)>;

    SpacePointSeedSource(const ActsTrk::SeedContainer &seeds,
                         const ITrackParamsEstimationTool &paramEstimationTool,
                         const DetectorElementToActsGeometryIdMap &detectorElementToGeometryIdMap,
                         const Acts::TrackingGeometry &trackingGeometry,
                         SearchStartSpFun_t searchStartSpFun);

    virtual std::size_t size() const override { return m_seeds.size(); }
    virtual void duplicateMeasurements(std::size_t iseed, SeedMeasurements &out) const override;
    virtual void seedMeasurements(std::size_t iseed, SeedMeasurements &out) const override;
    virtual double statEta(std::size_t iseed) const override;
    virtual bool searchInward(std::size_t iseed) const override;
    virtual EstimationResult initialParameters(std::size_t iseed,
                                               const Acts::GeometryContext &geoContext,
                                               const Acts::MagneticFieldContext &magFieldContext,
                                               const Acts::CalibrationContext &calibContext) const override;
    virtual const ActsTrk::SeedContainer *seedContainer() const override { return &m_seeds; }

  private:
    const Acts::Surface &retrieveSurface(const xAOD::SpacePoint &sp, std::size_t measIndex) const;

    const ActsTrk::SeedContainer &m_seeds;
    const ITrackParamsEstimationTool &m_paramEstimationTool;
    ITrackParamsEstimationTool::SpacePointIndicesFun_t m_spacePointIndicesFun;
    const DetectorElementToActsGeometryIdMap &m_detectorElementToGeometryIdMap;
    const Acts::TrackingGeometry &m_trackingGeometry;
    SearchStartSpFun_t m_searchStartSpFun;
  };

  // === ParameterSeedSource =================================================
  // Seeds given by precomputed initial track parameters and the measurements
  // of each seed, e.g. made on an accelerator. No space points are needed.
  // Entry i of both containers belongs to seed i; a null parameter entry
  // means that the parameter estimation failed for this seed.
  class ParameterSeedSource final : public ICkfSeedSource {
  public:
    ParameterSeedSource(const ActsTrk::BoundTrackParametersContainer &parameters,
                        const ActsTrk::SeedMeasurementsContainer &measurements);

    virtual std::size_t size() const override { return m_parameters.size(); }
    virtual void duplicateMeasurements(std::size_t iseed, SeedMeasurements &out) const override;
    virtual void seedMeasurements(std::size_t iseed, SeedMeasurements &out) const override;
    virtual double statEta(std::size_t iseed) const override;
    /// The parameters are given at the bottom of the seed: always search outward
    virtual bool searchInward(std::size_t /*iseed*/) const override { return false; }
    virtual EstimationResult initialParameters(std::size_t iseed,
                                               const Acts::GeometryContext &geoContext,
                                               const Acts::MagneticFieldContext &magFieldContext,
                                               const Acts::CalibrationContext &calibContext) const override;

  private:
    const ActsTrk::BoundTrackParametersContainer &m_parameters;
    const ActsTrk::SeedMeasurementsContainer &m_measurements;
  };

}  // namespace ActsTrk::detail

#endif
