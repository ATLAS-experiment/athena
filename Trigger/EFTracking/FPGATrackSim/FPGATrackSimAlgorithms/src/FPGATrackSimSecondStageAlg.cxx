// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

/* Second stage alg needs to:
 *  retrieve Tracks_1st and Hits_2nd from storegate
 *  call some function that returns second stage roads by adding the hits to the tracks
 *  take the second stage roads and run them through scoring + DR again, optionally.
 */

#include "FPGATrackSimSecondStageAlg.h"

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

#ifdef BENCHMARK_LOGICALHITSALG
#define TIME(name) \
    t_1 = std::chrono::steady_clock::now(); \
    (name) += std::chrono::duration_cast<std::chrono::microseconds>(t_1 - t_0).count(); \
    t_0 = t_1;

size_t m_troads = 0, m_troad_filter = 0, m_tlrt = 0, m_ttracks = 0, m_tOR = 0, m_tmon = 0, m_tfin = 0;
#else
#define TIME(name)
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

    ATH_CHECK(m_houghRootOutputTool.retrieve(EnableTool{m_doHoughRootOutput}));
    ATH_CHECK(m_NNTrackTool.retrieve(EnableTool{m_doNNTrack}));
    if (m_doSpacepoints) ATH_CHECK(m_spRoadFilterTool.retrieve(EnableTool{m_spRoadFilterTool}));

    ATH_CHECK(m_trackFitterTool.retrieve(EnableTool{m_doTracking}));
    ATH_CHECK(m_overlapRemovalTool.retrieve());
    ATH_CHECK(m_writeOutputTool.retrieve());
    ATH_CHECK(m_FPGATrackSimMapping.retrieve());
    ATH_CHECK(m_trackExtensionTool.retrieve());

    ATH_MSG_DEBUG("initialize() Instantiating root objects");

    // This file should only need to generate one output tool!
    m_logicEventOutputHeader = m_writeOutputTool->addOutputBranch(m_outputBranch.value(), true);

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

    ATH_MSG_DEBUG("initialize() Finished");

    return StatusCode::SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
//                          MAIN EXECUTE ROUTINE                             //
///////////////////////////////////////////////////////////////////////////////

StatusCode FPGATrackSimSecondStageAlg::execute()
{
#ifdef BENCHMARK_LOGICALHITSALG
    std::chrono::time_point<std::chrono::steady_clock> t_0, t_1;
    t_0 = std::chrono::steady_clock::now();
#endif

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

    // Query the event selection service to make sure this event passed cuts.
    if (!m_evtSel->getSelectedEvent()) {
        ATH_MSG_DEBUG("Event skipped by: " << m_evtSel->name());
        return StatusCode::SUCCESS;
    }

    // Event passes cuts, count it. technically, DataPrep does this now.
    m_evt++;

    // If we get here, FPGAHits_2nd is valid, copy it over.
    std::vector<std::shared_ptr<const FPGATrackSimHit>> phits_2nd;
    phits_2nd.reserve(FPGAHits->size());
    for (const auto& hit : *FPGAHits) {
        phits_2nd.push_back(std::make_shared<const FPGATrackSimHit>(hit));
    }

    std::vector<std::shared_ptr<const FPGATrackSimTrack>> tracks_1st;
    tracks_1st.reserve(FPGAInputTracks->size());
    for (const auto& track : *FPGAInputTracks) {
        tracks_1st.push_back(std::make_shared<const FPGATrackSimTrack>(track));
    }
    ATH_MSG_DEBUG("Retrieved " << phits_2nd.size() << " hits and " << tracks_1st.size() << " tracks from storegate");

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

    // Get second stage roads from tracks.
    std::vector<std::shared_ptr<const FPGATrackSimRoad>> prefilter_roads;
    std::vector<std::shared_ptr<const FPGATrackSimRoad>> roads = prefilter_roads;

    // Use the track extension tool to actually produce a new set of roads.
    ATH_CHECK(m_trackExtensionTool->extendTracks(phits_2nd, tracks_1st, roads));

    for (auto const &road : roads) {
        std::vector<FPGATrackSimHit> road_hits;
        ATH_MSG_DEBUG("Hough Road X Y: " << road->getX() << " " << road->getY());
        for (size_t l = 0; l < road->getNLayers(); ++l) {
            for (const auto &layerH : road->getHits(l)) {
                road_hits.push_back(*layerH);
            }
        }
        FPGAHitsInRoads_2nd->push_back(road_hits);
        FPGARoads_2nd->push_back(*road);
    }

    auto mon_nroads = Monitored::Scalar<unsigned>("nroads_2nd", roads.size());
    for (auto const &road : roads) {
      unsigned bitmask = road->getHitLayers();
      for (size_t l = 0; l < m_FPGATrackSimMapping->PlaneMap_2nd(0)->getNLogiLayers(); l++) {
        if (bitmask & (1 << l)) {
            auto mon_layerIDs = Monitored::Scalar<unsigned>("layerIDs_2nd",l);
            Monitored::Group(m_monTool,mon_layerIDs);
        }
      }
    }
    Monitored::Group(m_monTool, mon_nroads);

    TIME(m_troads);

    // NOTE: for now we don't support road filtering again in the second stage,
    // except for the special case of the spacepoint road filter tool. In principle filters
    // could be added here.

    // Spacepoint road filter tool. Needed when fitting to spacepoints.
    std::vector<std::shared_ptr<const FPGATrackSimRoad>> post_spfilter_roads;
    if (m_doSpacepoints) {
        ATH_CHECK(m_spRoadFilterTool->filterRoads(roads, post_spfilter_roads));
        roads = post_spfilter_roads;
    }
    auto mon_nroads_postfilter = Monitored::Scalar<unsigned>("nroads_2nd_postfilter", roads.size());
    Monitored::Group(m_monTool, mon_nroads_postfilter);

    TIME(m_troad_filter);

    // Get tracks, again, after extrapolation.
    // All of this code is effectively copied from LogicalHitsProcessAlg, except we use 2nd stage now.
    std::vector<FPGATrackSimTrack> tracks;
    if (m_doTracking) {
        if (m_doNNTrack) {
            ATH_MSG_DEBUG("Performing NN tracking");
            ATH_CHECK(m_NNTrackTool->getTracks(roads, tracks));
        }
        else {
            ATH_CHECK(m_trackFitterTool->getTracks(roads, tracks));
            float bestchi2 = 1.e15;
            for (const FPGATrackSimTrack& track : tracks) {
                float chi2 = track.getChi2ndof();
                if (chi2 < bestchi2) bestchi2 = chi2;
                auto mon_chi2 = Monitored::Scalar<float>("chi2_2nd_all", chi2);
                Monitored::Group(m_monTool, mon_chi2);
            }
            auto mon_best_chi2 = Monitored::Scalar<float>("best_chi2_2nd",bestchi2);
            Monitored::Group(m_monTool,mon_best_chi2);
        }
    }
    else { // we are not doing tracking so get the number of combinations for monitoring
        int ntrackDummy = 0;
        for (const std::shared_ptr<const FPGATrackSimRoad>& road : roads) {
            ntrackDummy += road->getNHitCombos();
        }
        tracks.resize(ntrackDummy); // just filled with dummy tracks for monitoring
    }
    auto mon_ntracks = Monitored::Scalar<unsigned>("ntrack_2nd", tracks.size());
    Monitored::Group(m_monTool,mon_ntracks);

    TIME(m_ttracks);

    // Overlap removal
    ATH_CHECK(m_overlapRemovalTool->runOverlapRemoval(tracks));
    unsigned ntrackOLRChi2 = 0;
    for (const FPGATrackSimTrack& track : tracks) {
        if (track.getChi2ndof() < m_trackScoreCut) {
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

    m_nRoadsTot += roads.size();
    m_nTracksTot += tracks.size();

    // Do some simple monitoring of efficiencies. okay, we need truth tracks here.
    std::vector<FPGATrackSimTruthTrack> truthtracks = *FPGATruthTracks;
    std::vector<FPGATrackSimOfflineTrack> offlineTracks = *FPGAOfflineTracks;
    if (truthtracks.size() > 0) {
        m_evt_truth++;
        auto passroad = Monitored::Scalar<bool>("eff_road_2nd",(roads.size() > 0));
        auto passtrack = Monitored::Scalar<bool>("eff_track_2nd",(tracks.size() > 0));
        auto truthpT_zoom = Monitored::Scalar<float>("pT_zoom_2nd",truthtracks.front().getPt()*0.001);
        auto truthpT = Monitored::Scalar<float>("pT_2nd",truthtracks.front().getPt()*0.001);
        auto trutheta = Monitored::Scalar<float>("eta_2nd",truthtracks.front().getEta());
        auto truthphi= Monitored::Scalar<float>("phi_2nd",truthtracks.front().getPhi());
        auto truthd0= Monitored::Scalar<float>("d0_2nd",truthtracks.front().getD0());
        auto truthz0= Monitored::Scalar<float>("z0_2nd",truthtracks.front().getZ0());
        if (roads.size() > 0) m_nRoadsFound++;
        bool passchi2 = false;
        bool passchi2OLR = false;
        if (tracks.size() > 0) {
            m_nTracksFound++;
            for (const auto& track : tracks) {
                if (track.getChi2ndof() < m_trackScoreCut) {
                    passchi2 = true;
                    if (track.passedOR()) {
                        passchi2OLR = true;
                        break;
                    }
                }
            }
        }
        if (passchi2) m_nTracksChi2Found++;
        if (passchi2OLR) m_nTracksChi2OLRFound++;
        auto passtrackchi2 = Monitored::Scalar<bool>("eff_track_chi2_2nd",passchi2);
        Monitored::Group(m_monTool,passroad,passtrack,truthpT_zoom,truthpT,trutheta,truthphi,truthd0,truthz0,passtrackchi2);
    }

    for (const FPGATrackSimTrack& track : tracks) FPGATracks_2ndHandle->push_back(track);

    TIME(m_tOR);

    // Write the output and reset
    if (m_writeOutputData)  {
        auto dataFlowInfo = std::make_unique<FPGATrackSimDataFlowInfo>();
        ATH_CHECK(writeOutputData(roads, tracks, dataFlowInfo.get()));
    }

    // This one we can do-- by passing in truth and offline tracks via storegate above.
    if (m_doHoughRootOutput) {
      ATH_CHECK(m_houghRootOutputTool->fillTree(roads, truthtracks, offlineTracks, phits_2nd, m_writeOutNonSPStripHits, m_trackScoreCut, m_NumOfHitPerGrouping));
    }

    // Reset data pointers
    m_logicEventOutputHeader->reset();

    TIME(m_tfin);

    return StatusCode::SUCCESS;
}


StatusCode FPGATrackSimSecondStageAlg::writeOutputData( const std::vector<std::shared_ptr<const FPGATrackSimRoad>>& roads_2nd,
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
#ifdef BENCHMARK_LOGICALHITSALG
    ATH_MSG_INFO("Timings:" <<
            "\nroads (2nd):       " << std::setw(10) << m_troads <<
            "\nroad filter (2nd): " << std::setw(10) << m_troad_filter <<
            "\ntracks (2nd):      " << std::setw(10) << m_ttracks <<
            "\nOR (2nd):          " << std::setw(10) << m_tOR <<
            "\nmon (2nd):         " << std::setw(10) << m_tmon <<
            "\nfin (@nd):         " << std::setw(10) << m_tfin
    );
#endif

    ATH_MSG_INFO("PRINTING FPGATRACKSIM SIMPLE STATS: SECOND STAGE");
    ATH_MSG_INFO("========================================================================================");
    ATH_MSG_INFO("Inclusive efficiency to find a road = " << m_nRoadsFound/(float)m_evt_truth);
    ATH_MSG_INFO("Inclusive efficiency to find a track = " << m_nTracksFound/(float)m_evt_truth);
    ATH_MSG_INFO("Inclusive efficiency to find a track passing chi2 = " << m_nTracksChi2Found/(float)m_evt_truth);
    ATH_MSG_INFO("Inclusive efficiency to find a track passing chi2 and OLR = " << m_nTracksChi2OLRFound/(float)m_evt_truth);


    ATH_MSG_INFO("Number of 2nd stage roads/event = " << m_nRoadsTot/(float)m_evt);
    ATH_MSG_INFO("Number of 2nd stage track combinations/event = " << m_nTracksTot/(float)m_evt);
    ATH_MSG_INFO("Number of 2nd stage tracks passing chi2/event = " << m_nTracksChi2Tot/(float)m_evt);
    ATH_MSG_INFO("Number of 2nd stage tracks passing chi2 and OLR/event = " << m_nTracksChi2OLRTot/(float)m_evt);
    ATH_MSG_INFO("========================================================================================");

    return StatusCode::SUCCESS;
}
