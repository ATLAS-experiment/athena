/*
Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "TrkToActsConvertorAlg.h"

#include "Acts/EventData/VectorTrackContainer.hpp"
#include "ActsGeometryInterfaces/IActsTrackingGeometryTool.h"
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
  return StatusCode::SUCCESS;
}

StatusCode ActsTrk::TrkToActsConvertorAlg::execute(
    const EventContext& ctx) const {
  if (!m_convertorTool->trackingGeometryTool()) {
    ATH_MSG_WARNING(
        "Convertor Tool is not returning tracking geometry. Cannot proceed.");
    return StatusCode::SUCCESS;
  }
  ATH_MSG_VERBOSE("create containers");

  ATH_MSG_VERBOSE("About to create trackContainer");
  ActsTrk::MutableTrackContainer tc;
  Acts::GeometryContext tgContext = m_convertorTool->trackingGeometryTool()
                                            ->getGeometryContext(ctx)
                                            .context();

    
  ATH_MSG_VERBOSE("Loop over track collections");
  for (auto handle : m_trackCollectionKeys.makeHandles(ctx)) {
    ATH_CHECK(handle.isValid());
    ATH_MSG_VERBOSE("Got back " << handle->size() << " tracks from "<< handle.key());

    m_convertorTool->trkTrackCollectionToActsTrackContainer(
          tc, *handle, tgContext);
    ATH_MSG_VERBOSE("multiTraj has  " << tc.trackStateContainer().size() << " states");
  }

  // // Let's dump some information for debugging (will be removed later)
  ATH_MSG_VERBOSE("TrackStateContainer has  " << tc.trackStateContainer().trackStatesAux()->size() << " states");
  ATH_MSG_VERBOSE("TrackParametersContainer has  " << tc.trackStateContainer().trackParametersAux()->size() << " parameters");

  std::unique_ptr<ActsTrk::TrackContainer> constTrackContainer = m_trackContainerBackendsHelper.moveToConst(std::move(tc), ctx);
  auto trackContainerHandle = SG::makeHandle(m_trackContainerKey, ctx);
  ATH_MSG_VERBOSE("Saving " << constTrackContainer->size() << " tracks to "<< trackContainerHandle.key());
  ATH_CHECK(trackContainerHandle.record(std::move(constTrackContainer)));
  return StatusCode::SUCCESS;
}
