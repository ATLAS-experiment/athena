/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "TrackMergerAlg.h"

namespace ActsTrk {

  TrackMergerAlg::TrackMergerAlg(const std::string& name,
				 ISvcLocator* pSvcLocator)
    : AthReentrantAlgorithm(name, pSvcLocator)
  {}

  StatusCode TrackMergerAlg::initialize()
  {
    ATH_MSG_DEBUG( "Initializing " << name() << " ..." );
    
    ATH_CHECK(m_inputTrackCollections.initialize());
    ATH_CHECK(m_outputTrackCollection.initialize());
    // Create all the backends
    ATH_CHECK(m_tracksBackendHandlesHelper.initialize(ActsTrk::prefixFromTrackContainerName(m_outputTrackCollection.key())));
    
    return StatusCode::SUCCESS;
  }
  
  StatusCode TrackMergerAlg::execute(const EventContext& ctx) const
  {
    ATH_MSG_DEBUG("Executing " << name() << " ...");

    ATH_MSG_DEBUG("Creating output (merged) track collection with key '" << m_outputTrackCollection.key() << "'");
    SG::WriteHandle< ActsTrk::TrackContainer > mergedTracksHandle = SG::makeHandle( m_outputTrackCollection, ctx );
    ActsTrk::MutableTrackContainer mergedTracks;

    ATH_MSG_INFO("Retrieving input track collections");
    std::size_t nInputs = 0ul;
    std::vector< const ActsTrk::TrackContainer* > inputCollections;
    inputCollections.reserve(m_inputTrackCollections.size());
    for (const SG::ReadHandleKey< ActsTrk::TrackContainer >& key : m_inputTrackCollections) {
      ATH_MSG_DEBUG("  \\__ Track Collection: " << key.key());
      SG::ReadHandle< ActsTrk::TrackContainer > handle = SG::makeHandle( key, ctx );
      ATH_CHECK(handle.isValid());
      inputCollections.push_back(handle.cptr());
      nInputs += handle.cptr()->size();
      ATH_MSG_DEBUG("    \\__ Entries: " << handle.cptr()->size());
    }
    ATH_MSG_DEBUG("Total input entries is " << nInputs);

    // Copy the tracks from all containers
    for (const ActsTrk::TrackContainer* tracks : inputCollections) {
      for (std::size_t i(0); i<tracks->size(); ++i) {
	const auto& track = tracks->getTrack(i);
	auto destProxy = mergedTracks.getTrack(mergedTracks.addTrack());
	destProxy.copyFrom(track, true);
      }
    }
    ATH_MSG_DEBUG("Created a track container with " << mergedTracks.size() << " elements");

    std::unique_ptr< ActsTrk::TrackContainer > constTracksContainer = m_tracksBackendHandlesHelper.moveToConst(std::move(mergedTracks), ctx);
    ATH_CHECK(mergedTracksHandle.record(std::move(constTracksContainer)));
    
    return StatusCode::SUCCESS;
  }

}
