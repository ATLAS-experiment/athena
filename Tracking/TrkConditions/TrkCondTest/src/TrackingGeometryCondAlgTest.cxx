/*
 *   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "GaudiKernel/ISvcLocator.h"


// Trk includes
#include "TrkCondTest/TrackingGeometryCondAlgTest.h"
#include "TrkGeometry/Layer.h"


StatusCode Trk::TrackingGeometryCondAlgTest::initialize()
{
  ATH_MSG_DEBUG("initialize " << name());

  // Read Handle Key
  ATH_CHECK(m_trackingGeometryReadKey.initialize());


  ATH_CHECK(m_trackingGeometrySvc.retrieve());
  ATH_CHECK(m_trackingGeometryProcessors.retrieve());
  return StatusCode::SUCCESS;
}

StatusCode Trk::TrackingGeometryCondAlgTest::execute(const EventContext& ctx) const {
  
  //Set up read handle
  SG::ReadCondHandle<Trk::TrackingGeometry> readHandle{m_trackingGeometryReadKey, ctx};
  if (!readHandle.isValid() || *readHandle == nullptr) {
    ATH_MSG_WARNING(m_trackingGeometryReadKey.fullKey() << " is not available.");
    return StatusCode::FAILURE;
  }
  const Trk::TrackingGeometry* trkGeom = *readHandle;
  ATH_MSG_INFO( "eventID: "  << ctx.eventID());
  const TrackingGeometry* trackingGeometry = m_trackingGeometrySvc->trackingGeometry();
  if( trackingGeometry == nullptr){
    ATH_MSG_FATAL( "TRACKING GEOMETRY NOT FOUND IN SVC");
    return StatusCode::FAILURE;
  }

  for (const ToolHandle<Trk::IGeometryProcessor>& proc : m_trackingGeometryProcessors) {
    ATH_MSG_VERBOSE("PRINT SVC TG");
    StatusCode sc1 = proc->process(const_cast<Trk::TrackingGeometry&>(*trackingGeometry));
    ATH_CHECK(sc1);
    ATH_MSG_VERBOSE("PRINT COND TG");
    StatusCode sc2 = proc->process(const_cast<Trk::TrackingGeometry&>(*trkGeom));
    ATH_CHECK(sc2);
  }
  return StatusCode::SUCCESS;
}
