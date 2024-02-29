/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ActsEvent_MultiTrajectoryHandle_h
#define ActsEvent_MultiTrajectoryHandle_h
#include "Acts/Geometry/TrackingGeometry.hpp"
#include "ActsEvent/MultiTrajectory.h"
#include "ActsGeometryInterfaces/ActsGeometryContext.h"
#include "GaudiKernel/StatusCode.h"
#include "MultiTrajectory.h"
#include "xAODTracking/TrackJacobianContainer.h"
#include "xAODTracking/TrackMeasurementContainer.h"
#include "xAODTracking/TrackParametersContainer.h"
#include "xAODTracking/TrackStateContainer.h"

namespace ActsTrk {

/**
 * Helper to simplify MutableMTJ construction and creation of the ConstMTJ
 * sharing the same backends
 *
 * It is modelled to be like the the usual SG handles.
 * The declaration in the algorithm would look like this.
 * In the class declaration:
 * MutableMultiTrajectoryHandle m_mtjHandle{this, "PropNamePrefix",
 * "TrackNamePrefix"}; Then in initialize():
 * ATH_CHECK(m_mtjHandle.initialize());
 * again identically to how it is done with the handles.
 *
 * The only differences is that the handle has additional methods
 * to build the MutableMTJ and ConstMTJ.
 *
 */
template <class C>
class MutableMultiTrajectoryHandle {
 public:
  /**
   * Defines how the properties should be named:
   * propertyNamePrefix+TrackStateKey, propertyNamePrefix+TrackMeasurementsKey,
   * ... with the default values: name+TracState, name+TrackMeasurements finally
   * the const MTJ version property and name are:
   * propertyNamePrefix+MultiTrajectoryKey and name: namePrefix+MultiTrajectory
   */
  MutableMultiTrajectoryHandle(C* algorithm,
                               const std::string& propertyNamePrefix,
                               const std::string& namePrefix);

  /**
   * Sets up the handles
   */
  StatusCode initialize();

  /**
   * builds ConstMTJ pointing to the same backends as MutableMTJ
   * @warning this call should happen last e.g. once MutableMTJ is not altered
   * anymore, the MutableMTJ is actually purged in this operation
   */
  std::unique_ptr<ActsTrk::MultiTrajectory> moveToConst(
      ActsTrk::MutableMultiTrajectory&& mmtj,
      const EventContext& context) const;

 private:
  Gaudi::Property<std::string> m_prefixName;

  SG::WriteHandleKey<xAOD::TrackStateContainer> m_statesKey;
  SG::WriteHandleKey<xAOD::TrackParametersContainer> m_parametersKey;
  SG::WriteHandleKey<xAOD::TrackJacobianContainer> m_jacobiansKey;
  SG::WriteHandleKey<xAOD::TrackMeasurementContainer> m_measurementsKey;
  SG::WriteHandleKey<xAOD::TrackSurfaceContainer> m_surfacesKey;
};

/**
 * Helper to simplify the ConstMTJ construction when reading from a file
 * It is expected that there will be an algorithm which will use this
 * functionality to build the ConstMTJ and clients would just use the ConstMTJ
 * directly
 */
template <class C>
class ConstMultiTrajectoryHandle {
 public:
  /**
   * see constructor of MutableMultiTrajectory
   */
  ConstMultiTrajectoryHandle(C* algorithm,
                             const std::string& propertyNamePrefix,
                             const std::string& namePrefix);
  /**
   * Sets up the handles
   */
  StatusCode initialize();

  /**
   * builds ConstMTJ, the geometry is used to obtain Acts::Surface pointers
   * if geo pointer is a nullptr and the surfaces are not in the original
   * collection an exception is thrown
   */
  std::unique_ptr<ActsTrk::MultiTrajectory> build(
      const Acts::TrackingGeometry* geo,
      const Acts::GeometryContext& geoContext,
      const EventContext& context) const;

 private:
  Gaudi::Property<std::string> m_prefixName;

  SG::ReadHandleKey<xAOD::TrackStateContainer> m_statesKey;
  SG::ReadHandleKey<xAOD::TrackParametersContainer> m_parametersKey;
  SG::ReadHandleKey<xAOD::TrackJacobianContainer> m_jacobiansKey;
  SG::ReadHandleKey<xAOD::TrackMeasurementContainer> m_measurementsKey;
};

/// implementations

template <class C>
MutableMultiTrajectoryHandle<C>::MutableMultiTrajectoryHandle(
    C* algorithm, const std::string& propertyNamePrefix,
    const std::string& namePrefix)
    : m_prefixName(algorithm, propertyNamePrefix + "MTJBackEndPrefixName",
                   namePrefix),
      m_statesKey(algorithm, propertyNamePrefix + "StatesKey",
                  namePrefix + "States"),
      m_parametersKey(algorithm, propertyNamePrefix + "ParametersKey",
                      namePrefix + "Parameters"),
      m_jacobiansKey(algorithm, propertyNamePrefix + "JacobiansKey",
                     namePrefix + "Jacobians"),
      m_measurementsKey(algorithm, propertyNamePrefix + "MeasurementsKey",
                        namePrefix + "Measurements"),
      m_surfacesKey(algorithm, propertyNamePrefix + "StateSurfacesKey",
                    namePrefix + "StateSurfaces") {}

template <class C>
StatusCode MutableMultiTrajectoryHandle<C>::initialize() {
  // Here we overwrite the keys in case the user has modified the
  // BackEndPrefixName in the JO. We only modify the value
  m_statesKey = m_prefixName + "States";
  m_parametersKey = m_prefixName + "Parameters";
  m_jacobiansKey = m_prefixName + "Jacobians";
  m_measurementsKey = m_prefixName + "Measurements";
  m_surfacesKey = m_prefixName + "StateSurfaces";

  // And now we initialize the keys
  ATH_CHECK(m_statesKey.initialize());
  ATH_CHECK(m_parametersKey.initialize());
  ATH_CHECK(m_jacobiansKey.initialize());
  ATH_CHECK(m_measurementsKey.initialize());
  ATH_CHECK(m_surfacesKey.initialize());

  return StatusCode::SUCCESS;
}

template <class C>
std::unique_ptr<ActsTrk::MultiTrajectory>
MutableMultiTrajectoryHandle<C>::moveToConst(
    ActsTrk::MutableMultiTrajectory&& mmtj, const EventContext& context) const {

  mmtj.trim();
  auto statesBackendHandle = SG::makeHandle(m_statesKey, context);
  auto statesInterface =
      ActsTrk::makeInterfaceContainer<xAOD::TrackStateContainer>(
          mmtj.trackStatesAux());
  if (statesBackendHandle
          .record(std::move(statesInterface), std::move(mmtj.m_trackStatesAux))
          .isFailure()) {
    throw std::runtime_error(
        "MutableMultiTrajectoryHandle::moveToConst, can't record TrackStates "
        "backend");
  }

  auto parametersBackendHandle = SG::makeHandle(m_parametersKey, context);
  auto parametersInterface =
      ActsTrk::makeInterfaceContainer<xAOD::TrackParametersContainer>(
          mmtj.trackParametersAux());
  if (parametersBackendHandle
          .record(std::move(parametersInterface),
                  std::move(mmtj.m_trackParametersAux))
          .isFailure()) {
    throw std::runtime_error(
        "MutableMultiTrajectoryHandle::moveToConst, can't record "
        "TrackParameters "
        "backend");
  }

  auto jacobiansBackendHandle = SG::makeHandle(m_jacobiansKey, context);
  auto jacobiansInterface =
      ActsTrk::makeInterfaceContainer<xAOD::TrackJacobianContainer>(
          mmtj.trackJacobiansAux());
  if (jacobiansBackendHandle
          .record(std::move(jacobiansInterface),
                  std::move(mmtj.m_trackJacobiansAux))
          .isFailure()) {
    throw std::runtime_error(
        "MutableMultiTrajectoryHandle::moveToConst, can't record "
        "TrackJacobians "
        "backend");
  }

  auto measurementsBackendHandle = SG::makeHandle(m_measurementsKey, context);
  auto measurementsInterface =
      ActsTrk::makeInterfaceContainer<xAOD::TrackMeasurementContainer>(
          mmtj.trackMeasurementsAux());
  if (measurementsBackendHandle
          .record(std::move(measurementsInterface),
                  std::move(mmtj.m_trackMeasurementsAux))
          .isFailure()) {
    throw std::runtime_error(
        "MutableMultiTrajectoryHandle::moveToConst, can't record "
        "TrackMeasurements "
        "backend");
  }

  auto surfacesBackendHandle = SG::makeHandle(m_surfacesKey, context);
  if (surfacesBackendHandle
          .record(std::move(mmtj.m_surfacesBackend),
                  std::move(mmtj.m_surfacesBackendAux))
          .isFailure()) {
    throw std::runtime_error(
        "MutableMultiTrajectoryHandle::moveToConst, can't record Surfaces "
        "backend");
  }

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

/**
 * Helper for ConstMTJ creation
 * It would be needed only in an algorithm scheduled bring the ConstMTJ
 * from persistent form.
 */

template <class C>
ConstMultiTrajectoryHandle<C>::ConstMultiTrajectoryHandle(
    C* algorithm, const std::string& propertyNamePrefix,
    const std::string& namePrefix)
    : m_prefixName(algorithm, propertyNamePrefix + "MTJBackEndPrefixName",
                   namePrefix),
      m_statesKey(algorithm, propertyNamePrefix + "TrackStatesKey",
                  namePrefix + "TrackStates"),
      m_parametersKey(algorithm, propertyNamePrefix + "TrackParametersKey",
                      namePrefix + "TrackParameters"),
      m_jacobiansKey(algorithm, propertyNamePrefix + "TrackJacobiansKey",
                     namePrefix + "TrackJacobians"),
      m_measurementsKey(algorithm, propertyNamePrefix + "TrackMeasurementsKey",
                        namePrefix + "TrackMeasurements") {}

template <class C>
StatusCode ConstMultiTrajectoryHandle<C>::initialize() {
  // Here we overwrite the keys in case the user has modified the
  // BackEndPrefixName in the JO. We only modify the value
  m_statesKey = m_prefixName + "States";
  m_parametersKey = m_prefixName + "Parameters";
  m_jacobiansKey = m_prefixName + "Jacobians";
  m_measurementsKey = m_prefixName + "Measurements";

  // And not we initialize the keys
  ATH_CHECK(m_statesKey.initialize());
  ATH_CHECK(m_parametersKey.initialize());
  ATH_CHECK(m_jacobiansKey.initialize());
  ATH_CHECK(m_measurementsKey.initialize());

  return StatusCode::SUCCESS;
}

template <class C>
std::unique_ptr<ActsTrk::MultiTrajectory> ConstMultiTrajectoryHandle<C>::build(
    const Acts::TrackingGeometry* geo, const Acts::GeometryContext& geoContext,
    const EventContext& context) const {
  // we need to build it from backends
  DataLink<xAOD::TrackStateAuxContainer> statesLink(m_statesKey.key() + "Aux.",
                                                    context);
  if (not statesLink.isValid()) {
    throw std::runtime_error("ConstMultiTrajectoryHandle::build, StatesLink is invalid");
  }
  DataLink<xAOD::TrackParametersAuxContainer> parametersLink(
      m_parametersKey.key() + "Aux.", context);
  if (not parametersLink.isValid()) {
    throw std::runtime_error("ConstMultiTrajectoryHandle::build, ParametersLink is invalid");
  }

  DataLink<xAOD::TrackJacobianAuxContainer> jacobiansLink(
      m_jacobiansKey.key() + "Aux.", context);
  if (not jacobiansLink.isValid()) {
    throw std::runtime_error("ConstMultiTrajectoryHandle::build, JacobiansLink is invalid");
  }

  DataLink<xAOD::TrackMeasurementAuxContainer> measurementsLink(
      m_measurementsKey.key() + "Aux.", context);
  if (not measurementsLink.isValid()) {
    throw std::runtime_error("ConstMultiTrajectoryHandle::build, MeasurementsLink is invalid");
  }


  auto cmtj = std::make_unique<ActsTrk::MultiTrajectory>(statesLink, parametersLink, jacobiansLink, measurementsLink);
  cmtj->fillSurfaces(geo, geoContext);
  return cmtj;
}

}  // namespace ActsTrk

#endif  // ActsEvent_MultiTrajectory_h
