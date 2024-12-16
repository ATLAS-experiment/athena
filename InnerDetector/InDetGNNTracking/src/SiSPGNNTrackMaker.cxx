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

  // get all space points from event
  std::vector<const Trk::SpacePoint*> spacePoints = getSpacePointsInEvent(ctx, eventNumber);

  // get all clusters from event
  std::vector<const Trk::PrepRawData*> allClusters = getClustersInEvent(ctx, eventNumber);

  // get tracks from GNN chain
  std::vector<std::vector<uint32_t> > TT;
  std::vector<std::vector<uint32_t> > clusterTracks;
  if (m_gnnTrackFinder.isSet()) {
    ATH_CHECK(m_gnnTrackFinder->getTracks(spacePoints, TT));
  } else if (m_gnnTrackReader.isSet()) {
    // if track candidates are built from cluster, get both clusters and SPs
    m_areInputClusters ? 
      m_gnnTrackReader->getTracks(runNumber, eventNumber, clusterTracks, TT) :
      m_gnnTrackReader->getTracks(runNumber, eventNumber, TT); 
  } else {
    ATH_MSG_ERROR("Both GNNTrackFinder and GNNTrackReader are not set");
    return StatusCode::FAILURE;
  }

  ATH_MSG_DEBUG("Event " << eventNumber << " obtained " << TT.size() << " Tracks");

  // loop over all track candidates
  // and perform track fitting for each.
  int trackCounter = -1;
  std::vector<int> status_codes;
  // track processing loop
  for (auto& trackIndices : TT) {
    // For each track candidate:
    trackCounter++;

    // 1. Sort space points by distance from origin
    std::vector<const Trk::SpacePoint*> trackCandidate = getSpacePoints(trackIndices, spacePoints);

    // 2. Get associated clusters
    // if track candidates are built from cluster, get both clusters and SPs
    std::vector<const Trk::PrepRawData*> clusters = m_areInputClusters ? 
      getClusters (clusterTracks, allClusters, trackCounter) 
      : spacePointsToClusters(trackCandidate) ;

    // 3. Perform track fitting:
    //    - Initial conformal mapping
    //    - First chi2 fit without outlier removal
    //    - Second chi2 fit with perigee parameters
    //    - Final fit with outlier removal
    // 4. Apply quality cuts (pT, eta)
    // 5. Compute track summary
    // 6. Store track if it passes all criteria
    auto [fitSuccess, passTrackCut, track] = doFitAndCut(ctx, trackCandidate, clusters, trackCounter);

    if (not fitSuccess) {
      continue;
    }

    if (passTrackCut <= 0) {
      outputTracks->push_back(track.release());
    }

    status_codes.push_back(passTrackCut);

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


bool InDet::SiSPGNNTrackMaker::prefitCheck(int nPix, int nStrip, int nClusters, int nSpacePoints) const {
  return nPix >= m_minPixelClusters && nStrip >= m_minStripClusters && nClusters >= m_minClusters && nSpacePoints >= 3;
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

std::vector<const Trk::SpacePoint*> InDet::SiSPGNNTrackMaker::getSpacePointsInEvent (
  const EventContext& ctx,
  int eventNumber
) const {
  std::vector<const Trk::SpacePoint*> spacePoints;

  int npixsp(0), nstrip(0), n_overlap(0);
  for (const Trk::SpacePoint* sp : getSpacePointsInEvent(ctx, m_SpacePointsPixelKey)) {
    spacePoints.push_back(sp);
    npixsp++;
  }
  for (const Trk::SpacePoint* sp : getSpacePointsInEvent(ctx, m_SpacePointsSCTKey)) {
    spacePoints.push_back(sp);
    nstrip++;
  }
  for (const Trk::SpacePoint* sp : getSpacePointsInEvent(ctx, m_SpacePointsOverlapKey)) {
    spacePoints.push_back(sp);
    n_overlap++;
  }
  ATH_MSG_DEBUG("Event " << eventNumber << " has " << npixsp
                         << " pixel space points, " << nstrip 
                         << " strips space points" << n_overlap
                         << " overlapping spacepoints" << spacePoints.size()
                          << " space points");

  return spacePoints;
}

std::vector<const Trk::SpacePoint*> InDet::SiSPGNNTrackMaker::getSpacePointsInEvent(
  const EventContext& ctx,
  const SG::ReadHandleKey<SpacePointContainer>& containerKey
) const {
  std::vector<const Trk::SpacePoint*> spacePoints;
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

  return spacePoints;
}

std::vector<const Trk::SpacePoint*> InDet::SiSPGNNTrackMaker::getSpacePointsInEvent(
  const EventContext& ctx,
  const SG::ReadHandleKey<SpacePointOverlapCollection>& containerKey
) const {

  std::vector<const Trk::SpacePoint*> spacePoints;

  if (not containerKey.empty()){

    SG::ReadHandle<SpacePointOverlapCollection> collection{containerKey, ctx};

    if (collection.isValid()){
      for (const Trk::SpacePoint *sp : *collection) {
        spacePoints.push_back(sp);
      }
    }
  }

  return spacePoints;
}

std::vector<const Trk::PrepRawData*> InDet::SiSPGNNTrackMaker::getClustersInEvent (
  const EventContext& ctx,
  int eventNumber
) const {
  std::vector<const Trk::PrepRawData*> allClusters;
  if (m_areInputClusters) {
    SG::ReadHandle<InDet::PixelClusterContainer> pixcontainer(m_ClusterPixelKey,
                                                            ctx);
    SG::ReadHandle<InDet::SCT_ClusterContainer> strip_container(m_ClusterStripKey,
                                                            ctx);

    if (!pixcontainer.isValid()) {
      ATH_MSG_ERROR("Pixel container invalid, returning");
      return allClusters;
    }

    if (!strip_container.isValid()) {
      ATH_MSG_ERROR("Strip container invalid, returning");
      return allClusters;
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
  return allClusters;
}
 

std::vector<const Trk::SpacePoint*> InDet::SiSPGNNTrackMaker::getSpacePoints (
  std::vector<uint32_t> trackIndices,
  std::vector<const Trk::SpacePoint*> allSpacePoints
) const {

  std::vector<const Trk::SpacePoint*> trackCandidate;
  trackCandidate.reserve(trackIndices.size());

  std::vector<std::pair<double, const Trk::SpacePoint*> > distanceSortedSPs;

  // get track space points
  // sort SPs in track by distance from origin
  for (auto& id : trackIndices) {
    //// for each spacepoint, attach all prepRawData to a list.
    if (id > allSpacePoints.size()) {
      ATH_MSG_WARNING("SpacePoint index "
                      << id << " out of range: " << allSpacePoints.size());
      continue;
    }

    const Trk::SpacePoint* sp = allSpacePoints[id];

    // store distance - hit paire
    if (sp != nullptr) {
      distanceSortedSPs.push_back(
        std::make_pair(
          pow(sp->globalPosition().x(), 2) + pow(sp->globalPosition().y(), 2),
          sp
        )
      );
    }
  }

  // sort by distance
  std::sort(distanceSortedSPs.begin(), distanceSortedSPs.end());

  // add SP to trk candidate in the same order
  for (size_t i = 0; i < distanceSortedSPs.size(); i++) {
    trackCandidate.push_back(distanceSortedSPs[i].second);
  }

  return trackCandidate;
}

std::vector<const Trk::PrepRawData*> InDet::SiSPGNNTrackMaker :: spacePointsToClusters (
  std::vector<const Trk::SpacePoint*> spacePoints
) const {
  std::vector<const Trk::PrepRawData*> clusters;
  for (const Trk::SpacePoint* sp : spacePoints) {
    clusters.push_back(sp->clusterList().first);
    if (sp->clusterList().second != nullptr) {
      clusters.push_back(sp->clusterList().second);
    }
  }
  return clusters;
}

std::vector<const Trk::PrepRawData*> InDet::SiSPGNNTrackMaker :: getClusters (
  std::vector<std::vector<uint32_t>> clusterTracks,
  std::vector<const Trk::PrepRawData*> allClusters,
  int trackNumber
) const {
  std::vector<uint32_t> clusterIndices = clusterTracks[trackNumber];
  std::vector<const Trk::PrepRawData*> clusters;
  clusters.reserve(clusterIndices.size());
  for (uint32_t id : clusterIndices) {
    if (id > allClusters.size()) {
      ATH_MSG_ERROR("Cluster index out of range");
      continue;
    }
    clusters.push_back(allClusters[id]);
  }
  return clusters;
}

/**
 * @brief Fits a track and applies quality cuts to determine if it should be kept
 *
 * @param ctx           Event context for conditions access
 * @param clusters      Vector of cluster measurements used to fit the track
 * @param initial_params Initial track parameters from seed finding
 * @param trackCounter  Index/ID of the track candidate being processed
 *
 * @return std::tuple<bool, int, std::unique_ptr<Trk::Track>>
 *         - bool: Whether the fit was successful
 *         - int: Error/status code (0 for success, >0 for specific failure modes)
 *         - Track: The fitted track (nullptr if fit fails or track rejected)
 *
 * This function performs the following steps:
 * 1. Fits the track using the provided clusters and initial parameters
 * 2. Checks basic kinematic cuts (pT, eta)
 * 3. Computes track summary (hits, holes)
 * 4. Applies eta-dependent quality cuts if configured
 *
 * Error codes:
 * - 0: Success, track passes all cuts
 * - 1: Failed minimum silicon hits requirement
 * - 2: Failed minimum pixel hits requirement
 * - 3: Failed minimum pT requirement
 * - 4: Failed maximum z0 requirement
 * - 5: Failed maximum d0 requirement
 * - 6: Failed maximum holes requirement
 * - 999: Track fit failed
 *
 * @note The track fitting is performed using the configured track fitter tool
 * @note Quality cuts are configurable via job options
 */

std::tuple<bool, int, std::unique_ptr<Trk::Track>> InDet::SiSPGNNTrackMaker::doFitAndCut (
    const EventContext& ctx,
    std::vector<const Trk::SpacePoint*>& spacePoints,
    std::vector<const Trk::PrepRawData*>& clusters,
    int& trackCounter
  ) const 
  {
    // get cluster list
    int nPIX(0), nStrip(0);

    for (const Trk::PrepRawData* cl : clusters) {
      if (cl->type(Trk::PrepRawDataType::PixelCluster)) nPIX++;
      if (cl->type(Trk::PrepRawDataType::SCT_Cluster)) nStrip++;
    }

    ATH_MSG_DEBUG("Track " << trackCounter << " has " << spacePoints.size()
                           << " space points, " << clusters.size()
                           << " clusters, " << nPIX << " pixel clusters, "
                           << nStrip << " strip clusters");

    // check hit counts
    if (not prefitCheck(nPIX, nStrip, clusters.size(), spacePoints.size())) {
      ATH_MSG_DEBUG("Track " << trackCounter << " does not pass prefit cuts, skipping");
      return std::make_tuple(false, 999, nullptr);
    }

    // conformal mapping for track parameters
    auto trkParameters = m_seedFitter->fit(spacePoints);
    if (trkParameters == nullptr) {
      ATH_MSG_DEBUG("Conformal mapping failed");
      return std::make_tuple(false, 999, nullptr);
    }

    std::unique_ptr<Trk::Track> track = fitTrack(ctx, clusters, *trkParameters, trackCounter);

    if (track == nullptr || track->perigeeParameters() == nullptr) {
      ATH_MSG_DEBUG("Track " << trackCounter
                             << " fails the third chi2 fit, skipping");
      return std::make_tuple(false, 999, nullptr);
    }

    // compute pT and skip if pT too low
    if (track->perigeeParameters()->pT() < m_pTmin) {
      ATH_MSG_DEBUG("Track " << trackCounter
                             << "with pt = " << track->perigeeParameters()->pT()
                             << " has pT too low, skipping track!");
      return std::make_tuple(false, 999, nullptr);
    }

    // get rid of tracks with eta too large
    if (std::abs(track->perigeeParameters()->eta()) > m_etamax) {
      ATH_MSG_DEBUG("Track " << trackCounter << "with eta = "
                             << std::abs(track->perigeeParameters()->eta())
                             << " has eta too high, skipping track!");
      return std::make_tuple(false, 999, nullptr);
    }

    // if track fit succeeds, eta and pT within range, compute track summary. This is quite expensive.
    m_trackSummaryTool->computeAndReplaceTrackSummary(
        *track, false /* DO NOT suppress hole search*/);
    
    int passTrackCut = (m_doRecoTrackCuts) ? passEtaDepCuts(*track) : -1;
    
    return std::make_tuple(true, passTrackCut, std::move(track));
    
  }

std::unique_ptr<Trk::Track> InDet::SiSPGNNTrackMaker::fitTrack (
    const EventContext& ctx,
    std::vector<const Trk::PrepRawData*> clusters,
    const Trk::TrackParameters& initial_params,
    int trackCounter
  ) const 
  {

    Trk::ParticleHypothesis matEffects = Trk::pion;
    // first fit the track with local parameters and without outlier removal.
    std::unique_ptr<Trk::Track> track =
        m_trackFitter->fit(ctx, clusters, initial_params, false, matEffects);
    
    if (track == nullptr || track->perigeeParameters() == nullptr) {
      ATH_MSG_DEBUG("Track " << trackCounter
                             << " fails the first chi2 fit, skipping");
      return track;
    }

    // reject track with pT too low, default 400 MeV
    if (track->perigeeParameters()->pT() < m_pTmin) {
      ATH_MSG_DEBUG("Track " << trackCounter
                             << " fails the first chi2 fit, skipping");
      return nullptr;
    }

    // fit the track again with perigee parameters and without outlier
    // removal.
    track = m_trackFitter->fit(ctx, clusters, *track->perigeeParameters(),
                                false, matEffects);

    // track = m_trackFitter->fit(ctx, *track, false, matEffects);
    if (track == nullptr || track->perigeeParameters() == nullptr) {
      ATH_MSG_DEBUG("Track " << trackCounter
                             << " fails the second chi2 fit, skipping");
      return track;
    }
    // finally fit with outlier removal
    // track = m_trackFitter->fit(ctx, clusters, *track->perigeeParameters(), true,
    //                            matEffects);

    return m_trackFitter->fit(ctx, clusters, *track->perigeeParameters(), true, matEffects);
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
