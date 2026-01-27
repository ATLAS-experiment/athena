// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

/* Second stage alg needs to:
 *  retrieve Tracks_1st and Hits_2nd from storegate
 *  call some function that returns second stage roads by adding the hits to the tracks
 *  take the second stage roads and run them through scoring + DR again, optionally.
 */

#include "FPGATrackSimSecondStageAlg.h"

#include "FPGATrackSimObjects/FPGATrackSimCluster.h"
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
#include "AthenaKernel/Chrono.h"
#include <optional>

constexpr bool enableBenchmark = 
#ifdef BENCHMARK_FPGATRACKSIM
    true;
#else
    false;
#endif

///////////////////////////////////////////////////////////////////////////////
// Initialize

FPGATrackSimSecondStageAlg::FPGATrackSimSecondStageAlg (const std::string& name, ISvcLocator* pSvcLocator) :
    AthAlgorithm(name, pSvcLocator)
{
}


StatusCode FPGATrackSimSecondStageAlg::initialize()
{
    std::stringstream ss(m_description);
    std::string line;
    ATH_MSG_INFO("Tag config:");
    if (!m_description.empty()) {
        while (std::getline(ss, line, '\n')) {
            ATH_MSG_INFO('\t' << line);
        }
    }

    ATH_CHECK(m_houghRootOutputTool.retrieve(EnableTool{m_doHoughRootOutput2nd}));
    ATH_CHECK(m_NNTrackTool.retrieve(EnableTool{m_doNNTrack_2nd}));

    ATH_CHECK(m_trackFitterTool.retrieve(EnableTool{m_doTracking}));
    ATH_CHECK(m_overlapRemovalTool.retrieve());
    ATH_CHECK(m_writeOutputTool.retrieve());
    ATH_CHECK(m_FPGATrackSimMapping.retrieve());
    ATH_CHECK(m_trackExtensionTool.retrieve());

    ATH_MSG_DEBUG("initialize() Instantiating root objects");

    // This file should only need to generate one input and output branch.
    m_slicedHitHeader = m_writeOutputTool->addInputBranch(m_sliceBranch.value(), true);
    m_logicEventOutputHeader = m_writeOutputTool->addOutputBranch(m_outputBranch.value(), true);

    // Connect the sliced hit tool accordingly. This may need to be a separate flag.
    if (m_writeOutputData) ATH_CHECK(m_trackExtensionTool->setupSlices(m_slicedHitHeader));

    ATH_MSG_DEBUG("initialize() Setting branch");

    if (!m_monTool.empty())
        ATH_CHECK(m_monTool.retrieve());

    ATH_CHECK( m_FPGAInputTrackKey.initialize());
    ATH_CHECK( m_FPGAHitInRoadsKey.initialize() );
    ATH_CHECK( m_FPGARoadKey.initialize() );
    ATH_CHECK( m_FPGATrackKey.initialize() );
    ATH_CHECK( m_FPGAHitKey.initialize() );
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

StatusCode FPGATrackSimSecondStageAlg::execute()
{
    const EventContext& ctx = getContext();
    // Get reference to hits from StoreGate.
    // Hits have been procesed by the DataPrep algorithm. Now, we need to read them.
    // If they aren't passed, assume this means we are done.
    SG::ReadHandle<FPGATrackSimHitCollection> FPGAHits(m_FPGAHitKey, ctx);
    if (!FPGAHits.isValid()) {
        if (m_evt == 0) {
            ATH_MSG_WARNING("Didn't receive FPGAHits_2nd on first event; assuming no input events.");
        }
        SmartIF<IEventProcessor> appMgr{service("ApplicationMgr")};
        if (!appMgr) {
            ATH_MSG_ERROR("Failed to retrieve ApplicationMgr as IEventProcessor");
            return StatusCode::FAILURE;
        }
        return appMgr->stopRun();
    }
    
    SG::ReadHandle<FPGATrackSimTrackCollection> FPGAInputTracks (m_FPGAInputTrackKey, ctx);
    if (!FPGAInputTracks.isValid()) {
        SmartIF<IEventProcessor> appMgr{service("ApplicationMgr")};
        if (!appMgr) {
            ATH_MSG_ERROR("Failed to retrieve ApplicationMgr as IEventProcessor");
            return StatusCode::FAILURE;
        }
        return appMgr->stopRun();
    }

    // Set up write handles.
    SG::WriteHandle<FPGATrackSimRoadCollection> FPGARoads_2nd (m_FPGARoadKey, ctx);
    SG::WriteHandle<FPGATrackSimHitContainer> FPGAHitsInRoads_2nd (m_FPGAHitInRoadsKey, ctx);

    ATH_CHECK( FPGARoads_2nd.record (std::make_unique<FPGATrackSimRoadCollection>()));
    ATH_CHECK( FPGAHitsInRoads_2nd.record (std::make_unique<FPGATrackSimHitContainer>()));

    SG::WriteHandle<FPGATrackSimTrackCollection> FPGATracks_2ndHandle (m_FPGATrackKey, ctx);
    ATH_CHECK(FPGATracks_2ndHandle.record (std::make_unique<FPGATrackSimTrackCollection>()));
    {
        std::optional<Athena::Chrono> chrono;
        if constexpr (enableBenchmark) chrono.emplace("2nd Stage: EventSelection", m_chrono.get());
        // Query the event selection service to make sure this event passed cuts.
        if (!m_evtSel->getSelectedEvent()) {
            ATH_MSG_DEBUG("Event skipped by: " << m_evtSel->name());
            return StatusCode::SUCCESS;
        }

        // Event passes cuts, count it. technically, DataPrep does this now.
        m_evt++;
    }

    // If we get here, FPGAHits_2nd is valid, copy it over.
    std::vector<std::shared_ptr<const FPGATrackSimHit>> phits_2nd;
    phits_2nd.reserve(FPGAHits->size());
    for (const auto& hit : *FPGAHits) {
        phits_2nd.push_back(std::make_shared<const FPGATrackSimHit>(*hit));
    }

    ATH_MSG_DEBUG("Retrieved " << phits_2nd.size() << " hits and " << FPGAInputTracks->size() << " tracks from storegate");

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

    // Same for event info structure. all we need this for is to propagate to our event info structures.
    SG::ReadHandle<FPGATrackSimEventInfo> FPGAEventInfo(m_FPGAEventInfoKey, ctx);
    if (!FPGAEventInfo.isValid()) {
        ATH_MSG_ERROR("Could not find FPGA Event Info with key " << FPGAEventInfo.key());
       return StatusCode::FAILURE;
    }
    FPGATrackSimEventInfo eventInfo = *FPGAEventInfo.cptr();
    m_slicedHitHeader->newEvent(eventInfo);

    // Get second stage roads from tracks.
    std::vector<FPGATrackSimRoad> roads;

    {
        std::optional<Athena::Chrono> chrono;
        if constexpr (enableBenchmark) chrono.emplace("2nd Stage: Track Extension", m_chrono.get());
        // Use the track extension tool to actually produce a new set of roads.
        ATH_CHECK(m_trackExtensionTool->extendTracks(phits_2nd, *FPGAInputTracks, roads));

        for (auto const& road : roads) {
            auto road_hits = std::make_unique<FPGATrackSimHitCollection>();
            ATH_MSG_DEBUG("Hough Road X Y: " << road.getX() << " " << road.getY());
            for (size_t l = 0; l < road.getNLayers(); ++l) {
                for (const auto& layerH : road.getHits(l)) {
                    road_hits->push_back(new FPGATrackSimHit(*layerH));
                }
            }
            FPGAHitsInRoads_2nd->push_back(std::move(*road_hits));
            FPGARoads_2nd->push_back(road);
        }
    }
    
    auto mon_nroads = Monitored::Scalar<unsigned>("nroads_2nd", roads.size());
    unsigned bitmask_best(0);
    unsigned nhit_best(0);
    for (auto const &road : roads) {
        unsigned bitmask = road.getHitLayers();
        if (road.getNHitLayers() > nhit_best) {
            nhit_best = road.getNHitLayers();
            bitmask_best = bitmask;
        }
        for (size_t l = 0; l < m_FPGATrackSimMapping->PlaneMap_2nd(0)->getNLogiLayers(); l++) {
            if (bitmask & (1 << l)) {
                auto mon_layerIDs = Monitored::Scalar<unsigned>("layerIDs_2nd",l);
                Monitored::Group(m_monTool,mon_layerIDs);
            }
        }
    }

    for (size_t l = 0; l < m_FPGATrackSimMapping->PlaneMap_2nd(0)->getNLogiLayers(); l++) {
        if (bitmask_best & (1 << l)) {
            auto mon_layerIDs_best = Monitored::Scalar<unsigned>("layerIDs_2nd_best",l);
            Monitored::Group(m_monTool,mon_layerIDs_best);
        }
    }
    Monitored::Group(m_monTool, mon_nroads);

    // NOTE: for now we don't support road filtering again in the second stage.
    auto mon_nroads_postfilter = Monitored::Scalar<unsigned>("nroads_2nd_postfilter", roads.size());
    Monitored::Group(m_monTool, mon_nroads_postfilter);

    // Get tracks, again, after extrapolation.
    // All of this code is effectively copied from LogicalHitsProcessAlg, except we use 2nd stage now.
    std::vector<FPGATrackSimTrack> tracks;
    {
        std::optional<Athena::Chrono> chrono;
        if constexpr (enableBenchmark) chrono.emplace("2nd Stage: Track Extraction", m_chrono.get());
        if (m_doTracking) {
        if (m_doNNTrack_2nd) {
            ATH_MSG_DEBUG("Performing NN tracking");
            ATH_CHECK(m_NNTrackTool->getTracks_2nd(roads, tracks));
        } else {
            ATH_MSG_DEBUG("Performing Linear tracking");

            if (m_passLowestChi2TrackOnly) { // Pass only the lowest chi2 track per road

                std::vector<FPGATrackSimTrack> filteredTracks;

                for (const auto& road : roads) {
                    // Collect tracks for the current road
                    std::vector<FPGATrackSimTrack> tracksForCurrentRoad;
                    std::vector<FPGATrackSimRoad> roadVec = {road};
                    ATH_CHECK(m_trackFitterTool->getTracks(roadVec, tracksForCurrentRoad, m_evtSel->getMin(), m_evtSel->getMax()));

                    // Find and keep the best track (lowest chi2) for this road
                    if (!tracksForCurrentRoad.empty()) {
                        auto bestTrackIter = std::min_element(
                            tracksForCurrentRoad.begin(), tracksForCurrentRoad.end(),
                            [](const FPGATrackSimTrack& a, const FPGATrackSimTrack& b) {
                                return a.getChi2ndof() < b.getChi2ndof();
                            });

                        if (bestTrackIter != tracksForCurrentRoad.end() && bestTrackIter->getChi2ndof() < 1.e15) {
                            filteredTracks.push_back(*bestTrackIter);

                            // Monitor chi2 of the best track
                            auto mon_chi2 = Monitored::Scalar<float>("chi2_2nd_all", bestTrackIter->getChi2ndof());
                            Monitored::Group(m_monTool, mon_chi2);
                        }
                    }
                }

                // Update tracks with filtered tracks
                tracks = std::move(filteredTracks);

                // Monitor the best chi2 across all roads
                if (!tracks.empty()) {
                    float bestChi2Overall = std::min_element(
                        tracks.begin(), tracks.end(),
                        [](const FPGATrackSimTrack& a, const FPGATrackSimTrack& b) {
                            return a.getChi2ndof() < b.getChi2ndof();
                        })->getChi2ndof();

                    auto mon_best_chi2 = Monitored::Scalar<float>("best_chi2_2nd", bestChi2Overall);
                    Monitored::Group(m_monTool, mon_best_chi2);
                }
            } else { // Pass all tracks with chi2 < 1e15
                ATH_CHECK(m_trackFitterTool->getTracks(roads, tracks, m_evtSel->getMin(), m_evtSel->getMax()));
                float bestchi2 = 1.e15;
                for (const FPGATrackSimTrack& track : tracks) {
                    float chi2 = track.getChi2ndof();
                    if (chi2 < bestchi2) bestchi2 = chi2;
                    auto mon_chi2 = Monitored::Scalar<float>("chi2_2nd_all", chi2);
                    Monitored::Group(m_monTool, mon_chi2);
                }
                auto mon_best_chi2 = Monitored::Scalar<float>("best_chi2_2nd", bestchi2);
                Monitored::Group(m_monTool, mon_best_chi2);
            }
        }
        } else {
            // No tracking; just run road to track
            roadsToTrack(roads, tracks, m_FPGATrackSimMapping->PlaneMap_2nd(0));
        }
    }
    auto mon_ntracks = Monitored::Scalar<unsigned>("ntrack_2nd", tracks.size());
    Monitored::Group(m_monTool,mon_ntracks);

    // Overlap removal
    {
        std::optional<Athena::Chrono> chrono;
        if constexpr (enableBenchmark) chrono.emplace("2nd Stage: Overlap Removal", m_chrono.get());
        ATH_CHECK(m_overlapRemovalTool->runOverlapRemoval(tracks));
    }
    {
        std::optional<Athena::Chrono> chrono;
        if constexpr (enableBenchmark) chrono.emplace("2nd Stage: Parameter Estimation", m_chrono.get());
        // If running NN Track tool, now we get the track parameters (it's slow so we only do it for tracks passing OLR)
        if (m_doTracking && m_doNNTrack_2nd) {
            ATH_CHECK(m_NNTrackTool->setTrackParameters(tracks,false,m_evtSel->getMin(), m_evtSel->getMax()));
        }
    }
    const auto& truthtracks = *FPGATruthTracks;
    const auto& offlineTracks = *FPGAOfflineTracks;

    // Optionally loop over tracks and set track parameters to truth
    {
        std::optional<Athena::Chrono> chrono;
        if constexpr (enableBenchmark) chrono.emplace("2nd Stage: Truth Param Override", m_chrono.get());
        if (m_SetTruthParametersForTracks >= 0 && truthtracks.size() > 0) {
            for (auto track : tracks) {
                if (m_SetTruthParametersForTracks != 0)
                    track.setQOverPt(truthtracks.front().getQOverPt());
                else if (m_SetTruthParametersForTracks != 1)
                    track.setD0(truthtracks.front().getD0());
                else if (m_SetTruthParametersForTracks != 2)
                    track.setPhi(truthtracks.front().getPhi());
                else if (m_SetTruthParametersForTracks != 3)
                    track.setZ0(truthtracks.front().getZ0());
                else if (m_SetTruthParametersForTracks != 4)
                    track.setEta(truthtracks.front().getEta());
            }
        }
    }

    unsigned ntrackOLRChi2 = 0;
    {
        std::optional<Athena::Chrono> chrono;
        if constexpr (enableBenchmark) chrono.emplace("2nd Stage: Track Counting", m_chrono.get());
        for (const FPGATrackSimTrack& track : tracks) {
            if (track.getChi2ndof() < m_trackScoreCut.value()) {
                m_nTracksChi2Tot++;
                if (track.passedOR()) {
                    ntrackOLRChi2++;
                    m_nTracksChi2OLRTot++;

                    // For tracks passing overlap removal-- record the chi2 so we can figure out the right cut.
                    float chi2olr = track.getChi2ndof();
                    auto mon_chi2_or = Monitored::Scalar<float>("chi2_2nd_afterOLR", chi2olr);
                    Monitored::Group(m_monTool, mon_chi2_or);
                }
            }
        }
        auto mon_ntracks_olr = Monitored::Scalar<unsigned>("ntrack_2nd_afterOLR", ntrackOLRChi2);
        Monitored::Group(m_monTool,mon_ntracks_olr);
    }

    m_nRoadsTot += roads.size();
    m_nTracksTot += tracks.size();

    // Do some simple monitoring of efficiencies. okay, we need truth tracks here.
    {
        std::optional<Athena::Chrono> chrono;
        if constexpr (enableBenchmark) chrono.emplace("2nd Stage: Efficiency Monitoring", m_chrono.get());
        if (truthtracks.size() > 0) {
            m_evt_truth++;
            auto passroad = Monitored::Scalar<bool>("eff_road_2nd",(roads.size() > 0));
            auto passtrack = Monitored::Scalar<bool>("eff_track_2nd",(tracks.size() > 0));
            auto truthpT_zoom = Monitored::Scalar<float>("pT_zoom",truthtracks.front().getPt()*0.001);
            auto truthpT = Monitored::Scalar<float>("pT",truthtracks.front().getPt()*0.001);
            auto trutheta = Monitored::Scalar<float>("eta",truthtracks.front().getEta());
            auto truthphi= Monitored::Scalar<float>("phi",truthtracks.front().getPhi());
            auto truthd0= Monitored::Scalar<float>("d0",truthtracks.front().getD0());
            auto truthz0= Monitored::Scalar<float>("z0",truthtracks.front().getZ0());
            if (roads.size() > 0) m_nRoadsFound++;
            if (roads.size() > m_maxNRoadsFound) m_maxNRoadsFound = roads.size();

            unsigned npasschi2(0);
            unsigned npasschi2OLR(0);

            if (tracks.size() > 0) {
                m_nTracksFound++;
                if (tracks.size() > m_maxNTracksTot) m_maxNTracksTot = tracks.size();
                for (const auto& track : tracks) {
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

            auto passtrackchi2 = Monitored::Scalar<bool>("eff_track_chi2_2nd",(npasschi2 > 0));
            Monitored::Group(m_monTool,passroad,passtrack,truthpT_zoom,truthpT,trutheta,truthphi,truthd0,truthz0,passtrackchi2);
        }
    }

    {
        std::optional<Athena::Chrono> chrono;
        if constexpr (enableBenchmark) chrono.emplace("2nd Stage: Write Tracks", m_chrono.get());
        for (const FPGATrackSimTrack& track : tracks) FPGATracks_2ndHandle->push_back(track);
    }

    // Write the output and reset
    {
        std::optional<Athena::Chrono> chrono;
        if constexpr (enableBenchmark) chrono.emplace("2nd Stage: Output", m_chrono.get());
        if (m_writeOutputData)  {
            auto dataFlowInfo = std::make_unique<FPGATrackSimDataFlowInfo>();
            ATH_CHECK(writeOutputData(roads, tracks, dataFlowInfo.get()));
        }

        if (m_doHoughRootOutput2nd) {
            ATH_MSG_DEBUG("Running HoughRootOutputTool in 2nd stage.");
            ATH_CHECK(m_houghRootOutputTool->fillTree(tracks, truthtracks, offlineTracks, phits_2nd, m_writeOutNonSPStripHits, true));
        }
    }

    // Reset data pointers
    m_slicedHitHeader->reset();
    m_logicEventOutputHeader->reset();

    return StatusCode::SUCCESS;
}


StatusCode FPGATrackSimSecondStageAlg::writeOutputData( const std::vector<FPGATrackSimRoad>& roads_2nd,
                                                        std::vector<FPGATrackSimTrack> const& tracks_2nd,
                                                        FPGATrackSimDataFlowInfo const* dataFlowInfo)
{
  m_logicEventOutputHeader->reset();

  ATH_MSG_DEBUG("NFPGATrackSimRoads_2nd = " << roads_2nd.size() << ", NFPGATrackSimTracks_2nd = " << tracks_2nd.size());

  if (!m_writeOutputData) return StatusCode::SUCCESS;
    m_logicEventOutputHeader->reserveFPGATrackSimRoads_2nd(roads_2nd.size());
    m_logicEventOutputHeader->addFPGATrackSimRoads_2nd(roads_2nd);
  if (m_doTracking) {
    m_logicEventOutputHeader->reserveFPGATrackSimTracks_2nd(tracks_2nd.size());
    m_logicEventOutputHeader->addFPGATrackSimTracks_2nd(tracks_2nd);
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

StatusCode FPGATrackSimSecondStageAlg::finalize()
{
    ATH_MSG_INFO("PRINTING FPGATRACKSIM SIMPLE STATS: SECOND STAGE");
    ATH_MSG_INFO("========================================================================================");
    ATH_MSG_INFO("Ran on events = " << m_evt);
    ATH_MSG_INFO("Inclusive efficiency to find a road = " << (m_evt_truth == 0 ? "NAN" : std::to_string(m_nRoadsFound/(float)m_evt_truth)));
    ATH_MSG_INFO("Inclusive efficiency to find a track = " << (m_evt_truth == 0 ? "NAN" : std::to_string(m_nTracksFound/(float)m_evt_truth)));
    ATH_MSG_INFO("Inclusive efficiency to find a track passing chi2 = " << (m_evt_truth == 0 ? "NAN" : std::to_string(m_nTracksChi2Found/(float)m_evt_truth)));
    ATH_MSG_INFO("Inclusive efficiency to find a track passing chi2 and OLR = " << (m_evt_truth == 0 ? "NAN" : std::to_string(m_nTracksChi2OLRFound/(float)m_evt_truth)));


    ATH_MSG_INFO("Number of 2nd stage roads/event = " << (m_evt == 0 ? "NAN" : std::to_string(m_nRoadsTot/(float)m_evt)));
    ATH_MSG_INFO("Number of 2nd stage track combinations/event = " << (m_evt == 0 ? "NAN" : std::to_string(m_nTracksTot/(float)m_evt)));
    ATH_MSG_INFO("Number of 2nd stage tracks passing chi2/event = " << (m_evt == 0 ? "NAN" : std::to_string(m_nTracksChi2Tot/(float)m_evt)));
    ATH_MSG_INFO("Number of 2nd stage tracks passing chi2 and OLR/event = " << (m_evt == 0 ? "NAN" : std::to_string(m_nTracksChi2OLRTot/(float)m_evt)));
    ATH_MSG_INFO("========================================================================================");

    ATH_MSG_INFO("Max number of 2nd stage roads in an event = " << m_maxNRoadsFound);
    ATH_MSG_INFO("Max number of 2nd stage track combinations in an event = " << m_maxNTracksTot);
    ATH_MSG_INFO("Max number of 2nd stage tracks passing chi2 in an event = " << m_maxNTracksChi2Tot);
    ATH_MSG_INFO("Max number of 2nd stage tracks passing chi2 and OLR in an event = " << m_maxNTracksChi2OLRTot);
    ATH_MSG_INFO("========================================================================================");
    return StatusCode::SUCCESS;
}
