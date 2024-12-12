/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include <memory>
#include <fstream>

#include "SiSPGNNTrackMaker.h"

#include "TrkPrepRawData/PrepRawData.h"
#include "TrkRIO_OnTrack/RIO_OnTrack.h"

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
  // retrieve eta dependent cut svc
  ATH_CHECK(m_etaDependentCutsSvc.retrieve());

  ATH_MSG_INFO("Applying the following cuts during GNN-based track reconstruction: ");
  ATH_MSG_INFO("Min pT: " << m_pTmin);
  ATH_MSG_INFO("Max eta: " << m_etamax);
  ATH_MSG_INFO("Min number of clusters: " << m_minClusters);
  ATH_MSG_INFO("Min number of pixel clusters: " << m_minPixelClusters);
  ATH_MSG_INFO("Min number of strip clusters: " << m_minStripClusters);


  if (m_doRecoTrackCuts) {
    ATH_MSG_INFO("Applying the following eta dependant track cuts after GNN-based track reconstruction: ");
    std::vector <double> etaBins, minPT, maxz0, maxd0;
    std::vector <int> minClusters, minPixelHits, maxHoles;
    m_etaDependentCutsSvc->getValue(InDet::CutName::etaBins, etaBins);
    ATH_MSG_INFO("Eta bins: " << etaBins );
    m_etaDependentCutsSvc->getValue(InDet::CutName::minClusters, minClusters);
    ATH_MSG_INFO("Min Si Hits: " << minClusters );
    m_etaDependentCutsSvc->getValue(InDet::CutName::minPixelHits, minPixelHits);
    ATH_MSG_INFO("Min pixel hits: " << minPixelHits );
    m_etaDependentCutsSvc->getValue(InDet::CutName::maxHoles, maxHoles);
    ATH_MSG_INFO("Max holes: " << maxHoles);
    m_etaDependentCutsSvc->getValue(InDet::CutName::minPT, minPT);
    ATH_MSG_INFO("Min pT: " << minPT);
    m_etaDependentCutsSvc->getValue(InDet::CutName::maxZImpact, maxz0);
    ATH_MSG_INFO("Max z0: " << maxz0);
    m_etaDependentCutsSvc->getValue(InDet::CutName::maxPrimaryImpact, maxd0);
    ATH_MSG_INFO("Max d0: " << maxd0);
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
  std::multimap<double, Trk::Track*> qualitySortedTrackCandidates;
  std::vector<int> status_codes;
  // track processing loop
  for (auto& trackIndices : TT) {
    // For each track candidate:
    // 1. Sort space points by distance from origin
    // 2. Get associated clusters
    // 3. Perform track fitting:
    //    - Initial conformal mapping
    //    - First chi2 fit without outlier removal
    //    - Second chi2 fit with perigee parameters
    //    - Final fit with outlier removal
    // 4. Apply quality cuts (pT, eta)
    // 5. Compute track summary
    // 6. Store track if it passes all criteria

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

    if (nPIX < m_minPixelClusters) {
      ATH_MSG_DEBUG("Track " << trackCounter << " does not have enough pixel clusters, rejecting");
      continue;
    }

    if (nStrip < m_minStripClusters) {
      ATH_MSG_DEBUG("Track " << trackCounter << " does not have enough strip clusters, rejecting");
      continue;
    }

    if (static_cast<int>(clusters.size()) < m_minClusters) {
      ATH_MSG_DEBUG("Track " << trackCounter
                             << " does not have enough hits, rejecting");
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

    // reject track with pT too low, default 400 MeV
    if (track->perigeeParameters()->pT() < m_pTmin) {
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
    if (track->perigeeParameters()->pT() < m_pTmin) {
      ATH_MSG_DEBUG("Track " << trackCounter
                             << "with pt = " << track->perigeeParameters()->pT()
                             << " has pT too low, skipping track!");
      continue;
    }

    // get rid of tracks with eta too large
    if (std::abs(track->perigeeParameters()->eta()) > m_etamax) {
      ATH_MSG_DEBUG("Track " << trackCounter << "with eta = "
                             << std::abs(track->perigeeParameters()->eta())
                             << " has eta too high, skipping track!");
      continue;
    }
    // need to compute track summary here. This is done during ambiguity 
    // resolution in the legacy chain. Since we skip it, we must do it here
    m_trackSummaryTool->computeAndReplaceTrackSummary(
        *track, false /* DO NOT suppress hole search*/);

    int passTrackCut = (m_doRecoTrackCuts) ? passEtaDepCuts(*track) : -1;
    status_codes.push_back(passTrackCut);
    if (passTrackCut<=0) {
      outputTracks->push_back(track.release());
    }
  }
  if (m_doRecoTrackCuts) {
    ATH_MSG_INFO("Event " << eventNumber << " has " << status_codes.size() << " tracks found, " 
                << std::count(status_codes.begin(), status_codes.end(), 0)
                << " tracks remains after applying track cuts");
  } else {
    ATH_MSG_INFO("Event " << eventNumber << " has " << status_codes.size() << " tracks found, all tracks are kept");
  }

  return StatusCode::SUCCESS;
}





int InDet::SiSPGNNTrackMaker::passEtaDepCuts(const Trk::Track& track) const 
{
  const Trk::Perigee* origPerigee = track.perigeeParameters();
  double pt = origPerigee->pT();

  double eta = std::abs(origPerigee->eta());

  double d0 = std::abs(origPerigee->parameters()[Trk::d0]);

  double z0 = std::abs(origPerigee->parameters()[Trk::z0]);

  int nHolesOnTrack = track.trackSummary()->get(Trk::numberOfPixelHoles) +
                      track.trackSummary()->get(Trk::numberOfSCTHoles);

  int nPixels = track.trackSummary()->get(Trk::numberOfPixelHits);
  int nStrips = track.trackSummary()->get(Trk::numberOfSCTHits);
  int nClusters = nPixels + nStrips;

  ATH_MSG_DEBUG("track params: " << pt << " " << eta << " " << d0 << " " << z0
                                 << " " << nClusters << nStrips
                                 << " " << nPixels);

  // min Si hits
  if (nClusters < m_etaDependentCutsSvc->getMinSiHitsAtEta(eta))
    return 1;

  // min pixel hits
  if (nPixels < m_etaDependentCutsSvc->getMinPixelHitsAtEta(eta))
    return 2;

  // min pT, default 400
  if (pt < m_etaDependentCutsSvc->getMinPtAtEta(eta))
    return 3;

  // max z0
  if (z0 > m_etaDependentCutsSvc->getMaxZImpactAtEta(eta))
    return 4;

  // max d0
  if (d0 > m_etaDependentCutsSvc->getMaxPrimaryImpactAtEta(eta))
    return 5;

  // max holes
  if (nHolesOnTrack > m_etaDependentCutsSvc->getMaxSiHolesAtEta(eta))
    return 6;

  return 0;
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
