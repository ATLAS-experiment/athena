/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "TrackAnalysisAlg.h"

namespace ActsTrk {

  TrackAnalysisAlg::TrackAnalysisAlg(const std::string& name, ISvcLocator* pSvcLocator)
    : AthMonitorAlgorithm(name, pSvcLocator) 
  {}

  StatusCode TrackAnalysisAlg::initialize() {
    ATH_MSG_INFO("Initializing " << name() << " ...");

    ATH_MSG_DEBUG("Properties:");
    ATH_MSG_DEBUG(m_tracksKey);

    ATH_CHECK(m_tracksKey.initialize());
     
    ATH_MSG_DEBUG("Monitoring settings ...");
    ATH_MSG_DEBUG(m_monGroupName);

    return AthMonitorAlgorithm::initialize();
  }

  StatusCode TrackAnalysisAlg::fillHistograms(const EventContext& ctx) const {
    ATH_MSG_DEBUG( "Filling Histograms for " << name() << " ... " );

    // Retrieve the tracks
    SG::ReadHandle<ActsTrk::TrackContainer> trackHandle = SG::makeHandle(m_tracksKey, ctx);
    ATH_CHECK(trackHandle.isValid());
    const ActsTrk::TrackContainer *tracks = trackHandle.cptr();

    auto monitor_ntracks = Monitored::Scalar<int>("Ntracks", tracks->size());
    fill(m_monGroupName.value(), monitor_ntracks);
    
    return StatusCode::SUCCESS;
  }

}
