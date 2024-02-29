

/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRKEVENT_TRACKCONTAINERHANDLE_H
#define ACTSTRKEVENT_TRACKCONTAINERHANDLE_H

#include <memory>
#include "Acts/Geometry/TrackingGeometry.hpp"
#include "ActsEvent/MultiTrajectory.h"
#include "ActsEvent/MultiTrajectoryHandle.h"
#include "ActsEvent/TrackContainer.h"
#include "ActsGeometryInterfaces/ActsGeometryContext.h"
#include "GaudiKernel/StatusCode.h"

namespace ActsTrk {

template <class C>
class MutableTrackContainerHandle {
 public:
  /**
   * Defines how the properties should be named:
   * propertyNamePrefix+TrackStateKey, propertyNamePrefix+TrackMeasurementsKey,
   * ... with the default values: name+TrackState, name+TrackMeasurements
   * finally the Acts compatible frontends and the track container itself
   *
   * usage in execute:
   * ActsTrk::TrackContainer tc;
   * //fill the tc
   * ATH_CHECK(m_handle.moveToConst(tc));
   * @arg algorithm/tool pointer
   */
  MutableTrackContainerHandle(C* algorithm,
                              const std::string& propertyNamePrefix,
                              const std::string& namePrefix);

  /**
   * Sets up the handles
   */
  StatusCode initialize();

  /**
   * produces ActsTrk::ConstTrackContainer with all backends stored in SG
   * @arg tc - MutableTrackContainer the source convert (will  be disassembled
   * after the operation)
   * @arg context - event context (needed for SG)
   */
  std::unique_ptr<ActsTrk::TrackContainer> moveToConst(
      ActsTrk::MutableTrackContainer&& tc, const EventContext& context) const;

 private:
  ActsTrk::MutableMultiTrajectoryHandle<C> m_mtjBackendsHandle;
  Gaudi::Property<std::string> m_prefixName;
  SG::WriteHandleKey<ActsTrk::MultiTrajectory> m_mtjKey;

  // track and its backend
  SG::WriteHandleKey<xAOD::TrackSummaryContainer> m_xAODSummaryKey;
  SG::WriteHandleKey<xAOD::TrackSurfaceContainer> m_surfacesKey;
  SG::WriteHandleKey<ActsTrk::TrackSummaryContainer> m_trackBackendKey;
};

template <class C>
class ConstTrackContainerHandle {
 public:
  ConstTrackContainerHandle(C* algorithm, const std::string& propertyNamePrefix,
                            const std::string& namePrefix);

  StatusCode initialize();

  std::unique_ptr<ActsTrk::TrackContainer> build(
      const Acts::TrackingGeometry* geo,
      const Acts::GeometryContext& geoContext,
      const EventContext& context) const;

 private:
  ActsTrk::ConstMultiTrajectoryHandle<C> m_mtjHandle;
  Gaudi::Property<std::string> m_prefixName;
  SG::WriteHandleKey<ActsTrk::MultiTrajectory> m_mtjKey;

  // track and its backend
  SG::ReadHandleKey<xAOD::TrackSummaryContainer> m_xAODSummaryKey;
  SG::ReadHandleKey<xAOD::TrackSurfaceContainer> m_surfacesKey;
  SG::WriteHandleKey<ActsTrk::TrackSummaryContainer> m_summaryKey;
};

//////// implementation of Mutable handle
template <class C>
MutableTrackContainerHandle<C>::MutableTrackContainerHandle(
    C* algorithm, const std::string& propertyNamePrefix,
    const std::string& namePrefix)
    : m_mtjBackendsHandle(algorithm, propertyNamePrefix, namePrefix),
      m_prefixName(algorithm, propertyNamePrefix + "TrackBackEndPrefixName",
                   namePrefix),
      m_mtjKey(algorithm, propertyNamePrefix + "MTJKey",
               namePrefix + "MultiTrajectory"),
      m_xAODSummaryKey(algorithm, propertyNamePrefix + "xAODTrackSummary",
                            namePrefix + "TrackSummary"),
      m_surfacesKey(algorithm, propertyNamePrefix + "Surfaces",
                    namePrefix + "Surfaces"),
      m_trackBackendKey(algorithm, propertyNamePrefix + "TrackSummary",
                        namePrefix + "TrackSummary") {}

template <class C>
StatusCode MutableTrackContainerHandle<C>::initialize() {
  ATH_CHECK(m_mtjBackendsHandle.initialize());

  // Here we overwrite the keys in case the user has modified the
  // BackEndPrefixName in the JO. We only modify the value
  m_mtjKey = m_prefixName + "MultiTrajectory";
  m_xAODSummaryKey = m_prefixName + "TrackSummary";
  m_surfacesKey = m_prefixName + "Surfaces";
  m_trackBackendKey = m_prefixName + "TrackSummary";

  // And now we initialize the keys
  ATH_CHECK(m_mtjKey.initialize());
  ATH_CHECK(m_xAODSummaryKey.initialize());
  ATH_CHECK(m_surfacesKey.initialize());
  ATH_CHECK(m_trackBackendKey.initialize());
  return StatusCode::SUCCESS;
}

template <class C>
std::unique_ptr<ActsTrk::TrackContainer>
MutableTrackContainerHandle<C>::moveToConst(ActsTrk::MutableTrackContainer&& tc,
                                            const EventContext& context) const {

  std::unique_ptr<ActsTrk::MultiTrajectory> constMtj =
      m_mtjBackendsHandle.moveToConst(std::move(tc.trackStateContainer()),
                                      context);

  auto constMtjHandle = SG::makeHandle(m_mtjKey, context);
  if (constMtjHandle.record(std::move(constMtj)).isFailure()) {
    throw std::runtime_error(
        "MutableTrackContainerHandle::moveToConst, can't record "
        "ConstMultiTrajectory");
  }

  auto xAODTrackSummaryHandle = SG::makeHandle(m_xAODSummaryKey, context);
  auto interfaceTrackSummaryContainer =
      ActsTrk::makeInterfaceContainer<xAOD::TrackSummaryContainer>(
          tc.container().m_mutableTrackBackendAux.get());
  if (xAODTrackSummaryHandle
          .record(std::move(interfaceTrackSummaryContainer),
                  std::move(tc.container().m_mutableTrackBackendAux))
          .isFailure()) {
    throw std::runtime_error(
        "MutableTrackContainerHandle::moveToConst, can't record "
        "xAODTrackSummary");
  }

  auto surfacesHandle = SG::makeHandle(m_surfacesKey, context);
  if (surfacesHandle
          .record(std::move(tc.container().m_mutableSurfBackend),
                  std::move(tc.container().m_mutableSurfBackendAux))
          .isFailure()) {
    throw std::runtime_error(
        "MutableTrackContainerHandle::moveToConst, can't record "
        "xAODTrackSurfaces");
  }
  auto constTrackSummary = std::make_unique<ActsTrk::TrackSummaryContainer>(
      DataLink<xAOD::TrackSummaryContainer>(m_xAODSummaryKey.key(),
                                            context));
  constTrackSummary->restoreDecorations();
  constTrackSummary->fillFrom(tc.container());

  auto constTrackSummaryHandle = SG::makeHandle(m_trackBackendKey, context);
  if (constTrackSummaryHandle.record(std::move(constTrackSummary))
          .isFailure()) {
    throw std::runtime_error(
        "MutableTrackContainerHandle::moveToConst, can't record "
        "TrackSummary");
  }
  auto constTrack = std::make_unique<ActsTrk::TrackContainer>(
      DataLink<ActsTrk::TrackSummaryContainer>(m_trackBackendKey.key(),
                                               context),
      DataLink<ActsTrk::MultiTrajectory>(m_mtjKey.key(), context));
  return constTrack;
}
////// const handle implementation
template <class C>
ConstTrackContainerHandle<C>::ConstTrackContainerHandle(
    C* algorithm, const std::string& propertyNamePrefix,
    const std::string& namePrefix)
    : m_mtjHandle(algorithm, propertyNamePrefix, namePrefix),
      m_mtjKey(algorithm, propertyNamePrefix + "MultiTrajectory",
               namePrefix + "MultiTrajectory"),
      m_xAODSummaryKey(algorithm, propertyNamePrefix + "xAODTrackSummary",
                            namePrefix + "TrackSummary"),
      m_surfacesKey(algorithm, propertyNamePrefix + "SurfacesKey",
                    namePrefix + "Surfaces"),
      m_summaryKey(algorithm, propertyNamePrefix + "TrackSummary",
                        namePrefix + "TrackSummary") {}

template <class C>
StatusCode ConstTrackContainerHandle<C>::initialize() {
  ATH_CHECK(m_mtjHandle.initialize());
  ATH_CHECK(m_mtjKey.initialize());
  ATH_CHECK(m_xAODSummaryKey.initialize());
  ATH_CHECK(m_summaryKey.initialize());
  ATH_CHECK(m_surfacesKey.initialize());
  return StatusCode::SUCCESS;
}

template <class C>
std::unique_ptr<ActsTrk::TrackContainer> ConstTrackContainerHandle<C>::build(
    const Acts::TrackingGeometry* geo, const Acts::GeometryContext& geoContext,
    const EventContext& context) const {
  std::unique_ptr<ActsTrk::MultiTrajectory> mtj =
      m_mtjHandle.build(geo, geoContext, context);
  auto mtjHandle = SG::makeHandle(m_mtjKey, context);
  if (mtjHandle.record(std::move(mtj)).isFailure()) {
    throw std::runtime_error(
        "ConstTrackContainerHandle<C>::build failed recording MTJ");
  }
  DataLink<xAOD::TrackSummaryContainer> summaryLink(
      m_xAODSummaryKey.key(), context);
  if (not summaryLink.isValid()) {
    throw std::runtime_error(
        "ConstTrackContainerHandle::build, SummaryLink is invalid");
  }

  DataLink<xAOD::TrackSurfaceAuxContainer> surfacesLink(
      m_surfacesKey.key()+"Aux.", context);
  if (not surfacesLink.isValid()) {
    throw std::runtime_error(
        "ConstTrackContainerHandle::build, SurfaceLink is invalid");
  }

  auto constTrackSummary =
      std::make_unique<ActsTrk::TrackSummaryContainer>(summaryLink, surfacesLink);
  auto summaryHandle = SG::makeHandle(m_summaryKey, context);
  if (summaryHandle.record(std::move(constTrackSummary)).isFailure()) {
    throw std::runtime_error(
        "MutableTrackContainerHandle::moveToConst, can't record "
        "TrackSummary");
  }

  auto constTrack = std::make_unique<ActsTrk::TrackContainer>(
      DataLink<ActsTrk::TrackSummaryContainer>(m_summaryKey.key(),
                                               context),
      DataLink<ActsTrk::MultiTrajectory>(m_mtjKey.key(), context));

  return constTrack;
}

}  // namespace ActsTrk

#endif
