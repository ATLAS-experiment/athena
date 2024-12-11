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
  ATH_CHECK(m_SpacePointsOverlapKey.initialize());
  ATH_CHECK(m_ClusterPixelKey.initialize());
  ATH_CHECK(m_ClusterStripKey.initialize());

  ATH_CHECK(m_outputTracksKey.initialize());

  ATH_CHECK(m_trackFitter.retrieve());
  ATH_CHECK(m_seedFitter.retrieve());
  ATH_CHECK(m_trackSummaryTool.retrieve());

  if (!m_gnnTrackFinder.empty() && !m_gnnTrackReader.empty()) {
    ATH_MSG_ERROR("Use either track finder or track reader, not both.");
    return StatusCode::FAILURE;
  }

  if (!m_gnnTrackFinder.empty()) {
    ATH_MSG_INFO("Use GNN Track Finder");
    ATH_CHECK(m_gnnTrackFinder.retrieve());
  }
  if (!m_gnnTrackReader.empty()) {
    ATH_MSG_INFO("Use GNN Track Reader");
    ATH_CHECK(m_gnnTrackReader.retrieve());
  }
  if (m_areInputClusters) {
    ATH_MSG_INFO("Use input clusters");
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

  auto getOverlapData = [&](const SG::ReadHandleKey<SpacePointOverlapCollection>& containerKey){
    if (not containerKey.empty()){
      
      SG::ReadHandle<SpacePointOverlapCollection> collection{containerKey, ctx};

      if (collection.isValid()){
        for (const Trk::SpacePoint *sp : *collection) {
          spacePoints.push_back(sp);
        }
      }
    }
  };

  getData(m_SpacePointsPixelKey);
  int npixsp = spacePoints.size();
  ATH_MSG_DEBUG("Event " << eventNumber << " has " << npixsp
                         << " pixel space points");
  getData(m_SpacePointsSCTKey);
  ATH_MSG_DEBUG("Event " << eventNumber << " has "
                         << spacePoints.size() - npixsp << " Strips space points");
  int nNonOverlap = spacePoints.size();
  ATH_MSG_DEBUG("Event " << eventNumber << " has " << nNonOverlap
                         << " non-overlapping spacepoints");
  getOverlapData(m_SpacePointsOverlapKey);
  ATH_MSG_DEBUG("Event " << eventNumber << " has " << spacePoints.size()
                        << " space points");

  // get clusters
  std::vector<const Trk::PrepRawData*> allClusters;
  if (m_areInputClusters) {
    SG::ReadHandle<InDet::PixelClusterContainer> pixcontainer(m_ClusterPixelKey,
                                                            ctx);
    SG::ReadHandle<InDet::SCT_ClusterContainer> strip_container(m_ClusterStripKey,
                                                            ctx);

    if (!pixcontainer.isValid()) {
      ATH_MSG_ERROR("Pixel container invalid, returning");
      return StatusCode::FAILURE;
    }

    if (!strip_container.isValid()) {
      ATH_MSG_ERROR("Strip container invalid, returning");
      return StatusCode::FAILURE;
    }

    auto pixcollection = pixcontainer->begin();
    auto pixcollectionEnd = pixcontainer->end();
    for (; pixcollection != pixcollectionEnd; ++pixcollection) {
      if ((*pixcollection)->empty()) {
        ATH_MSG_WARNING("Empty pixel cluster collection encountered");
        continue;
      }
      auto const* clusterCollection = (*pixcollection);
      auto thisCluster = clusterCollection->begin();
      auto clusterEnd = clusterCollection->end();
      for (; thisCluster != clusterEnd; ++thisCluster) {
        const PixelCluster* cl = (*thisCluster);
        allClusters.push_back(cl);
      }
    }

    auto strip_collection = strip_container->begin();
    auto strip_collectionEnd = strip_container->end();
    for (; strip_collection != strip_collectionEnd; ++strip_collection) {
      if ((*strip_collection)->empty()) {
        ATH_MSG_WARNING("Empty strip cluster collection encountered");
        continue;
      }
      auto const* clusterCollection = (*strip_collection);
      auto thisCluster = clusterCollection->begin();
      auto clusterEnd = clusterCollection->end();
      for (; thisCluster != clusterEnd; ++thisCluster) {
        const SCT_Cluster* cl = (*thisCluster);
        allClusters.push_back(cl);
      }
    }

    ATH_MSG_DEBUG("Event " << eventNumber << " has " << allClusters.size()
                          << " clusters");
  }

  // get tracks

  std::vector<std::vector<uint32_t> > TT;
  std::vector<std::vector<uint32_t> > clusterTracks;
  if (m_gnnTrackFinder.isSet()) {
    ATH_CHECK(m_gnnTrackFinder->getTracks(spacePoints, TT));
  } else if (m_gnnTrackReader.isSet()) {
    // if track candidates are built from cluster, get both clusters and SPs
    if (m_areInputClusters) {
      m_gnnTrackReader->getTracks(runNumber, eventNumber, clusterTracks, TT);
    } else {
      m_gnnTrackReader->getTracks(runNumber, eventNumber, TT);
    }
  } else {
    ATH_MSG_ERROR("Both GNNTrackFinder and GNNTrackReader are not set");
    return StatusCode::FAILURE;
  }

  ATH_MSG_DEBUG("Event " << eventNumber << " obtained " << TT.size() << " Tracks");

  // loop over all track candidates
  // and perform track fitting for each.
  int trackCounter = -1;
  for (auto& trackIndices : TT) {

    std::vector<const Trk::PrepRawData*> clusters;
    std::vector<const Trk::SpacePoint*> trackCandidate;
    trackCandidate.reserve(trackIndices.size());

    trackCounter++;
    ATH_MSG_DEBUG("Track " << trackCounter << " has " << trackIndices.size()
                           << " spacepoints");

    std::stringstream spCoordinates;
    std::vector<std::pair<double, const Trk::SpacePoint*> > distanceSortedSPs;

    // get track space points
    // sort SPs in track by distance from origin
    for (auto& id : trackIndices) {
      //// for each spacepoint, attach all prepRawData to a list.
      if (id > spacePoints.size()) {
        ATH_MSG_WARNING("SpacePoint index "
                        << id << " out of range: " << spacePoints.size());
        continue;
      }

      const Trk::SpacePoint* sp = spacePoints[id];
      if (static_cast<int>(id) > nNonOverlap) {
        ATH_MSG_DEBUG("Track " << trackCounter << " Overlapping Hit " << id
                               << ": (" << sp->globalPosition().x() << ", "
                               << sp->globalPosition().y() << ", "
                               << sp->globalPosition().z() << ")");
      }

      // store distance - hit paire
      if (sp != nullptr) {
        distanceSortedSPs.push_back(
          std::make_pair(
            std::pow(sp->globalPosition().x(), 2) + std::pow(sp->globalPosition().y(), 2),
            sp
          )
        );
      }
    }

    // sort by distance
    std::sort(distanceSortedSPs.begin(), distanceSortedSPs.end());

    // add SP to trk candidate in the same order
    for (std::pair<double, const Trk::SpacePoint*> pair : distanceSortedSPs) {
      trackCandidate.push_back(pair.second);
    }

    // get cluster list
    int nPIX(0), nStrip(0);
    // if use input clusters, get the cluster list from clusterTracks
    if (m_areInputClusters) {
      std::vector<uint32_t> clusterIndices = clusterTracks[trackCounter];
      clusters.reserve(clusterIndices.size());
      for (uint32_t id : clusterIndices) {
        if (id > allClusters.size()) {
          ATH_MSG_ERROR("Cluster index out of range");
          continue;
        }
        if (allClusters[id]->type(Trk::PrepRawDataType::PixelCluster))
          nPIX++;
        if (allClusters[id]->type(Trk::PrepRawDataType::SCT_Cluster))
          nStrip++;
        clusters.push_back(allClusters[id]);
      }  // if not get list of clusters from space points
    } else {
      for (const Trk::SpacePoint* sp : trackCandidate) {
        if (sp->clusterList().first->type(Trk::PrepRawDataType::PixelCluster))
          nPIX++;
        if (sp->clusterList().first->type(Trk::PrepRawDataType::SCT_Cluster))
          nStrip++;
        clusters.push_back(sp->clusterList().first);
        if (sp->clusterList().second != nullptr) {
          clusters.push_back(sp->clusterList().second);
          nStrip++;
        }
      }
    }

    ATH_MSG_DEBUG("Track " << trackCounter << " has " << trackCandidate.size()
                           << " space points, " << clusters.size()
                           << " clusters, " << nPIX << " pixel clusters, "
                           << nStrip << " Strip clusters");

    // reject track with less than 3 space point hits, the conformal map will
    // fail any way
    if (trackCandidate.size() < 3) {
      ATH_MSG_DEBUG(
          "Track "
          << trackCounter
          << " does not have enough hits to run a conformal map, rejecting");
      continue;
    }

    // conformal mapping for track parameters
    auto trkParameters = m_seedFitter->fit(trackCandidate);
    if (trkParameters == nullptr) {
      ATH_MSG_DEBUG("Conformal mapping failed");
      continue;
    }

    Trk::ParticleHypothesis matEffects = Trk::pion;
    // first fit the track with local parameters and without outlier removal.
    std::unique_ptr<Trk::Track> track =
        m_trackFitter->fit(ctx, clusters, *trkParameters, false, matEffects);

    if (track == nullptr || track->perigeeParameters() == nullptr) {
      ATH_MSG_DEBUG("Track " << trackCounter
                             << " fails the first chi2 fit, skipping");
      continue;
    }
      // fit the track again with perigee parameters and without outlier
      // removal.
      track = m_trackFitter->fit(ctx, clusters, *track->perigeeParameters(),
                                 false, matEffects);
    if (track == nullptr || track->perigeeParameters() == nullptr) {
      ATH_MSG_DEBUG("Track " << trackCounter
                             << " fails the second chi2 fit, skipping");
      continue;
    }
        // finally fit with outlier removal
    track = m_trackFitter->fit(ctx, clusters, *track->perigeeParameters(), true,
                               matEffects);
    if (track == nullptr || track->perigeeParameters() == nullptr) {
      ATH_MSG_DEBUG("Track " << trackCounter
                             << " fails the third chi2 fit, skipping");
      continue;
    }

    // compute pT and skip if pT too low
    const Trk::Perigee* origPerigee = track->perigeeParameters();
    double pt = std::hypot(origPerigee->momentum().x(), origPerigee->momentum().y());
    ATH_MSG_DEBUG("Track " << trackCounter << " has pT: " << pt);
    if (pt < 500) {
      ATH_MSG_DEBUG("Track " << trackCounter
                             << " has pT too low, skipping track!");
      continue;
    }
    // need to compute track summary here. This is done during ambiguity 
    // resolution in the legacy chain. Since we skip it, we must do it here
    m_trackSummaryTool->computeAndReplaceTrackSummary(
        *track, false /* DO NOT suppress hole search*/);
    outputTracks->push_back(track.release());
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
