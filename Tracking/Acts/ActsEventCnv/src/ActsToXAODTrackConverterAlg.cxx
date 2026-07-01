/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "src/ActsToXAODTrackConverterAlg.h"

namespace ActsTrk {


  StatusCode ActsToXAODTrackConverterAlg::initialize()
  {
    ATH_MSG_DEBUG( "Initializing " << name() << " ..." );

    ATH_CHECK( m_inputTrackContainerKey.initialize() );
    ATH_CHECK( m_outputTrackContainerKey.initialize() );

    ATH_CHECK(m_tracksBackendHandlesHelper.initialize(ActsTrk::prefixFromTrackContainerName(m_outputTrackContainerKey.key())));
    ATH_CHECK(m_trackingGeometryTool.retrieve());
    
    return StatusCode::SUCCESS;
  }

  StatusCode ActsToXAODTrackConverterAlg::execute(const EventContext &ctx) const {
    ATH_MSG_DEBUG( "Executing " << name() << " ..." );
    const ActsTrk::TrackContainer* actsTracks{nullptr};
    ATH_CHECK(SG::get(actsTracks, m_inputTrackContainerKey, ctx));
   
    // Convert to xAOD version
    ActsTrk::MutablePersistentTrackContainer tracksContainer;
    // tracksContainer.ensureDynamicColumns(*actsTracks);
    for ( auto track : *actsTracks ) {

      auto destProxy = tracksContainer.makeTrack();
      destProxy.copyFrom(track);
    }
    ATH_MSG_DEBUG("    \\__ Converted " << tracksContainer.size() << " tracks");

    // Make const
   auto constTracksContainer = m_tracksBackendHandlesHelper.moveToConst(std::move(tracksContainer), 
                                                                        m_trackingGeometryTool->getGeometryContext(ctx).context(),
                                                                        ctx );

    // Store into StoreGate
    SG::WriteHandle trackContainerHandle{m_outputTrackContainerKey, ctx};
    ATH_CHECK( trackContainerHandle.record(std::move(constTracksContainer)) );    
    return StatusCode::SUCCESS;
  }
  
} // namespace

