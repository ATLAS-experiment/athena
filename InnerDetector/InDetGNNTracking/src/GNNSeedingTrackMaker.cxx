/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "GNNSeedingTrackMaker.h"

#include <algorithm>
#include <fstream>
#include <memory>
#include <unordered_set>

#include "SiSPSeededTrackFinderData/SiDetElementRoadMakerData_xk.h"
#include "TrkPrepRawData/PrepRawData.h"

InDet::GNNSeedingTrackMaker::GNNSeedingTrackMaker(const std::string& name,
                                                  ISvcLocator* pSvcLocator)
    : AthReentrantAlgorithm(name, pSvcLocator) {}

StatusCode InDet::GNNSeedingTrackMaker::initialize() {
  ATH_CHECK(m_SpacePointsPixelKey.initialize());
  ATH_CHECK(m_SpacePointsStripKey.initialize());

  ATH_CHECK(m_pixelClusterKey.initialize());
  ATH_CHECK(m_stripClusterKey.initialize());
  ATH_CHECK(m_boundaryPixelKey.initialize());
  ATH_CHECK(m_boundaryStripKey.initialize());

  ATH_CHECK(m_fieldCondObjInputKey.initialize());
  ATH_CHECK(m_pixelDetElStatus.initialize(!m_pixelDetElStatus.empty()));
  ATH_CHECK(m_stripDetElStatus.initialize(!m_stripDetElStatus.empty()));

  ATH_CHECK(m_outputTracksKey.initialize());

  ATH_CHECK(m_seedFitter.retrieve());
  ATH_CHECK(m_trackFitter.retrieve());
  ATH_CHECK(m_roadmaker.retrieve());

  ATH_CHECK(m_proptool.retrieve());
  ATH_CHECK(m_updatortool.retrieve());
  ATH_CHECK(m_riocreator.retrieve());

  ATH_CHECK(m_pixelCondSummaryTool.retrieve(DisableTool{
      (!m_pixelDetElStatus.empty() && !VALIDATE_STATUS_ARRAY_ACTIVATED)}));
  ATH_CHECK(m_stripCondSummaryTool.retrieve(DisableTool{
      (!m_stripDetElStatus.empty() && !VALIDATE_STATUS_ARRAY_ACTIVATED)}));
  ATH_CHECK(m_boundaryCheckTool.retrieve());

  magneticFieldInit();

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

  return StatusCode::SUCCESS;
}

void InDet::GNNSeedingTrackMaker::magneticFieldInit() {
  // Build MagneticFieldProperties
  //
  if (m_fieldmode == "NoField")
    m_fieldprop = Trk::MagneticFieldProperties(Trk::NoField);
  else if (m_fieldmode == "MapSolenoid")
    m_fieldprop = Trk::MagneticFieldProperties(Trk::FastField);
  else
    m_fieldprop = Trk::MagneticFieldProperties(Trk::FullField);
}

StatusCode InDet::GNNSeedingTrackMaker::execute(const EventContext& ctx) const {
  SG::WriteHandle<TrackCollection> outputTracks{m_outputTracksKey, ctx};
  ATH_CHECK(outputTracks.record(std::make_unique<TrackCollection>()));

  // get event info
  uint32_t runNumber = ctx.eventID().run_number();
  uint32_t eventNumber = ctx.eventID().event_number();

  std::vector<const Trk::SpacePoint*> spacePoints;

  auto getSpacepointData =
      [&](const SG::ReadHandleKey<SpacePointContainer>& containerKey) {
        if (not containerKey.empty()) {

          SG::ReadHandle<SpacePointContainer> container{containerKey, ctx};

          if (container.isValid()) {
            for (auto spCollection : *container.cptr()) {
              for (const Trk::SpacePoint* spacepoint : *spCollection) {
                spacePoints.push_back(spacepoint);
              }
            }
          }
        }
      };

  getSpacepointData(m_SpacePointsPixelKey);
  getSpacepointData(m_SpacePointsStripKey);

  SG::ReadHandle<InDet::PixelClusterContainer> pixelClusters{m_pixelClusterKey,
                                                             ctx};
  SG::ReadHandle<InDet::SCT_ClusterContainer> stripClusters{m_stripClusterKey,
                                                            ctx};
  ATH_MSG_DEBUG("Found " << pixelClusters->size() << " pixel clusters");
  ATH_MSG_DEBUG("Found " << stripClusters->size() << " strip clusters");
  ATH_MSG_DEBUG("Found " << spacePoints.size() << " space points");

  std::vector<std::vector<uint32_t>> gnnTrackCandidates;
  if (m_gnnTrackFinder.isSet()) {
    ATH_CHECK(m_gnnTrackFinder->getTracks(spacePoints, gnnTrackCandidates));
  } else if (m_gnnTrackReader.isSet()) {
    m_gnnTrackReader->getTracks(runNumber, eventNumber, gnnTrackCandidates);
  } else {
    ATH_MSG_ERROR("Both GNNTrackFinder and GNNTrackReader are not set");
    return StatusCode::FAILURE;
  }

  ATH_MSG_DEBUG("Obtained " << gnnTrackCandidates.size() << " Tracks");

  // loop over all track candidates
  // and perform track fitting for each.
  int trackCounter = -1;
  SiCombinatorialTrackFinderData_xk data;
  if (not data.isInitialized())
    initializeCombinatorialData(ctx, data);
  // Erase statistic information
  //
  data.inputseeds() = 0;
  data.goodseeds() = 0;
  data.inittracks() = 0;
  data.findtracks() = 0;
  data.roadbug() = 0;
  // Set track info
  //
  data.trackinfo().setPatternRecognitionInfo(Trk::TrackInfo::SiSPSeededFinder);
  data.setCosmicTrack(0);
  data.setPixContainer(pixelClusters.cptr());
  data.setSctContainer(stripClusters.cptr());

  getTrackQualityCuts(data);

  // Get AtlasFieldCache
  MagField::AtlasFieldCache fieldCache;

  /// read the B-field cache
  SG::ReadCondHandle<AtlasFieldCacheCondObj> readHandle{m_fieldCondObjInputKey,
                                                        ctx};
  const AtlasFieldCacheCondObj* fieldCondObj{*readHandle};
  if (fieldCondObj == nullptr) {
    ATH_MSG_ERROR(
        "InDet::SiTrackMaker_xk::getTracks: Failed to retrieve "
        "AtlasFieldCacheCondObj with key "
        << m_fieldCondObjInputKey.key());
    return StatusCode::FAILURE;
  }
  fieldCondObj->getInitializedCache(fieldCache);

  int num_extended_tracks = 0;

  for (auto& trackIndices : gnnTrackCandidates) {

    trackCounter++;
    std::vector<const Trk::PrepRawData*> clusters;  // only pixel clusters!
    std::vector<const Trk::SpacePoint*> trackCandidate;
    trackCandidate.reserve(trackIndices.size());

    for (auto& id : trackIndices) {
      //// for each spacepoint, attach all prepRawData to a list.
      if (id > spacePoints.size()) {
        ATH_MSG_WARNING("SpacePoint index out of range");
        continue;
      }
      const Trk::SpacePoint* sp = spacePoints[id];
      if (sp != nullptr) {
        trackCandidate.push_back(sp);
        clusters.push_back(sp->clusterList().first);
      }
    }

    // I will follow the following steps to fit the track:
    // 1. get initial track parameters based on these space points
    // 2. fit the track segments to get a better estimate of the track
    // parameters, twice. (first with local parameters, then with perigee
    // parameters)
    // 3. create the detector element road based on the track parameters.
    // 4. forward extension of the track with smoothing
    // 5. backward smoother of the track
    // 6. create a Trk::Track object and store it in the outputTracks
    // collection. If step 3 - 6 failed, I will refit the track from step 2 with
    // overlap removal and store it in the outputTracks collection.

    // other options we could explore. See SiTrajectory_xk.h for more details.
    // backwardExtension, forwardFilter, filterWithPreciseClustersError,

    // conformal mapping for track parameters
    auto trkParameters = m_seedFitter->fit(trackCandidate);
    if (trkParameters == nullptr) {
      ATH_MSG_WARNING("Conformal mapping failed");
      continue;
    }

    // step 2. fit the pixel-based track with the track fitter
    Trk::ParticleHypothesis matEffects = Trk::pion;
    // first fit the track with local parameters and without outlier removal.
    bool outlier_removal = false;
    std::unique_ptr<Trk::Track> track = m_trackFitter->fit(
        ctx, clusters, *trkParameters, outlier_removal, matEffects);
    if (track != nullptr && track->perigeeParameters() != nullptr) {
      // fit the track again with perigee parameters and without outlier
      // removal.
      track = m_trackFitter->fit(ctx, clusters, *track->perigeeParameters(),
                                 outlier_removal, matEffects);
    }
    // done with step 2.

    if (track == nullptr)
      continue;
    const Trk::TrackParameters& Tp = *(track->perigeeParameters());
    bool is_extension_successful = false;

    // step 3. building the road
    SiDetElementRoadMakerData_xk roadMakerData;
    std::vector<const InDetDD::SiDetectorElement*> trackRoad;
    m_roadmaker->detElementsRoad(ctx, fieldCache, Tp, Trk::alongMomentum,
                                 trackRoad, roadMakerData);
    if (!trackRoad.empty()) {
      std::vector<const InDet::SiDetElementBoundaryLink_xk*> DEL;
      detectorElementLinks(trackRoad, DEL, ctx);

      data.tools().setBremNoise(false, false);
      data.tracks().erase(data.tracks().begin(), data.tracks().end());
      data.statistic().fill(false);
      ++data.inputseeds();

      // step 3.5 initialize the trajectory
      std::vector<const InDet::SiCluster*> Cl;
      spacePointsToClusters(trackCandidate, Cl);
      bool Qr;
      bool Q = data.trajectory().initialize(true, true, pixelClusters.cptr(),
                                            stripClusters.cptr(), Tp, Cl, DEL,
                                            Qr, ctx);

      if (Q) {
        // step 4. forward extension of the track
        int itmax = 10;
        bool do_smooth = true;
        if (data.trajectory().forwardExtension(do_smooth, itmax, ctx)) {
          // step 5. backward smoother of the track
          if (data.trajectory().backwardSmoother(false, ctx)) {
            /// sort in step order
            data.trajectory().sortStep();

            Trk::TrackInfo info;
            info.setPatternRecognitionInfo(
                Trk::TrackInfo::SiSPSeededFinderSimple);
            info.setParticleHypothesis(Trk::pion);

            Trk::Track final_track(
                info,
                std::make_unique<Trk::TrackStates>(
                    data.trajectory().convertToSimpleTrackStateOnSurface(ctx)),
                data.trajectory().convertToFitQuality());
            // refit the track with overlap removal.
            auto extended_track =
                m_trackFitter->fit(ctx, final_track, true, matEffects);
            if (extended_track != nullptr &&
                extended_track->trackSummary() != nullptr) {
              num_extended_tracks++;
              is_extension_successful = true;
              outputTracks->push_back(extended_track.release());
            }
          }
        }  // end of forward extension. We could try backward smoother if
           // forward extension failed.
      }
    }
    if (!is_extension_successful) {
      track = m_trackFitter->fit(ctx, clusters, *track->perigeeParameters(),
                                 true, matEffects);
      if (track != nullptr && track->trackSummary() != nullptr) {
        outputTracks->push_back(track.release());
      }
    }
  }

  data.tracks().erase(data.tracks().begin(), data.tracks().end());
  ATH_MSG_DEBUG("Run " << runNumber << ", Event " << eventNumber << " has "
                       << outputTracks->size() << " tracks stored, with "
                       << num_extended_tracks << " extended.");
  return StatusCode::SUCCESS;
}

///////////////////////////////////////////////////////////////////
// Convert space points to clusters and (for Run 4) detector elements
///////////////////////////////////////////////////////////////////

bool InDet::GNNSeedingTrackMaker::spacePointsToClusters(
    const std::vector<const Trk::SpacePoint*>& Sp,
    std::vector<const InDet::SiCluster*>& Sc,
    std::optional<
        std::reference_wrapper<std::vector<const InDetDD::SiDetectorElement*>>>
        DE) {
  Sc.reserve(Sp.size());
  /// loop over all SP
  for (const Trk::SpacePoint* s : Sp) {
    /// get the first cluster on an SP
    const Trk::PrepRawData* p = s->clusterList().first;
    if (p) {
      /// add to list
      const InDet::SiCluster* c = static_cast<const InDet::SiCluster*>(p);
      if (c) {
        Sc.push_back(c);
      }
    }
    /// for strips, also make sure to pick up the second one!
    p = s->clusterList().second;
    if (p) {
      const InDet::SiCluster* c = static_cast<const InDet::SiCluster*>(p);
      if (c) {
        Sc.push_back(c);
      }
    }
  }

  ///  Detector elments test
  std::vector<const InDet::SiCluster*>::iterator cluster = Sc.begin(),
                                                 nextCluster,
                                                 endClusters = Sc.end();

  /// here we reject cases where two subsequent clusters are on the same
  /// detector element
  if (DE) {
    DE->get().reserve(Sc.size());
  }
  for (; cluster != endClusters; ++cluster) {

    const InDetDD::SiDetectorElement* de = (*cluster)->detectorElement();

    nextCluster = cluster;
    ++nextCluster;
    for (; nextCluster != endClusters; ++nextCluster) {
      if (de == (*nextCluster)->detectorElement()) {
        return false;
      }
    }
    if (DE) {
      DE->get().push_back(de);
    }
  }
  return true;
}

///////////////////////////////////////////////////////////////////
// Convert detector elements to detector element links
///////////////////////////////////////////////////////////////////

void InDet::GNNSeedingTrackMaker::detectorElementLinks(
    std::vector<const InDetDD::SiDetectorElement*>& DE,
    std::vector<const InDet::SiDetElementBoundaryLink_xk*>& DEL,
    const EventContext& ctx) const {
  const InDet::SiDetElementBoundaryLinks_xk* boundaryPixel{nullptr};
  const InDet::SiDetElementBoundaryLinks_xk* boundaryStrip{nullptr};

  SG::ReadCondHandle<InDet::SiDetElementBoundaryLinks_xk> boundaryPixelHandle(
      m_boundaryPixelKey, ctx);
  boundaryPixel = *boundaryPixelHandle;
  if (boundaryPixel == nullptr) {
    ATH_MSG_FATAL(m_boundaryPixelKey.fullKey() << " returns null pointer");
  }

  SG::ReadCondHandle<InDet::SiDetElementBoundaryLinks_xk> boundaryStripHandle(
      m_boundaryStripKey, ctx);
  boundaryStrip = *boundaryStripHandle;
  if (boundaryStrip == nullptr) {
    ATH_MSG_FATAL(m_boundaryStripKey.fullKey() << " returns null pointer");
  }

  DEL.reserve(DE.size());
  for (const InDetDD::SiDetectorElement* d : DE) {
    IdentifierHash id = d->identifyHash();
    if (d->isPixel() && boundaryPixel && id < boundaryPixel->size())
      DEL.push_back(&(*boundaryPixel)[id]);
    else if (d->isSCT() && boundaryStrip && id < boundaryStrip->size())
      DEL.push_back(&(*boundaryStrip)[id]);
  }
}

void InDet::GNNSeedingTrackMaker::initializeCombinatorialData(
    const EventContext& ctx, SiCombinatorialTrackFinderData_xk& data) const {

  /// Add conditions object to SiCombinatorialTrackFinderData to be able to
  /// access the field cache for each new event Get conditions object for
  /// field cache
  SG::ReadCondHandle<AtlasFieldCacheCondObj> readHandle{m_fieldCondObjInputKey,
                                                        ctx};
  const AtlasFieldCacheCondObj* fieldCondObj{*readHandle};
  if (fieldCondObj == nullptr) {
    std::string msg =
        "InDet::SiCombinatorialTrackFinder_xk::initializeCombinatorialData: "
        "Failed to retrieve AtlasFieldCacheCondObj with key " +
        m_fieldCondObjInputKey.key();
    throw(std::runtime_error(msg));
  }
  data.setFieldCondObj(fieldCondObj);

  /// Must have set fieldCondObj BEFORE calling setTools because fieldCondObj
  /// is used there
  data.setTools(&*m_proptool, &*m_updatortool, &*m_riocreator,
                (m_pixelDetElStatus.empty() || VALIDATE_STATUS_ARRAY_ACTIVATED)
                    ? &*m_pixelCondSummaryTool
                    : nullptr,
                (m_stripDetElStatus.empty() || VALIDATE_STATUS_ARRAY_ACTIVATED)
                    ? &*m_stripCondSummaryTool
                    : nullptr,
                &m_fieldprop, &*m_boundaryCheckTool);
  if (!m_pixelDetElStatus.empty()) {
    SG::ReadHandle<InDet::SiDetectorElementStatus> pixelDetElStatus(
        m_pixelDetElStatus, ctx);
    data.setPixelDetectorElementStatus(pixelDetElStatus.cptr());
  }
  if (!m_stripDetElStatus.empty()) {
    SG::ReadHandle<InDet::SiDetectorElementStatus> stripDetElStatus(
        m_stripDetElStatus, ctx);
    data.setSCTDetectorElementStatus(stripDetElStatus.cptr());
  }

  // Set the ITk Geometry setup
  data.setITkGeometry(true);
  // Set the ITk Fast Tracking setup
  data.setFastTracking(false);
}

void InDet::GNNSeedingTrackMaker::getTrackQualityCuts(
    SiCombinatorialTrackFinderData_xk& data) const {
    
  data.setCosmicTrack(0);
  data.setNclusmin(m_nclusmin);
  data.setNclusminb(std::max(3, data.nclusmin() - 1));
  data.setNwclusmin(m_nwclusmin);
  data.setNholesmax(m_nholesmax);
  data.setDholesmax(m_dholesmax);

  data.tools().setHolesClusters(data.nholesmax(), data.dholesmax(),
                                data.nclusmin());

  data.tools().setAssociation(0);
  data.setSimpleTrack(false);

  data.setPTmin(m_pTmin);
  data.setPTminBrem(m_pTminBrem);
  data.setXi2max(m_xi2max);
  data.setXi2maxNoAdd(m_xi2maxNoAdd);
  data.setXi2maxlink(m_xi2maxlink);
  data.tools().setXi2pTmin(data.xi2max(), data.xi2maxNoAdd(), data.xi2maxlink(),
                           data.pTmin());
  data.tools().setMultiTracks(m_doMultiTracksProd, m_xi2multitracks);

  data.trajectory().setParameters();
}
