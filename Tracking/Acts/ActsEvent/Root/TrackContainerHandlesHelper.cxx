/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "ActsEvent/TrackContainerHandlesHelper.h"
#include "xAODTracking/TrackStateContainer.h"
#include "xAODTracking/TrackState.h"
#include "xAODMeasurementBase/UncalibratedMeasurementContainer.h"

#include "GeoModelKernel/throwExcept.h"

#include <string>
#include <sstream>
#include <regex>
#include <span>

#include "StoreGate/WriteHandle.h"

namespace ActsTrk {

std::string prefixFromTrackContainerName(const std::string& tracks) {
  std::regex word_regex("(.*)Tracks$");
  std::smatch match_regex;
  
  if ( not std::regex_search(tracks, match_regex, word_regex) or match_regex.size() < 2) {
    throw std::runtime_error(
       std::string("prefixFromTrackContainerName: key does not contain "
		   "Tracks in the name ") + tracks);
  }

  return match_regex[1].str();
}

#define RECORD_xAOD(key, container, auxContainer, ctx)                \
  {                                                                   \
    SG::WriteHandle handle{key, ctx};                                 \
    if (!handle.record(std::move(container),                          \
                       std::move(auxContainer)).isSuccess()){         \
        THROW_EXCEPTION("Cannot record "<<key.fullKey()<<" backend"); \
    }                                                                 \
  }

#define INIT_CHECK(key)                                               \
    if (!key.initialize().isSuccess()) {                              \
        THROW_EXCEPTION("Failed to initialize key "<<key.fullKey());  \
    }

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
    ActsTrk::MutableMultiTrajectory&& mmtj, const EventContext& evtContext) const {

  mmtj.trim();

  auto statesBackendHandle = SG::makeHandle(m_statesKey, evtContext);
  auto statesInterface =
      ActsTrk::makeInterfaceContainer<xAOD::TrackStateContainer>(
          mmtj.trackStatesAux());
  RECORD_xAOD(m_statesKey, statesInterface, mmtj.m_trackStatesAux, evtContext);

  auto parametersInterface =
      ActsTrk::makeInterfaceContainer<xAOD::TrackParametersContainer>(
          mmtj.trackParametersAux());
  RECORD_xAOD(m_parametersKey, parametersInterface, mmtj.m_trackParametersAux, evtContext);

  auto jacobiansInterface =
      ActsTrk::makeInterfaceContainer<xAOD::TrackJacobianContainer>(
          mmtj.trackJacobiansAux());
  RECORD_xAOD(m_jacobiansKey, jacobiansInterface, mmtj.m_trackJacobiansAux, evtContext);

  auto measurementsInterface =
      ActsTrk::makeInterfaceContainer<xAOD::TrackMeasurementContainer>(
          mmtj.trackMeasurementsAux());
   RECORD_xAOD(m_measurementsKey, measurementsInterface, mmtj.m_trackMeasurementsAux, evtContext);

  auto surfacesBackendHandle = SG::makeHandle(m_surfacesKey, evtContext);
  RECORD_xAOD(m_surfacesKey, mmtj.m_surfacesBackend, mmtj.m_surfacesBackendAux, evtContext);

  // construct const MTJ version
  auto cmtj = std::make_unique<ActsTrk::MultiTrajectory>(
      DataLink<xAOD::TrackStateAuxContainer>(m_statesKey.key() + "Aux.",
                                             evtContext),
      DataLink<xAOD::TrackParametersAuxContainer>(
        m_parametersKey.key() + "Aux.", evtContext),
      DataLink<xAOD::TrackJacobianAuxContainer>(
        m_jacobiansKey.key() + "Aux.", evtContext),
      DataLink<xAOD::TrackMeasurementAuxContainer>(
        m_measurementsKey.key() + "Aux.", evtContext), 
      DataLink<xAOD::TrackSurfaceAuxContainer>(
        m_surfacesKey.key() + "Aux.", evtContext)
      );
  cmtj->moveSurfaces(&mmtj);
  cmtj->moveLinks(&mmtj);

  return cmtj;
}

std::unique_ptr<ActsTrk::TrackContainer>
MutableTrackContainerHandlesHelper::moveToConst(
    ActsTrk::MutableTrackContainer&& tc, const Acts::GeometryContext& geoContext, const EventContext& evtContext) const {


  std::unique_ptr<ActsTrk::MultiTrajectory> constMtj =
      moveToConst(std::move(tc.trackStateContainer()), evtContext);

  auto constMtjHandle = SG::makeHandle(m_mtjKey, evtContext);
  if (constMtjHandle.record(std::move(constMtj)).isFailure()) {
    throw std::runtime_error(
        "MutableTrackContainerHandlesHelper::moveToConst, can't record "
        "ConstMultiTrajectory");
  }

  auto trackSurfacesAux = std::make_unique<xAOD::TrackSurfaceAuxContainer>();
  tc.container().encodeSurfaces(trackSurfacesAux.get(), geoContext);

  auto interfaceTrackSummaryContainer =
      ActsTrk::makeInterfaceContainer<xAOD::TrackSummaryContainer>(
          tc.container().m_mutableTrackBackendAux.get());
  RECORD_xAOD(m_xAODTrackSummaryKey, interfaceTrackSummaryContainer, tc.container().m_mutableTrackBackendAux, evtContext);

  auto trackSurfaces = ActsTrk::makeInterfaceContainer<xAOD::TrackSurfaceContainer>(
                        trackSurfacesAux.get());
  RECORD_xAOD(m_trackSurfacesKey, trackSurfaces, trackSurfacesAux, evtContext);

  auto constTrackSummary = std::make_unique<ActsTrk::TrackSummaryContainer>(
      DataLink<xAOD::TrackSummaryContainer>(m_xAODTrackSummaryKey.key(),
                                            evtContext));
  constTrackSummary->restoreDecorations();
  constTrackSummary->fillFrom(tc.container());

  auto constTrackSummaryHandle = SG::makeHandle(m_trackSummaryKey, evtContext);
  if (constTrackSummaryHandle.record(std::move(constTrackSummary))
          .isFailure()) {
    throw std::runtime_error(
        "MutableTrackContainerHandlesHelper::moveToConst, can't record "
        "TrackSummary");
  }
  auto constTrack = std::make_unique<ActsTrk::TrackContainer>(
      DataLink<ActsTrk::TrackSummaryContainer>(m_trackSummaryKey.key(),
                                               evtContext),
      DataLink<ActsTrk::MultiTrajectory>(m_mtjKey.key(), evtContext));
  return constTrack;
}

// const version
StatusCode ConstTrackContainerHandlesHelper::initialize(
    const std::string& prefix) {
  m_statesKey = prefix + "TrackStates";
  m_parametersKey = prefix + "TrackParameters";
  m_jacobiansKey = prefix + "TrackJacobians";
  m_measurementsKey = prefix + "TrackMeasurements";
  m_surfacesKey = prefix + "TrackSurfaces";
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
                                           const EventContext& evtContext) const {
  // we need to build it from backends
  DataLink<xAOD::TrackStateAuxContainer> statesLink(m_statesKey.key() + "Aux.",
                                                    evtContext);
  if (not statesLink.isValid()) {
    throw std::runtime_error(
        "ConstMultiTrajectoryHandle::build, StatesLink is invalid");
  }
  // The restoration of the pointers to uncalibrated measurements should only
  // be done once, if it is done in parallel by multiple callers all callers should
  // write exactly the same value to exactly the same memory location
  xAOD::TrackStateAuxContainer *nonConstStatesLink ATLAS_THREAD_SAFE = const_cast<xAOD::TrackStateAuxContainer *>(statesLink.getDataPtr());
  restoreUncalibMeasurementPtr(*nonConstStatesLink);

  DataLink<xAOD::TrackParametersAuxContainer> parametersLink(
      m_parametersKey.key() + "Aux.", evtContext);
  if (not parametersLink.isValid()) {
    throw std::runtime_error(
        "ConstMultiTrajectoryHandle::build, ParametersLink is invalid");
  }

  DataLink<xAOD::TrackJacobianAuxContainer> jacobiansLink(
      m_jacobiansKey.key() + "Aux.", evtContext);
  if (not jacobiansLink.isValid()) {
    throw std::runtime_error(
        "ConstMultiTrajectoryHandle::build, JacobiansLink is invalid");
  }

  DataLink<xAOD::TrackMeasurementAuxContainer> measurementsLink(
      m_measurementsKey.key() + "Aux.", evtContext);
  if (not measurementsLink.isValid()) {
    throw std::runtime_error(
        "ConstMultiTrajectoryHandle::build, MeasurementsLink is invalid");
  }

  DataLink<xAOD::TrackSurfaceAuxContainer> surfacesLink(
      m_surfacesKey.key() + "Aux.", evtContext);
  if (not surfacesLink.isValid()) {
    throw std::runtime_error(
        "ConstMultiTrajectoryHandle::build, SurfacesLink is invalid");
  }


  auto cmtj = std::make_unique<ActsTrk::MultiTrajectory>(
      statesLink, parametersLink, jacobiansLink, measurementsLink, surfacesLink);
  cmtj->fillSurfaces(geo);
  return cmtj;
}

std::unique_ptr<ActsTrk::TrackContainer>
ConstTrackContainerHandlesHelper::build(const Acts::TrackingGeometry* geo,
                                        const Acts::GeometryContext& /*geoContext*/,
                                        const EventContext& evtContext) const {

  std::unique_ptr<ActsTrk::MultiTrajectory> mtj = buildMtj(geo,evtContext);
  auto mtjHandle = SG::makeHandle(m_mtjKey, evtContext);
  if (mtjHandle.record(std::move(mtj)).isFailure()) {
    throw std::runtime_error(
        "ConstTrackContainerHandle<C>::build failed recording MTJ");
  }
  DataLink<xAOD::TrackSummaryContainer> summaryLink(m_xAODTrackSummaryKey.key(),
                                                    evtContext);
  if (not summaryLink.isValid()) {
    throw std::runtime_error(
        "ConstTrackContainerHandle::build, SummaryLink is invalid");
  }

  auto surfacesHandle = SG::makeHandle(m_trackSurfacesKey, evtContext);
  if (not surfacesHandle.isValid()) {
    throw std::runtime_error(
        "ConstTrackContainerHandle::build, SurfaceHandle is invalid");
  }

  auto constTrackSummary = std::make_unique<ActsTrk::TrackSummaryContainer>(summaryLink);
  constTrackSummary->decodeSurfaces( surfacesHandle.cptr()); 
    
  auto summaryHandle = SG::makeHandle(m_trackSummaryKey, evtContext);
  if (summaryHandle.record(std::move(constTrackSummary)).isFailure()) {
    throw std::runtime_error(
        "MutableTrackContainerHandle::build, can't record "
        "TrackSummary");
  }

  auto constTrack = std::make_unique<ActsTrk::TrackContainer>(
      DataLink<ActsTrk::TrackSummaryContainer>(m_trackSummaryKey.key(),
                                               evtContext),
      DataLink<ActsTrk::MultiTrajectory>(m_mtjKey.key(), evtContext));

  return constTrack;

}

namespace {
   template <typename T>
   using const_span = std::span<T const>;

   // get an aux variable for elements (read only)
   template <typename T>
   const_span<T> getElementVector( const xAOD::AuxContainerBase &aux_container, const SG::ConstAccessor<T> &accessor) {
      const T *data = static_cast<const T *>(aux_container.getData (accessor.auxid()));
      const_span<T> ret( data, aux_container.size() );
      return ret;
   }

   // create a new decoration for all elements using the default value
   template <typename T>
   std::span<T> createDecoration(xAOD::AuxContainerBase &aux_container, const SG::Decorator<T> &decor) {
      std::size_t sz=aux_container.size();
      T *data = static_cast<T *>(aux_container.getDecoration(decor.auxid(), sz, sz));
      return std::span<T>( data, sz );
   }

   void throwConflictingUncalibratedMeasurementPointerValue(const xAOD::UncalibratedMeasurement *is,
                                                            const xAOD::UncalibratedMeasurement *should) {
      std::stringstream msg;
      msg << "Conflicting values for TrackState.uncalibratedMeasurement. Already set " << static_cast<const void *>(is)
          << " would set " << static_cast<const void *>(should);
      throw std::runtime_error(msg.str());
   }
}

void ConstTrackContainerHandlesHelper::restoreUncalibMeasurementPtr(xAOD::TrackStateAuxContainer &statesLink) const {
   using link_t = ElementLink< xAOD::UncalibratedMeasurementContainer >;
   static const SG::ConstAccessor< link_t > link_accessor("uncalibratedMeasurementLink");

   if (statesLink.getAuxIDs().test(link_accessor.auxid())){
      const_span<link_t> elementLinks = getElementVector(statesLink, link_accessor);

      static const SG::AuxElement::Decorator< const xAOD::UncalibratedMeasurement * >
         decor("uncalibratedMeasurement");

      std::span<const xAOD::UncalibratedMeasurement *> uncalibratedMeasurements
         = createDecoration( statesLink, decor);

      for (unsigned int index = 0; index < elementLinks.size(); ++index) {
         const link_t &el = elementLinks[index];
         const xAOD::UncalibratedMeasurement *a_measurement = (el.isValid() ? *el : nullptr);
         // @TODO is this check necessary ?
         if (uncalibratedMeasurements[index] != nullptr && a_measurement != uncalibratedMeasurements[index]) {
            throwConflictingUncalibratedMeasurementPointerValue(uncalibratedMeasurements[index], a_measurement);
         }
         uncalibratedMeasurements[index]=a_measurement;
        }
      }
   else {
      std::cerr << "WARNING no uncalibratedMeasurementLink aux data " << std::endl;
   }
}
}  // namespace ActsTrk

#undef INIT_CHECK
#undef RECORD_xAOD