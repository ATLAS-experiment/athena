/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "src/ActsToXAODTrackConverterAlg.h"

namespace ActsTrk {

  ActsToXAODTrackConverterAlg::ActsToXAODTrackConverterAlg(const std::string &name,
                                                           ISvcLocator *pSvcLocator)
    : AthReentrantAlgorithm(name, pSvcLocator)
  {}

  StatusCode ActsToXAODTrackConverterAlg::initialize()
  {
    ATH_MSG_DEBUG( "Initializing " << name() << " ..." );

    ATH_CHECK( m_inputTrackContainerKey.initialize() );
    ATH_CHECK( m_outputTrackContainerKey.initialize() );

    ATH_CHECK(m_tracksBackendHandlesHelper.initialize(ActsTrk::prefixFromTrackContainerName(m_outputTrackContainerKey.key())));
    ATH_CHECK(m_trackingGeometryTool.retrieve());
    
    return StatusCode::SUCCESS;
  }

  StatusCode ActsToXAODTrackConverterAlg::execute(const EventContext &ctx) const
  {
    ATH_MSG_DEBUG( "Executing " << name() << " ..." );

    // Read inputs
    SG::ReadHandle< ActsTrk::TrackContainer > inputTracksHandle = SG::makeHandle( m_inputTrackContainerKey, ctx );
    ATH_CHECK( inputTracksHandle.isValid() );
    const ActsTrk::TrackContainer* actsTracks = inputTracksHandle.cptr();

    // Convert to xAOD version
    ActsTrk::MutablePersistentTrackContainer tracksContainer;    
    for ( auto track : *actsTracks ) {
      auto destProxy = tracksContainer.makeTrack();
      destProxy.copyFrom(track);
    }
    ATH_MSG_DEBUG("    \\__ Converted " << tracksContainer.size() << " tracks");

    // Make const
    std::unique_ptr< ActsTrk::PersistentTrackContainer > constTracksContainer = m_tracksBackendHandlesHelper.moveToConst( std::move(tracksContainer), 
                                                                                                                          m_trackingGeometryTool->getGeometryContext(ctx).context(),
                                                                                                                          ctx );

    // Store into StoreGate
    SG::WriteHandle< ActsTrk::PersistentTrackContainer > trackContainerHandle = SG::makeHandle( m_outputTrackContainerKey, ctx );
    ATH_CHECK( trackContainerHandle.record(std::move(constTracksContainer)) );    
    return StatusCode::SUCCESS;
  }
  
} // namespace

