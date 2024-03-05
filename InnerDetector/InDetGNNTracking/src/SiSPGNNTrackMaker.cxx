/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include <memory>
#include <fstream>

#include "SiSPGNNTrackMaker.h"

#include "TrkPrepRawData/PrepRawData.h"

InDet::SiSPGNNTrackMaker::SiSPGNNTrackMaker(
  const std::string& name, ISvcLocator* pSvcLocator)
  : AthReentrantAlgorithm(name, pSvcLocator)
{
  
}

StatusCode InDet::SiSPGNNTrackMaker::initialize()
{
  ATH_CHECK(m_SpacePointsPixelKey.initialize());
  ATH_CHECK(m_SpacePointsSCTKey.initialize());

  ATH_CHECK(m_outputTracksKey.initialize());

  ATH_CHECK(m_trackFitter.retrieve());
  ATH_CHECK(m_seedFitter.retrieve());

  if (m_useTrackFinder.value() == m_useTrackReader.value()) {
    ATH_MSG_ERROR("Use either track finder or track reader, not both.");
    return StatusCode::FAILURE;
  }

  if (m_useTrackFinder) {
    ATH_MSG_INFO("Use GNN Track Finder");
    ATH_CHECK(m_gnnTrackFinder.retrieve());
  }
  if (m_useTrackReader) {
    ATH_MSG_INFO("Use GNN Track Reader");
    ATH_CHECK(m_gnnTrackReader.retrieve());
  }

  return StatusCode::SUCCESS;
}


StatusCode InDet::SiSPGNNTrackMaker::execute(const EventContext& ctx) const
{ 
  SG::WriteHandle<TrackCollection> outputTracks{m_outputTracksKey, ctx};
  ATH_CHECK(outputTracks.record(std::make_unique<TrackCollection>()));

  // get event info
  uint32_t runNumber = ctx.eventID().run_number();
  uint32_t eventNumber = ctx.eventID().event_number();

  std::vector<const Trk::SpacePoint*> spacePoints;

  auto getData = [&](const SG::ReadHandleKey<SpacePointContainer>& containerKey){
    if (not containerKey.empty()){

      SG::ReadHandle<SpacePointContainer> container{containerKey, ctx};

      if (container.isValid()){
        // loop over spacepoint collection
        auto spc = container->begin();
        auto spce = container->end();
        for(; spc != spce; ++spc){
          const SpacePointCollection* spCollection = (*spc);
          auto sp = spCollection->begin();
          auto spe = spCollection->end();
          for(; sp != spe; ++sp) {
            const Trk::SpacePoint* spacePoint = (*sp);
            spacePoints.push_back(spacePoint);
          }
        }
      }
    }
  };

  getData(m_SpacePointsPixelKey);
  getData(m_SpacePointsSCTKey);

  std::vector<std::vector<uint32_t> > TT;
  if (m_gnnTrackFinder.isSet()) {
    ATH_CHECK(m_gnnTrackFinder->getTracks(spacePoints, TT));
  } else if (m_gnnTrackReader.isSet()) {
    m_gnnTrackReader->getTracks(runNumber, eventNumber, TT);
  } else {
    ATH_MSG_ERROR("Both GNNTrackFinder and GNNTrackReader are not set");
    return StatusCode::FAILURE;
  }


  ATH_MSG_DEBUG("Obtained " << TT.size() << " Tracks");

  // loop over all track candidates
  // and perform track fitting for each.
  int trackCounter = -1;
  for (auto& trackIndices : TT) {

    std::vector<const Trk::PrepRawData*> clusters;
    std::vector<const Trk::SpacePoint*> trackCandiate;
    trackCandiate.reserve(trackIndices.size());

    trackCounter++;
    ATH_MSG_DEBUG("Track " << trackCounter << " has " << trackIndices.size() << " spacepoints");

    std::stringstream spCoordinates;

    for (auto& id : trackIndices) {
      //// for each spacepoint, attach all prepRawData to a list.
      if (id > spacePoints.size()) {
        ATH_MSG_WARNING("SpacePoint index "<< id << " out of range: " << spacePoints.size());
        continue;
      }

      const Trk::SpacePoint* sp = spacePoints[id];
      if (sp != nullptr) {
        trackCandiate.push_back(sp);
        clusters.push_back(sp->clusterList().first);
        if (sp->clusterList().second != nullptr) {
          clusters.push_back(sp->clusterList().second);
        }
      }
    }
    ATH_MSG_DEBUG("Track " << trackCounter << " has " << clusters.size() << " clusters");
    ATH_MSG_DEBUG("spacepoints: " << spCoordinates.str());

    // conformal mapping for track parameters
    auto trkParameters = m_seedFitter->fit(trackCandiate);
    if (trkParameters == nullptr) {
      ATH_MSG_WARNING("Conformal mapping failed");
      continue;
    }

    Trk::ParticleHypothesis matEffects = Trk::pion;
    // first fit the track with local parameters and without outlier removal.
    std::unique_ptr<Trk::Track> track = m_trackFitter->fit(ctx, clusters, *trkParameters, false, matEffects);
    if (track != nullptr && track->perigeeParameters() != nullptr) {
      // fit the track again with perigee parameters and without outlier removal.
      track = m_trackFitter->fit(ctx, clusters, *track->perigeeParameters(), false, matEffects);
      if (track != nullptr) {
        // finally fit with outlier removal
        track = m_trackFitter->fit(ctx, clusters, *track->perigeeParameters(), true, matEffects);
        if (track != nullptr && track->trackSummary() != nullptr) {
          outputTracks->push_back(track.release());
        }
      }
    }
  }

  ATH_MSG_DEBUG("Run " << runNumber << ", Event " << eventNumber << " has " << outputTracks->size() << " tracks stored");
  return StatusCode::SUCCESS;
}


///////////////////////////////////////////////////////////////////
// Overload of << operator MsgStream
///////////////////////////////////////////////////////////////////

MsgStream& InDet::operator    << 
  (MsgStream& sl,const InDet::SiSPGNNTrackMaker& se)
{ 
  return se.dump(sl);
}

///////////////////////////////////////////////////////////////////
// Overload of << operator std::ostream
///////////////////////////////////////////////////////////////////

std::ostream& InDet::operator << 
  (std::ostream& sl,const InDet::SiSPGNNTrackMaker& se)
{
  return se.dump(sl);
}   

///////////////////////////////////////////////////////////////////
// Dumps relevant information into the MsgStream
///////////////////////////////////////////////////////////////////

MsgStream& InDet::SiSPGNNTrackMaker::dump( MsgStream& out ) const
{
  out<<std::endl;
  if(msgLvl(MSG::DEBUG))  return dumpevent(out);
  else return dumptools(out);
}

///////////////////////////////////////////////////////////////////
// Dumps conditions information into the MsgStream
///////////////////////////////////////////////////////////////////

MsgStream& InDet::SiSPGNNTrackMaker::dumptools( MsgStream& out ) const
{
  out<<"| Location of output tracks                       | "
     <<std::endl;
  out<<"|----------------------------------------------------------------"
     <<"----------------------------------------------------|"
     <<std::endl;
  return out;
}

///////////////////////////////////////////////////////////////////
// Dumps event information into the ostream
///////////////////////////////////////////////////////////////////

MsgStream& InDet::SiSPGNNTrackMaker::dumpevent( MsgStream& out ) const
{
  return out;
}


std::ostream& InDet::SiSPGNNTrackMaker::dump( std::ostream& out ) const
{
  return out;
}
