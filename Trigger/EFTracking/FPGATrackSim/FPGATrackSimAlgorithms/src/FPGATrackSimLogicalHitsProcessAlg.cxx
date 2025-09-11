// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#include "FPGATrackSimLogicalHitsProcessAlg.h"

#include "FPGATrackSimObjects/FPGATrackSimCluster.h"
#include "FPGATrackSimObjects/FPGATrackSimHit.h"
#include "FPGATrackSimObjects/FPGATrackSimDataFlowInfo.h"
#include "FPGATrackSimObjects/FPGATrackSimRoad.h"
#include "FPGATrackSimObjects/FPGATrackSimTrack.h"
#include "FPGATrackSimObjects/FPGATrackSimLogicalEventOutputHeader.h"
#include "FPGATrackSimObjects/FPGATrackSimLogicalEventInputHeader.h"
#include "FPGATrackSimObjects/FPGATrackSimTrackPars.h"

#include "FPGATrackSimAlgorithms/FPGATrackSimNNTrackTool.h"
#include "FPGATrackSimAlgorithms/FPGATrackSimOverlapRemovalTool.h"
#include "FPGATrackSimAlgorithms/FPGATrackSimTrackFitterTool.h"

#include "FPGATrackSimConfTools/FPGATrackSimRegionSlices.h"

#include "FPGATrackSimInput/FPGATrackSimRawToLogicalHitsTool.h"
#include "FPGATrackSimInput/FPGATrackSimReadRawRandomHitsTool.h"

#include "FPGATrackSimMaps/FPGATrackSimRegionMap.h"

#include "GaudiKernel/IEventProcessor.h"

constexpr bool enableBenchmark =
#ifdef BENCHMARK_FPGATRACKSIM
    true;
#else
    false;
#endif

///////////////////////////////////////////////////////////////////////////////
// Initialize

FPGATrackSimLogicalHitsProcessAlg::FPGATrackSimLogicalHitsProcessAlg (const std::string& name, ISvcLocator* pSvcLocator) :
    AthAlgorithm(name, pSvcLocator)
{
}


StatusCode FPGATrackSimLogicalHitsProcessAlg::initialize()
{
    std::stringstream ss(m_description);
    std::string line;
    ATH_MSG_INFO("Tag config:");
    if (!m_description.empty()) {
        while (std::getline(ss, line, '\n')) {
            ATH_MSG_INFO('\t' << line);
        }
    }
    ATH_CHECK(m_roadFinderTool.retrieve());
    ATH_CHECK(m_LRTRoadFilterTool.retrieve(EnableTool{m_doLRT}));
    ATH_CHECK(m_LRTRoadFinderTool.retrieve(EnableTool{m_doLRT}));
    ATH_CHECK(m_houghRootOutputTool.retrieve(EnableTool{m_doHoughRootOutput1st}));
    ATH_CHECK(m_NNTrackTool.retrieve(EnableTool{m_doNNTrack}));
    ATH_CHECK(m_roadFilterTool.retrieve(EnableTool{m_filterRoads}));
    ATH_CHECK(m_roadFilterTool2.retrieve(EnableTool{m_filterRoads2}));

    ATH_CHECK(m_spacepointsTool.retrieve(EnableTool{m_doSpacepoints}));

    ATH_CHECK(m_trackFitterTool_1st.retrieve(EnableTool{m_doTracking}));
    ATH_CHECK(m_overlapRemovalTool_1st.retrieve());
    ATH_CHECK(m_writeOutputTool.retrieve());
    ATH_CHECK(m_FPGATrackSimMapping.retrieve());
    ATH_CHECK(m_slicingEngineTool.retrieve());

    ATH_MSG_DEBUG("initialize() Instantiating root objects");

    // ROOT branches created for test vectors.
    if (m_outputRoadUnionTool) m_slicedHitHeader = m_writeOutputTool->addInputBranch(m_sliceBranch.value(), true);
    m_logicEventOutputHeader = m_writeOutputTool->addOutputBranch(m_outputBranch.value(), true);

    // Updated slicing engine test vectors will have three streams.
    m_slicedFirstPixelHeader = m_writeOutputTool->addInputBranch(m_sliceFirstPixelBranch.value(), true);
    m_slicedSecondPixelHeader = m_writeOutputTool->addInputBranch(m_sliceSecondPixelBranch.value(), true);
    m_slicedStripHeader = m_writeOutputTool->addInputBranch(m_sliceStripBranch.value(), true);

    // We also need a pre- and post- SP copy of the SPs.
    m_slicedStripHeaderPreSP = m_writeOutputTool->addInputBranch(m_sliceStripBranchPreSP.value(), true);

    // Connect the slicing tools accordingly. We probably no longer need to hook up the roadfinder here.
    if (m_outputRoadUnionTool) m_roadFinderTool->setupSlices(m_slicedHitHeader);
    m_slicingEngineTool->setupSlices(m_slicedFirstPixelHeader, m_slicedSecondPixelHeader, m_slicedStripHeader);

    ATH_MSG_DEBUG("initialize() Setting branch");

    if (!m_monTool.empty())
        ATH_CHECK(m_monTool.retrieve());

    ATH_CHECK( m_FPGASpacePointsKey.initialize() );
    ATH_CHECK( m_FPGAHitInRoadsKey.initialize() );
    ATH_CHECK( m_FPGAHitFilteredKey.initialize() );
    ATH_CHECK( m_FPGARoadKey.initialize() );
    ATH_CHECK( m_FPGATrackKey.initialize() );
    ATH_CHECK( m_FPGAHitKey.initialize() );
    ATH_CHECK( m_FPGAHitKey_2nd.initialize() );
    ATH_CHECK( m_FPGATruthTrackKey.initialize() );
    ATH_CHECK( m_FPGAOfflineTrackKey.initialize() );
    ATH_CHECK( m_FPGAEventInfoKey.initialize() );

    ATH_CHECK( m_chrono.retrieve() );
    ATH_MSG_DEBUG("initialize() Finished");

    return StatusCode::SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
//                          MAIN EXECUTE ROUTINE                             //
///////////////////////////////////////////////////////////////////////////////

StatusCode FPGATrackSimLogicalHitsProcessAlg::execute()
{
    const EventContext& ctx = getContext();

    // Get reference to hits from StoreGate.
    SG::ReadHandle<FPGATrackSimHitCollection> FPGAHits(m_FPGAHitKey, ctx);
    if (!FPGAHits.isValid()) {
        if (m_evt == 0) {
            ATH_MSG_WARNING("Didn't receive " << FPGAHits.key() << " on first event; assuming no input events.");
        }
        SmartIF<IEventProcessor> appMgr{service("ApplicationMgr")};
        if (!appMgr) {
            ATH_MSG_ERROR("Failed to retrieve ApplicationMgr as IEventProcessor");
            return StatusCode::FAILURE;
        }
        return appMgr->stopRun();
    }

    // Set up write handles.
    SG::WriteHandle<FPGATrackSimHitCollection> FPGAHits_2nd (m_FPGAHitKey_2nd,ctx);
    SG::WriteHandle<FPGATrackSimRoadCollection> FPGARoads_1st (m_FPGARoadKey, ctx);
    SG::WriteHandle<FPGATrackSimHitContainer> FPGAHitsInRoads_1st (m_FPGAHitInRoadsKey, ctx);

    ATH_CHECK( FPGAHits_2nd.record (std::make_unique<FPGATrackSimHitCollection>()));
    ATH_CHECK( FPGARoads_1st.record (std::make_unique<FPGATrackSimRoadCollection>()));
    ATH_CHECK( FPGAHitsInRoads_1st.record (std::make_unique<FPGATrackSimHitContainer>()));

    SG::WriteHandle<FPGATrackSimTrackCollection> FPGATracks_1stHandle (m_FPGATrackKey, ctx);
    ATH_CHECK(FPGATracks_1stHandle.record (std::make_unique<FPGATrackSimTrackCollection>()));

    SG::WriteHandle<FPGATrackSimHitCollection> FPGAHitsFiltered_1st (m_FPGAHitFilteredKey, ctx);
    ATH_CHECK( FPGAHitsFiltered_1st.record (std::make_unique<FPGATrackSimHitCollection>()));

    SG::WriteHandle<FPGATrackSimClusterCollection> FPGASpacePoints (m_FPGASpacePointsKey, ctx);
    ATH_CHECK( FPGASpacePoints.record (std::make_unique<FPGATrackSimClusterCollection>()));

    // Query the event selection service to make sure this event passed cuts.
    if (!m_evtSel->getSelectedEvent()) {
        return StatusCode::SUCCESS;
    }
    ATH_MSG_INFO("Event accepted by: " << m_evtSel->name());

    // Event passes cuts, count it. technically, DataPrep does this now.
    m_evt++;

    // Read event info structure. all we need this for is to propagate to our event info structures.
    SG::ReadHandle<FPGATrackSimEventInfo> FPGAEventInfo(m_FPGAEventInfoKey, ctx);
    if (!FPGAEventInfo.isValid()) {
        ATH_MSG_ERROR("Could not find FPGA Event Info with key " << FPGAEventInfo.key());
        return StatusCode::FAILURE;
    }
    FPGATrackSimEventInfo eventInfo = *FPGAEventInfo.cptr();
    m_slicedFirstPixelHeader->newEvent(eventInfo);
    m_slicedSecondPixelHeader->newEvent(eventInfo);
    m_slicedStripHeader->newEvent(eventInfo);
    m_slicedStripHeaderPreSP->newEvent(eventInfo);

    if constexpr (enableBenchmark) m_chrono->chronoStart("1st Stage: Split hits to 1st and 2nd stage");

    std::vector<std::shared_ptr<const FPGATrackSimHit>> phits_output, phits_all, phits_1st, phits_2nd;
    phits_1st.reserve(FPGAHits->size());
    phits_2nd.reserve(FPGAHits->size());
    ATH_MSG_DEBUG("Incoming Hits: " << FPGAHits->size());
    for (const FPGATrackSimHit& hit : *(FPGAHits.cptr())) {
        phits_all.emplace_back(&hit, [](const FPGATrackSimHit*){});
    }

    // Use the slicing engine tool to do the stage-based separation. Does not use the pmap.
    m_slicingEngineTool->sliceHits(phits_all, phits_1st, phits_2nd);
    if(m_writeOutputData) *m_slicedStripHeaderPreSP = *m_slicedStripHeader;

    // The slicing engine puts strip hits into a logical event input header. That header now needs to go
    // to the spacepoint tool if it's turned on. Those hits then get added to phits_1st or phits_2nd as appropriate.
    if (m_doSpacepoints) {
        m_spacepoints.clear();
        if constexpr (enableBenchmark) m_chrono->chronoStart("1st Stage: SP fornmation");
        ATH_CHECK(m_spacepointsTool->DoSpacePoints(*m_slicedStripHeader, m_spacepoints));
        if constexpr (enableBenchmark) m_chrono->chronoStop("1st Stage: SP fornmation");
        for (const FPGATrackSimCluster& cluster : m_spacepoints) FPGASpacePoints->push_back(cluster);
    }

    // Use a property to control whether the strips/SPs go to 1st or second stage.
    for (const FPGATrackSimHit& hit : m_slicedStripHeader->towers().at(0).hits()) {
        (m_secondStageStrips ? phits_2nd : phits_1st).emplace_back(&hit, [](const FPGATrackSimHit*){});
    }
    for (auto& hit : phits_2nd) {
        FPGAHits_2nd->push_back(*hit);
    }

    // Add all hits including SPs to this for the HoughRootOutputTool
    for (const FPGATrackSimHit& hit : *(FPGAHits_2nd.cptr())) {
        phits_output.emplace_back(&hit, [](const FPGATrackSimHit*){});
    }

    if constexpr (enableBenchmark) m_chrono->chronoStop("1st Stage: Split hits to 1st and 2nd stage");
    ATH_MSG_DEBUG("1st stage hits: " << phits_1st.size() << "          2nd stage hits: " << phits_2nd.size() );
    if (phits_1st.empty()) return StatusCode::SUCCESS;
    // Get truth tracks from DataPrep as well.
    SG::ReadHandle<FPGATrackSimTruthTrackCollection> FPGATruthTracks(m_FPGATruthTrackKey, ctx);
    if (!FPGATruthTracks.isValid()) {
        ATH_MSG_ERROR("Could not find FPGA Truth Track Collection with key " << FPGATruthTracks.key());
        return StatusCode::FAILURE;
    }

    // Same for offline tracks.
    SG::ReadHandle<FPGATrackSimOfflineTrackCollection> FPGAOfflineTracks(m_FPGAOfflineTrackKey, ctx);
    if (!FPGAOfflineTracks.isValid()) {
        ATH_MSG_ERROR("Could not find FPGA Offline Track Collection with key " << FPGAOfflineTracks.key());
        return StatusCode::FAILURE;
    }

    if constexpr (enableBenchmark) m_chrono->chronoStart("1st Stage: GetRoads");
    // Get roads
    std::vector<std::shared_ptr<const FPGATrackSimRoad>> roads_1st;
    ATH_CHECK(m_roadFinderTool->getRoads(phits_1st, roads_1st, *(FPGATruthTracks.cptr())));

    ATH_MSG_DEBUG("Looping over " << roads_1st.size() << " roads");
    auto mon_nroads_1st = Monitored::Scalar<unsigned>("nroads_1st", roads_1st.size());
    for (auto const &road : roads_1st) {
      unsigned bitmask = road->getHitLayers();
      for (size_t l = 0; l < m_FPGATrackSimMapping->PlaneMap_1st(0)->getNLogiLayers(); l++) {
        if (bitmask & (1 << l)) {
            auto mon_layerIDs_1st = Monitored::Scalar<unsigned>("layerIDs_1st",l);
            Monitored::Group(m_monTool,mon_layerIDs_1st);
        }
      }
    }
    Monitored::Group(m_monTool, mon_nroads_1st);
    if constexpr (enableBenchmark) m_chrono->chronoStop("1st Stage: GetRoads");
    
    // Standard road Filter
    if constexpr (enableBenchmark) m_chrono->chronoStart("1st Stage: RoadFiltering");
    std::vector<std::shared_ptr<const FPGATrackSimRoad>> postfilter_roads;
    if (m_filterRoads)
    {
        ATH_CHECK(m_roadFilterTool->filterRoads(roads_1st, postfilter_roads));
        roads_1st = std::move(postfilter_roads);
    }
    if (m_doOverlapRemoval) ATH_CHECK(m_overlapRemovalTool_1st->runOverlapRemoval(roads_1st));
    // Road Filter2
    std::vector<std::shared_ptr<const FPGATrackSimRoad>> postfilter2_roads;
    if (m_filterRoads2) {
        ATH_CHECK(m_roadFilterTool2->filterRoads(roads_1st, postfilter2_roads));
        roads_1st = std::move(postfilter2_roads);
    }

    auto mon_nroads_1st_postfilter = Monitored::Scalar<unsigned>("nroads_1st_postfilter", roads_1st.size());
    Monitored::Group(m_monTool, mon_nroads_1st_postfilter);
    if constexpr (enableBenchmark) m_chrono->chronoStop("1st Stage: RoadFiltering");
    if constexpr (enableBenchmark) m_chrono->chronoStart("1st Stage: Tracking");

    // Get tracks
    std::vector<FPGATrackSimTrack> tracks_1st;
    if (m_doTracking) {
        if (m_doNNTrack) {
            ATH_MSG_DEBUG("Performing NN tracking");
            ATH_CHECK(m_NNTrackTool->getTracks_1st(roads_1st, tracks_1st));
            if (m_doGNNTrack) {
                ATH_MSG_DEBUG("Performing track parameter estimation");
                ATH_CHECK(m_NNTrackTool->setTrackParameters(tracks_1st,true,m_evtSel->getMin(), m_evtSel->getMax()));
            }
        } else {
            ATH_MSG_DEBUG("Performing Linear tracking");
            if (m_passLowestChi2TrackOnly) { // Pass only the lowest chi2 track per road
                // Loop over roads and keep only the best track for each road
                for (const auto& road : roads_1st) {
                    std::vector<FPGATrackSimTrack> tracksForCurrentRoad;

                    // Collect tracks for this road
                    std::vector<std::shared_ptr<const FPGATrackSimRoad>> roadVec = {road};
                    ATH_CHECK(m_trackFitterTool_1st->getTracks(roadVec, tracksForCurrentRoad, m_evtSel->getMin(), m_evtSel->getMax()));

                    // Find the best track for this road
                    if (!tracksForCurrentRoad.empty()) {
                        auto bestTrackIter = std::min_element(
                            tracksForCurrentRoad.begin(), tracksForCurrentRoad.end(),
                            [](const FPGATrackSimTrack& a, const FPGATrackSimTrack& b) {
                                return a.getChi2ndof() < b.getChi2ndof();
                            });

                        if (bestTrackIter != tracksForCurrentRoad.end() && bestTrackIter->getChi2ndof() < 1.e15) {
                            tracks_1st.push_back(*bestTrackIter);

                            // Monitor chi2 of the best track
                            auto mon_chi2_1st = Monitored::Scalar<float>("chi2_1st_all", bestTrackIter->getChi2ndof());
                            Monitored::Group(m_monTool, mon_chi2_1st);
                        }
                    }
                }
                // Monitor the best chi2 (from all tracks across all roads)
                if (!tracks_1st.empty()) {
                    float bestChi2Overall = std::min_element(
                        tracks_1st.begin(), tracks_1st.end(),
                        [](const FPGATrackSimTrack& a, const FPGATrackSimTrack& b) {
                            return a.getChi2ndof() < b.getChi2ndof();
                        })->getChi2ndof();

                    auto mon_best_chi2_1st = Monitored::Scalar<float>("best_chi2_1st", bestChi2Overall);
                    Monitored::Group(m_monTool, mon_best_chi2_1st);
                }
            } else { // Pass all tracks with chi2 < 1e15
	      ATH_CHECK(m_trackFitterTool_1st->getTracks(roads_1st, tracks_1st, m_evtSel->getMin(), m_evtSel->getMax()));
                float bestchi2 = 1.e15;
                for (const FPGATrackSimTrack& track : tracks_1st) {
                    float chi2 = track.getChi2ndof();
                    if (chi2 < bestchi2) bestchi2 = chi2;
                    auto mon_chi2_1st = Monitored::Scalar<float>("chi2_1st_all", chi2);
                    Monitored::Group(m_monTool, mon_chi2_1st);
                }
                auto mon_best_chi2_1st = Monitored::Scalar<float>("best_chi2_1st", bestchi2);
                Monitored::Group(m_monTool, mon_best_chi2_1st);
            }
        }
    } else { // No tracking; 
      ATH_MSG_DEBUG("No tracking. Just running dummy road2track algorith");
      roadsToTrack(roads_1st, tracks_1st, m_FPGATrackSimMapping->PlaneMap_1st(0));
    }

    std::vector<FPGATrackSimTruthTrack> truthtracks = *FPGATruthTracks;
    std::vector<FPGATrackSimOfflineTrack> offlineTracks = *FPGAOfflineTracks;
    //Loop over tracks and set the region for all of them, also optionally set track parameters to truth
    for (FPGATrackSimTrack& track : tracks_1st) {
        track.setRegion(m_region);
	if (m_SetTruthParametersForTracks >= 0 && truthtracks.size() > 0) {
	  if (m_SetTruthParametersForTracks != 0)
	    track.setQOverPt(truthtracks.front().getQOverPt());
	  else if	(m_SetTruthParametersForTracks != 1)
	    track.setD0(truthtracks.front().getD0());
	  else if (m_SetTruthParametersForTracks != 2)
	    track.setPhi(truthtracks.front().getPhi());
	  else if (m_SetTruthParametersForTracks != 3)
	    track.setZ0(truthtracks.front().getZ0());
	  else if (m_SetTruthParametersForTracks != 4)
	    track.setEta(truthtracks.front().getEta());
	}
    }
    // Loop over roads and store them in SG (after track finding to also copy the sector information)
    for (auto const& road : roads_1st) {
        std::vector<FPGATrackSimHit> road_hits;
        ATH_MSG_DEBUG("Hough Road X Y: " << road->getX() << " " << road->getY());
        for (size_t l = 0; l < road->getNLayers(); ++l) {
            for (const auto& layerH : road->getHits(l)) {
                road_hits.push_back(*layerH);
            }
        }
        FPGAHitsInRoads_1st->push_back(road_hits);
        FPGARoads_1st->push_back(*road);
    }
    if constexpr (enableBenchmark) m_chrono->chronoStop("1st Stage: Tracking");

    // Monitor the number of tracks
    auto mon_ntracks_1st = Monitored::Scalar<unsigned>("ntrack_1st", tracks_1st.size());
    Monitored::Group(m_monTool, mon_ntracks_1st);
    if constexpr (enableBenchmark) m_chrono->chronoStart("1st Stage: OverlapRemoval");
    // Overlap removal
    if (m_doOverlapRemoval)  ATH_CHECK(m_overlapRemovalTool_1st->runOverlapRemoval(tracks_1st));

    unsigned ntrackOLRChi2 = 0;
    for (const FPGATrackSimTrack& track : tracks_1st) {
      if (track.getChi2ndof() < m_trackScoreCut.value()) {
            m_nTracksChi2Tot++;
            if (track.passedOR()) {
                ntrackOLRChi2++;
                m_nTracksChi2OLRTot++;

                // For tracks passing overlap removal-- record the chi2 so we can figure out the right cut.
                float chi2olr = track.getChi2ndof();
                auto mon_chi2_1st_or = Monitored::Scalar<float>("chi2_1st_afterOLR", chi2olr);
                Monitored::Group(m_monTool, mon_chi2_1st_or);
            }
        }
    }
    if constexpr (enableBenchmark) m_chrono->chronoStop("1st Stage: OverlapRemoval");
    auto mon_ntracks_1st_olr = Monitored::Scalar<unsigned>("ntrack_1st_afterOLR", ntrackOLRChi2);
    Monitored::Group(m_monTool,mon_ntracks_1st_olr);

    m_nRoadsTot += roads_1st.size();
    m_nTracksTot += tracks_1st.size();

    // Do some simple monitoring of efficiencies. okay, we need truth tracks here.
    if (truthtracks.size() > 0) {
        m_evt_truth++;
        auto passroad = Monitored::Scalar<bool>("eff_road",(roads_1st.size() > 0));
        auto passtrack = Monitored::Scalar<bool>("eff_track",(tracks_1st.size() > 0));
        auto truthpT_zoom = Monitored::Scalar<float>("pT_zoom",truthtracks.front().getPt()*0.001);
        auto truthpT = Monitored::Scalar<float>("pT",truthtracks.front().getPt()*0.001);
        auto trutheta = Monitored::Scalar<float>("eta",truthtracks.front().getEta());
        auto truthphi= Monitored::Scalar<float>("phi",truthtracks.front().getPhi());
        auto truthd0= Monitored::Scalar<float>("d0",truthtracks.front().getD0());
        auto truthz0= Monitored::Scalar<float>("z0",truthtracks.front().getZ0());
        if (roads_1st.size() > 0) m_nRoadsFound++;
	if (roads_1st.size() > m_maxNRoadsFound) m_maxNRoadsFound = roads_1st.size();

        unsigned npasschi2(0);
        unsigned npasschi2OLR(0);
        if (tracks_1st.size() > 0) {
            m_nTracksFound++;
	    if (tracks_1st.size() > m_maxNTracksTot) m_maxNTracksTot = tracks_1st.size();
            for (const auto& track : tracks_1st) {
	      if (track.getChi2ndof() < m_trackScoreCut.value()) {
		  npasschi2++;
                    if (track.passedOR()) {
		      npasschi2OLR++;
                    }
                }
            }
        }
	if (npasschi2 > m_maxNTracksChi2Tot) m_maxNTracksChi2Tot = npasschi2;
	if (npasschi2OLR > m_maxNTracksChi2OLRTot) m_maxNTracksChi2OLRTot = npasschi2OLR;
        if (npasschi2 > 0) m_nTracksChi2Found++;
        if (npasschi2OLR > 0) m_nTracksChi2OLRFound++;
        auto passtrackchi2 = Monitored::Scalar<bool>("eff_track_chi2",(npasschi2 > 0));
        Monitored::Group(m_monTool,passroad,passtrack,truthpT_zoom,truthpT,trutheta,truthphi,truthd0,truthz0,passtrackchi2);
    }

    for (const FPGATrackSimTrack& track : tracks_1st) FPGATracks_1stHandle->push_back(track);

    // Now, we may want to do large-radius tracking on the hits not used by the first stage tracking.
    // This follows overlap removal.
    std::vector<std::shared_ptr<const FPGATrackSimRoad>> roadsLRT;
    std::vector<FPGATrackSimTrack> tracksLRT; // currently empty
    if (m_doLRT) {
        // Filter out hits that are on successful first-stage tracks
        std::vector<std::shared_ptr<const FPGATrackSimHit>> remainingHits;

        if (m_doLRTHitFiltering) {
            ATH_MSG_DEBUG("Doing hit filtering based on prompt tracks.");
            ATH_CHECK(m_LRTRoadFilterTool->filterUsedHits(tracks_1st, phits_1st, remainingHits));

            for (const auto &Hit : remainingHits) FPGAHitsFiltered_1st->push_back(*Hit);

        } else {
            ATH_MSG_DEBUG("No hit filtering requested; using all hits for LRT.");
            remainingHits = phits_1st;
        }

        // Get LRT roads with remaining hits
        ATH_MSG_DEBUG("Finding LRT roads");
        ATH_CHECK(m_LRTRoadFinderTool->getRoads( remainingHits, roadsLRT ));
    }

    auto dataFlowInfo = std::make_unique<FPGATrackSimDataFlowInfo>();

    // Write the output and reset
    if (m_writeOutputData)  {
        ATH_CHECK(writeOutputData(roads_1st, tracks_1st, dataFlowInfo.get()));
    }

    // This one we can do-- by passing in truth and offline tracks via storegate above.
    if (m_doHoughRootOutput1st) {
        ATH_MSG_DEBUG("Running HoughRootOutputTool in 1st stage.");

        SmartIF<IEventProcessor> appMgr{service("ApplicationMgr")};
        if (!appMgr) {
            ATH_MSG_ERROR("Failed to retrieve ApplicationMgr as IEventProcessor");
            return StatusCode::FAILURE;
        }

        // Create output ROOT file
        ATH_CHECK(m_houghRootOutputTool->fillTree(roads_1st, truthtracks, offlineTracks, phits_output, m_writeOutNonSPStripHits, m_trackScoreCut.value(), m_NumOfHitPerGrouping, false));
    }

    // Reset data pointers
    if (m_outputRoadUnionTool) m_slicedHitHeader->reset();
    m_logicEventOutputHeader->reset();

    return StatusCode::SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
//                  INPUT PASSING, READING AND PROCESSING                    //
///////////////////////////////////////////////////////////////////////////////

StatusCode FPGATrackSimLogicalHitsProcessAlg::writeOutputData(  const std::vector<std::shared_ptr<const FPGATrackSimRoad>>& roads_1st,
                                                                std::vector<FPGATrackSimTrack> const& tracks_1st,
                                                                FPGATrackSimDataFlowInfo const* dataFlowInfo)
{
  m_logicEventOutputHeader->reset();

  ATH_MSG_DEBUG("NFPGATrackSimRoads_1st = " << roads_1st.size() << ", NFPGATrackSimTracks_1st = " << tracks_1st.size());

  if (!m_writeOutputData) return StatusCode::SUCCESS;
    m_logicEventOutputHeader->reserveFPGATrackSimRoads_1st(roads_1st.size());
    m_logicEventOutputHeader->addFPGATrackSimRoads_1st(roads_1st);
  if (m_doTracking) {
    m_logicEventOutputHeader->reserveFPGATrackSimTracks_1st(tracks_1st.size());
    m_logicEventOutputHeader->addFPGATrackSimTracks_1st(tracks_1st);
  }


  m_logicEventOutputHeader->setDataFlowInfo(*dataFlowInfo);
  ATH_MSG_DEBUG(m_logicEventOutputHeader->getDataFlowInfo());

  // It would be nice to rearrange this so both algorithms use one instance of this tool, I think.
  // Which means that dataprep can't call writeData because that does Fill().
  ATH_CHECK(m_writeOutputTool->writeData());



  return StatusCode::SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
// Finalize

StatusCode FPGATrackSimLogicalHitsProcessAlg::finalize()
{
    ATH_MSG_INFO("PRINTING FPGATRACKSIM SIMPLE STATS");
    ATH_MSG_INFO("========================================================================================");
    ATH_MSG_INFO("Ran on events = " << m_evt);
    ATH_MSG_INFO("Inclusive efficiency to find a road = " << (m_evt_truth == 0 ? "NAN" : std::to_string(m_nRoadsFound/(float)m_evt_truth)));
    ATH_MSG_INFO("Inclusive efficiency to find a track = " << (m_evt_truth == 0 ? "NAN" : std::to_string(m_nTracksFound/(float)m_evt_truth)));
    ATH_MSG_INFO("Inclusive efficiency to find a track passing chi2 = " << (m_evt_truth == 0 ? "NAN" : std::to_string(m_nTracksChi2Found/(float)m_evt_truth)));
    ATH_MSG_INFO("Inclusive efficiency to find a track passing chi2 and OLR = " << (m_evt_truth == 0 ? "NAN" : std::to_string(m_nTracksChi2OLRFound/(float)m_evt_truth)));


    ATH_MSG_INFO("Number of 1st stage roads/event = " << (m_evt == 0 ? "NAN" : std::to_string(m_nRoadsTot/(float)m_evt)));
    ATH_MSG_INFO("Number of 1st stage track combinations/event = " << (m_evt == 0 ? "NAN" : std::to_string(m_nTracksTot/(float)m_evt)));
    ATH_MSG_INFO("Number of 1st stage tracks passing chi2/event = " << (m_evt == 0 ? "NAN" : std::to_string(m_nTracksChi2Tot/(float)m_evt)));
    ATH_MSG_INFO("Number of 1st stage tracks passing chi2 and OLR/event = " << (m_evt == 0 ? "NAN" : std::to_string(m_nTracksChi2OLRTot/(float)m_evt)));
    ATH_MSG_INFO("========================================================================================");

    ATH_MSG_INFO("Max number of 1st stage roads in an event = " << m_maxNRoadsFound);
    ATH_MSG_INFO("Max number of 1st stage track combinations in an event = " << m_maxNTracksTot);
    ATH_MSG_INFO("Max number of 1st stage tracks passing chi2 in an event = " << m_maxNTracksChi2Tot);
    ATH_MSG_INFO("Max number of 1st stage tracks passing chi2 and OLR in an event = " << m_maxNTracksChi2OLRTot);
    ATH_MSG_INFO("========================================================================================");

    return StatusCode::SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
// Helpers

void FPGATrackSimLogicalHitsProcessAlg::printHitSubregions(std::vector<FPGATrackSimHit> const & hits)
{
    ATH_MSG_WARNING("Hit regions:");
    for (const auto& hit : hits)
    {
        std::vector<uint32_t> regions = m_FPGATrackSimMapping->SubRegionMap()->getRegions(hit);
        std::stringstream ss;
        for (auto r : regions)
            ss << r << ",";
        ATH_MSG_WARNING("\t[" << ss.str() << "]");
    }
}
