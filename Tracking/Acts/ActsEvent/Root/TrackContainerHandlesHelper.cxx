/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "ActsEvent/TrackContainerHandlesHelper.h"

#include <string>

#include "StoreGate/WriteHandle.h"

namespace ActsTrk {

std::string prefixFromTrackContainerName(const std::string& tracks) {
  auto sindex = tracks.find("Tracks");
  if (sindex == std::string::npos)
    throw std::runtime_error(
          std::string("prefixFromTrackContainerName: key does not contain "
                    "Tracks in the name ") + tracks);
  return tracks.substr(0, tracks.find("Tracks"));
}

template <typename T, typename IFACE, typename AUX>
void recordxAOD(const SG::WriteHandleKey<T>& key, IFACE& iface, AUX& aux,
                const EventContext& context) {
  SG::WriteHandle<T> handle = SG::makeHandle(key, context);
  if (handle.record(std::move(iface), std::move(aux)).isFailure()) {
    throw std::runtime_error(
        std::string("MutableTrackContainerHandlesHelper::recordxAOD, can't record ") + key.key() + " backend");
  }
}

namespace {
template <typename KTYPE>
void INIT_CHECK(KTYPE& key) {
  if (key.initialize().isFailure()) {
    throw std::runtime_error(
        std::string("MutableTrackContainerHandlesHelper: can not initialize handle") + key.key());
  }
};
}  // namespace

StatusCode MutableTrackContainerHandlesHelper::initialize(
    const std::string& prefix) {
  m_statesKey = prefix + "TrackStates";
  m_parametersKey = prefix + "TrackParameters";
  m_jacobiansKey = prefix + "TrackJacobians";
  m_measurementsKey = prefix + "TrackMeasurements";
  m_surfacesKey = prefix + "TrackStateSurfaces";
  m_mtjKey =
      prefix + "MultiTrajectory";  // identical names, underlying types are distinct
  INIT_CHECK(m_statesKey);
  INIT_CHECK(m_parametersKey);
  INIT_CHECK(m_jacobiansKey);
  INIT_CHECK(m_measurementsKey);
  INIT_CHECK(m_surfacesKey);
  INIT_CHECK(m_mtjKey);

  // Track Container backends
  m_xAODTrackSummaryKey = prefix + "TrackSummary";
  m_trackSurfacesKey = prefix + "TrackSurfaces";
  m_trackSummaryKey =
      prefix +
      "TrackSummary";  // identical names, underlying types are distinct

  INIT_CHECK(m_xAODTrackSummaryKey);
  INIT_CHECK(m_trackSurfacesKey);
  INIT_CHECK(m_trackSummaryKey);
  return StatusCode::SUCCESS;
}

std::unique_ptr<ActsTrk::MultiTrajectory>
MutableTrackContainerHandlesHelper::moveToConst(
    ActsTrk::MutableMultiTrajectory&& mmtj, const EventContext& context) const {

  mmtj.trim();

  auto statesBackendHandle = SG::makeHandle(m_statesKey, context);
  auto statesInterface =
      ActsTrk::makeInterfaceContainer<xAOD::TrackStateContainer>(
          mmtj.trackStatesAux());
  recordxAOD(m_statesKey, statesInterface, mmtj.m_trackStatesAux, context);

  auto parametersInterface =
      ActsTrk::makeInterfaceContainer<xAOD::TrackParametersContainer>(
          mmtj.trackParametersAux());
  recordxAOD(m_parametersKey, parametersInterface, mmtj.m_trackParametersAux, context);

  auto jacobiansInterface =
      ActsTrk::makeInterfaceContainer<xAOD::TrackJacobianContainer>(
          mmtj.trackJacobiansAux());
  recordxAOD(m_jacobiansKey, jacobiansInterface, mmtj.m_trackJacobiansAux, context);

  auto measurementsInterface =
      ActsTrk::makeInterfaceContainer<xAOD::TrackMeasurementContainer>(
          mmtj.trackMeasurementsAux());
   recordxAOD(m_measurementsKey, measurementsInterface, mmtj.m_trackMeasurementsAux, context);

  auto surfacesBackendHandle = SG::makeHandle(m_surfacesKey, context);
  recordxAOD(m_surfacesKey, mmtj.m_surfacesBackend, mmtj.m_surfacesBackendAux, context);

  // construct const MTJ version
  auto cmtj = std::make_unique<ActsTrk::MultiTrajectory>(
      DataLink<xAOD::TrackStateAuxContainer>(m_statesKey.key() + "Aux.",
                                             context),
      DataLink<xAOD::TrackParametersAuxContainer>(
          m_parametersKey.key() + "Aux.", context),
      DataLink<xAOD::TrackJacobianAuxContainer>(m_jacobiansKey.key() + "Aux.",
                                                context),
      DataLink<xAOD::TrackMeasurementAuxContainer>(
          m_measurementsKey.key() + "Aux.", context));
  cmtj->moveSurfaces(&mmtj);
  cmtj->moveLinks(&mmtj);

  return cmtj;
}

std::unique_ptr<ActsTrk::TrackContainer>
MutableTrackContainerHandlesHelper::moveToConst(
    ActsTrk::MutableTrackContainer&& tc, const EventContext& context) const {

  std::unique_ptr<ActsTrk::MultiTrajectory> constMtj =
      moveToConst(std::move(tc.trackStateContainer()), context);

  auto constMtjHandle = SG::makeHandle(m_mtjKey, context);
  if (constMtjHandle.record(std::move(constMtj)).isFailure()) {
    throw std::runtime_error(
        "MutableTrackContainerHandlesHelper::moveToConst, can't record "
        "ConstMultiTrajectory");
  }

  auto interfaceTrackSummaryContainer =
      ActsTrk::makeInterfaceContainer<xAOD::TrackSummaryContainer>(
          tc.container().m_mutableTrackBackendAux.get());
  recordxAOD(m_xAODTrackSummaryKey, interfaceTrackSummaryContainer, tc.container().m_mutableTrackBackendAux, context);

  recordxAOD(m_trackSurfacesKey, tc.container().m_mutableSurfBackend, tc.container().m_mutableSurfBackendAux, context);

  auto constTrackSummary = std::make_unique<ActsTrk::TrackSummaryContainer>(
      DataLink<xAOD::TrackSummaryContainer>(m_xAODTrackSummaryKey.key(),
                                            context));
  constTrackSummary->restoreDecorations();
  constTrackSummary->fillFrom(tc.container());

  auto constTrackSummaryHandle = SG::makeHandle(m_trackSummaryKey, context);
  if (constTrackSummaryHandle.record(std::move(constTrackSummary))
          .isFailure()) {
    throw std::runtime_error(
        "MutableTrackContainerHandlesHelper::moveToConst, can't record "
        "TrackSummary");
  }
  auto constTrack = std::make_unique<ActsTrk::TrackContainer>(
      DataLink<ActsTrk::TrackSummaryContainer>(m_trackSummaryKey.key(),
                                               context),
      DataLink<ActsTrk::MultiTrajectory>(m_mtjKey.key(), context));
  return constTrack;
}

// const version
StatusCode ConstTrackContainerHandlesHelper::initialize(
    const std::string& prefix) {
  m_statesKey = prefix + "States";
  m_parametersKey = prefix + "Parameters";
  m_jacobiansKey = prefix + "Jacobians";
  m_measurementsKey = prefix + "Measurements";
  m_surfacesKey = prefix + "Surfaces";
  m_mtjKey = prefix + "MultiTrajectory";

  INIT_CHECK(m_statesKey);
  INIT_CHECK(m_parametersKey);
  INIT_CHECK(m_jacobiansKey);
  INIT_CHECK(m_measurementsKey);
  INIT_CHECK(m_surfacesKey);
  INIT_CHECK(m_mtjKey);

  m_xAODTrackSummaryKey = prefix + "TrackSummary";
  m_trackSurfacesKey = prefix + "TrackSurfaces";
  m_trackSummaryKey = prefix + "TrackSummary";

  INIT_CHECK(m_xAODTrackSummaryKey);
  INIT_CHECK(m_trackSurfacesKey);
  INIT_CHECK(m_trackSummaryKey);

  return StatusCode::SUCCESS;
}

std::unique_ptr<ActsTrk::MultiTrajectory>
ConstTrackContainerHandlesHelper::buildMtj(const Acts::TrackingGeometry* geo,
                                        const Acts::GeometryContext& geoContext,
                                        const EventContext& context) const {
  // we need to build it from backends
  DataLink<xAOD::TrackStateAuxContainer> statesLink(m_statesKey.key() + "Aux.",
                                                    context);
  if (not statesLink.isValid()) {
    throw std::runtime_error(
        "ConstMultiTrajectoryHandle::build, StatesLink is invalid");
  }
  DataLink<xAOD::TrackParametersAuxContainer> parametersLink(
      m_parametersKey.key() + "Aux.", context);
  if (not parametersLink.isValid()) {
    throw std::runtime_error(
        "ConstMultiTrajectoryHandle::build, ParametersLink is invalid");
  }

  DataLink<xAOD::TrackJacobianAuxContainer> jacobiansLink(
      m_jacobiansKey.key() + "Aux.", context);
  if (not jacobiansLink.isValid()) {
    throw std::runtime_error(
        "ConstMultiTrajectoryHandle::build, JacobiansLink is invalid");
  }

  DataLink<xAOD::TrackMeasurementAuxContainer> measurementsLink(
      m_measurementsKey.key() + "Aux.", context);
  if (not measurementsLink.isValid()) {
    throw std::runtime_error(
        "ConstMultiTrajectoryHandle::build, MeasurementsLink is invalid");
  }

  auto cmtj = std::make_unique<ActsTrk::MultiTrajectory>(
      statesLink, parametersLink, jacobiansLink, measurementsLink);
  cmtj->fillSurfaces(geo, geoContext);
  return cmtj;
}

std::unique_ptr<ActsTrk::TrackContainer>
ConstTrackContainerHandlesHelper::build(const Acts::TrackingGeometry* geo,
                                        const Acts::GeometryContext& geoContext,
                                        const EventContext& context) const {

  std::unique_ptr<ActsTrk::MultiTrajectory> mtj =
      buildMtj(geo, geoContext, context);
  auto mtjHandle = SG::makeHandle(m_mtjKey, context);
  if (mtjHandle.record(std::move(mtj)).isFailure()) {
    throw std::runtime_error(
        "ConstTrackContainerHandle<C>::build failed recording MTJ");
  }
  DataLink<xAOD::TrackSummaryContainer> summaryLink(m_xAODTrackSummaryKey.key(),
                                                    context);
  if (not summaryLink.isValid()) {
    throw std::runtime_error(
        "ConstTrackContainerHandle::build, SummaryLink is invalid");
  }

  DataLink<xAOD::TrackSurfaceAuxContainer> surfacesLink(
      m_trackSurfacesKey.key() + "Aux.", context);
  if (not surfacesLink.isValid()) {
    throw std::runtime_error(
        "ConstTrackContainerHandle::build, SurfaceLink is invalid");
  }

  auto constTrackSummary = std::make_unique<ActsTrk::TrackSummaryContainer>(
      summaryLink, surfacesLink);
  auto summaryHandle = SG::makeHandle(m_trackSummaryKey, context);
  if (summaryHandle.record(std::move(constTrackSummary)).isFailure()) {
    throw std::runtime_error(
        "MutableTrackContainerHandle::moveToConst, can't record "
        "TrackSummary");
  }

  auto constTrack = std::make_unique<ActsTrk::TrackContainer>(
      DataLink<ActsTrk::TrackSummaryContainer>(m_trackSummaryKey.key(),
                                               context),
      DataLink<ActsTrk::MultiTrajectory>(m_mtjKey.key(), context));

  return constTrack;

}
}  // namespace ActsTrk
