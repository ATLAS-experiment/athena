/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "WriteTrackingGeometry.h"

// ATHENA
#include "ActsInterop/Logger.h"

// PACKAGE

#include "ActsPlugins/Json/TrackingGeometryJsonConverter.hpp"

// STL
#include <fstream>
#include <iostream>

namespace ActsTrk{

StatusCode WriteTrackingGeometry::initialize() {
  ATH_MSG_INFO("initializing");

  ATH_CHECK(m_trackingGeometryTool.retrieve());
  ATH_CHECK(m_ctxProvider.initialize());

  return StatusCode::SUCCESS;
}

StatusCode WriteTrackingGeometry::execute(const EventContext& ctx)  {

  if (m_dumped) {
    return StatusCode::SUCCESS;
  }
  std::ofstream outFile{m_outFile};
  if (!outFile.good()) {
    ATH_MSG_FATAL("Failed to open "<<m_outFile);
    return StatusCode::FAILURE;
  }


  m_dumped = true;
  
  /// Define the tacking geometry context
  const Acts::GeometryContext tgContext = m_ctxProvider.getGeometryContext(ctx);

  using Config_t  = Acts::TrackingGeometryJsonConverter::Config;
  
  Config_t cfg = Config_t::defaultConfig();

  Acts::TrackingGeometryJsonConverter converter{cfg,  makeActsAthenaLogger(this, name())};


  nlohmann::json trackGeo = converter.toJson(tgContext, *m_trackingGeometryTool->trackingGeometry());


  outFile<<trackGeo.dump(4);

  ATH_MSG_INFO("Dump tracking geometry JSON: "<<m_outFile);
  
  return StatusCode::SUCCESS;
}


}