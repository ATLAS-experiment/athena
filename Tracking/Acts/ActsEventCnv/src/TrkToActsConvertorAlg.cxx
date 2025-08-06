/*
Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "TrkToActsConvertorAlg.h"

#include "Acts/EventData/VectorTrackContainer.hpp"
#include "ActsGeometryInterfaces/ITrackingGeometryTool.h"
#include "xAODTracking/TrackJacobianAuxContainer.h"
#include "xAODTracking/TrackMeasurementAuxContainer.h"
#include "xAODTracking/TrackParametersAuxContainer.h"
#include "xAODTracking/TrackStateAuxContainer.h"
#include "ActsEvent/MultiTrajectory.h"

StatusCode ActsTrk::TrkToActsConvertorAlg::initialize() {
  ATH_CHECK(m_trackCollectionKeys.initialize());
  ATH_CHECK(m_trackContainerKey.initialize());
  ATH_CHECK(m_convertorTool.retrieve());
  ATH_CHECK(m_trackContainerBackendsHelper.initialize(ActsTrk::prefixFromTrackContainerName(m_trackContainerKey.key())));
  ATH_CHECK(m_geometryContextKey.initialize());
  return StatusCode::SUCCESS;
}

StatusCode ActsTrk::TrkToActsConvertorAlg::execute(
    const EventContext& ctx) const {

  ATH_MSG_VERBOSE("About to create trackContainer");
  Acts::VectorTrackContainer trackBackend;
  Acts::VectorMultiTrajectory trackStateBackend;
  ActsTrk::MutableTrackContainer tc( std::move(trackBackend),
                                     std::move(trackStateBackend) );

  SG::ReadHandle<ActsGeometryContext> gcx = SG::makeHandle(m_geometryContextKey, ctx);
  ATH_CHECK(gcx.isPresent());
  Acts::GeometryContext tgContext = gcx->context();

    
  ATH_MSG_VERBOSE("Loop over track collections");
  for (auto handle : m_trackCollectionKeys.makeHandles(ctx)) {
    ATH_CHECK(handle.isValid());
    ATH_MSG_VERBOSE("Got back " << handle->size() << " tracks from "<< handle.key());

    m_convertorTool->trkTrackCollectionToActsTrackContainer(
          tc, *handle, tgContext);
    ATH_MSG_VERBOSE("multiTraj has  " << tc.trackStateContainer().size() << " states");
  }

  Acts::ConstVectorTrackContainer ctrackBackend( std::move(tc.container()) );
  Acts::ConstVectorMultiTrajectory ctrackStateBackend( std::move(tc.trackStateContainer()) );
  std::unique_ptr< ActsTrk::TrackContainer > constTrackContainer = std::make_unique< ActsTrk::TrackContainer >( std::move(ctrackBackend),
                                                                                                                std::move(ctrackStateBackend) );
  
  auto trackContainerHandle = SG::makeHandle(m_trackContainerKey, ctx);
  ATH_MSG_VERBOSE("Saving " << constTrackContainer->size() << " tracks to "<< trackContainerHandle.key());
  ATH_CHECK(trackContainerHandle.record(std::move(constTrackContainer)));
  return StatusCode::SUCCESS;
}
