/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "src/detail/CkfSeedSource.h"

#include "ActsGeometry/DetectorElementToActsGeometryIdMap.h"
#include "xAODInDetMeasurement/SpacePoint.h"

#include "Acts/Geometry/TrackingGeometry.hpp"

#include <cassert>
#include <cmath>
#include <stdexcept>

namespace ActsTrk::detail {

  SpacePointSeedSource::SpacePointSeedSource(const ActsTrk::SeedContainer &seeds,
                                             const ITrackParamsEstimationTool &paramEstimationTool,
                                             const DetectorElementToActsGeometryIdMap &detectorElementToGeometryIdMap,
                                             const Acts::TrackingGeometry &trackingGeometry,
                                             SearchStartSpFun_t searchStartSpFun)
    : m_seeds(seeds),
      m_paramEstimationTool(paramEstimationTool),
      m_spacePointIndicesFun(paramEstimationTool.spacePointIndicesFun()),
      m_detectorElementToGeometryIdMap(detectorElementToGeometryIdMap),
      m_trackingGeometry(trackingGeometry),
      m_searchStartSpFun(std::move(searchStartSpFun)) {}

  void SpacePointSeedSource::duplicateMeasurements(std::size_t iseed, SeedMeasurements &out) const {
    out.clear();
    // the measurements of the space points that the parameter estimation uses
    const ActsTrk::Seed seed = m_seeds[iseed];
    const auto& sps = seed.sp();
    for (std::size_t isp : m_spacePointIndicesFun(sps, m_searchStartSpFun(seed))) {
      const xAOD::SpacePoint *sp = sps.at(isp);
      for (const xAOD::UncalibratedMeasurement *meas : sp->measurements()) {
        out.push_back(meas);
      }
    }
  }

  void SpacePointSeedSource::seedMeasurements(std::size_t iseed, SeedMeasurements &out) const {
    out.clear();
    for (const xAOD::SpacePoint *sp : m_seeds[iseed].sp()) {
      for (const xAOD::UncalibratedMeasurement *meas : sp->measurements()) {
        out.push_back(meas);
      }
    }
  }

  double SpacePointSeedSource::statEta(std::size_t iseed) const {
    const xAOD::SpacePoint* sp = m_seeds[iseed].sp().front();
    const xAOD::SpacePoint::ConstVectorMap pos = sp->globalPosition();
    return std::atanh(pos[2] / pos.norm());
  }

  bool SpacePointSeedSource::searchInward(std::size_t iseed) const {
    return m_searchStartSpFun(m_seeds[iseed]) > 0;
  }

  ICkfSeedSource::EstimationResult
  SpacePointSeedSource::initialParameters(std::size_t iseed,
                                          const Acts::GeometryContext &geoContext,
                                          const Acts::MagneticFieldContext &magFieldContext,
                                          const Acts::CalibrationContext &calibContext) const {
    const ActsTrk::Seed seed = m_seeds[iseed];
    return m_paramEstimationTool.estimateTrackParameters(seed,
                                                         m_searchStartSpFun(seed),
                                                         geoContext,
                                                         magFieldContext,
                                                         calibContext,
                                                         [this](const xAOD::SpacePoint &sp, std::size_t measIndex) -> const Acts::Surface& {
                                                           return retrieveSurface(sp, measIndex);
                                                         });
  }

  const Acts::Surface &SpacePointSeedSource::retrieveSurface(const xAOD::SpacePoint &sp, std::size_t measIndex) const {
    const xAOD::UncalibratedMeasurement* meas = sp.measurements().at(measIndex);
    const auto geoid_iter = m_detectorElementToGeometryIdMap.find(ActsTrk::makeDetectorElementKey(meas->type(), meas->identifierHash()));
    if (geoid_iter == m_detectorElementToGeometryIdMap.end()) {
      throw std::runtime_error("measurement not linked to Acts surface.");
    }
    const Acts::Surface *surface = DetectorElementToActsGeometryIdMap::getSurface(*geoid_iter);
    if (surface == nullptr) {
      surface = m_trackingGeometry.findSurface(DetectorElementToActsGeometryIdMap::getValue(*geoid_iter));
    }
    assert(surface);
    return *surface;
  }

  ParameterSeedSource::ParameterSeedSource(const ActsTrk::BoundTrackParametersContainer &parameters,
                                           const ActsTrk::SeedMeasurementsContainer &measurements)
    : m_parameters(parameters),
      m_measurements(measurements) {
    if (m_parameters.size() != m_measurements.size()) {
      throw std::invalid_argument("ParameterSeedSource: the number of track parameters and of seed measurements differ");
    }
  }

  void ParameterSeedSource::duplicateMeasurements(std::size_t iseed, SeedMeasurements &out) const {
    const ActsTrk::SeedMeasurementsContainer::MeasurementRange measurements = m_measurements.duplicateDetectionMeasurements(iseed);
    out.assign(measurements.begin(), measurements.end());
  }

  void ParameterSeedSource::seedMeasurements(std::size_t iseed, SeedMeasurements &out) const {
    const ActsTrk::SeedMeasurementsContainer::MeasurementRange measurements = m_measurements.measurements(iseed);
    out.assign(measurements.begin(), measurements.end());
  }

  double ParameterSeedSource::statEta(std::size_t iseed) const {
    const Acts::BoundTrackParameters *parameters = m_parameters[iseed];
    // without parameters there is nothing to compute eta from, use the central bin
    if (parameters == nullptr) return 0.;
    return -std::log(std::tan(0.5 * parameters->theta()));
  }

  ICkfSeedSource::EstimationResult
  ParameterSeedSource::initialParameters(std::size_t iseed,
                                         const Acts::GeometryContext &/*geoContext*/,
                                         const Acts::MagneticFieldContext &/*magFieldContext*/,
                                         const Acts::CalibrationContext &/*calibContext*/) const {
    const Acts::BoundTrackParameters *parameters = m_parameters[iseed];
    if (parameters == nullptr) return {std::nullopt, ITrackParamsEstimationTool::kNoSeedRefit};
    return {*parameters, ITrackParamsEstimationTool::kNoSeedRefit};
  }

}  // namespace ActsTrk::detail
