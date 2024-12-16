// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#include "FPGATrackSimLogicalHitsProcessAlg.h"

#include "FPGATrackSimObjects/FPGATrackSimCluster.h"
#include "FPGATrackSimObjects/FPGATrackSimHit.h"
#include "FPGATrackSimObjects/FPGATrackSimDataFlowInfo.h"
#include "FPGATrackSimObjects/FPGATrackSimRoad.h"
#include "FPGATrackSimObjects/FPGATrackSimTrack.h"
#include "FPGATrackSimObjects/FPGATrackSimLogicalEventOutputHeader.h"
#include "FPGATrackSimObjects/FPGATrackSimLogicalEventInputHeader.h"
#include "FPGATrackSimObjects/FPGATrackSimTrackPars.h"

#include "FPGATrackSimNNTrackTool.h"
#include "FPGATrackSimOverlapRemovalTool.h"
#include "FPGATrackSimTrackFitterTool.h"

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

size_t m_tread = 0, m_tprocess = 0, m_troads = 0, m_troad_filter = 0, m_tlrt = 0, m_ttracks = 0, m_tOR = 0, m_t2ndStage = 0, m_tmon = 0, m_tfin = 0;
#else
#define TIME(name)
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
    ATH_CHECK(m_houghRootOutputTool.retrieve(EnableTool{m_doHoughRootOutput}));
    ATH_CHECK(m_NNTrackTool.retrieve(EnableTool{m_doNNTrack}));
    ATH_CHECK(m_roadFilterTool.retrieve(EnableTool{m_filterRoads}));
    ATH_CHECK(m_roadFilterTool2.retrieve(EnableTool{m_filterRoads2}));
    if (m_doSpacepoints) ATH_CHECK(m_spRoadFilterTool.retrieve(EnableTool{m_spRoadFilterTool}));

    ATH_CHECK(m_trackFitterTool_1st.retrieve(EnableTool{m_doTracking}));
    ATH_CHECK(m_overlapRemovalTool_1st.retrieve());
    ATH_CHECK(m_writeOutputTool.retrieve());
    ATH_CHECK(m_FPGATrackSimMapping.retrieve());

    ATH_MSG_DEBUG("initialize() Instantiating root objects");

    // This file should only need to generate one input and output branch.
    m_slicedHitHeader = m_writeOutputTool->addInputBranch(m_sliceBranch.value(), true);
    m_logicEventOutputHeader = m_writeOutputTool->addOutputBranch(m_outputBranch.value(), true);

    // Connect the road union tool accordingly.
    m_roadFinderTool->setupSlices(m_slicedHitHeader);

    ATH_MSG_DEBUG("initialize() Setting branch");

    if (!m_monTool.empty())
        ATH_CHECK(m_monTool.retrieve());

    ATH_CHECK( m_FPGAHitInRoadsKey.initialize() );
    ATH_CHECK( m_FPGAHitFilteredKey.initialize() );
    ATH_CHECK( m_FPGARoadKey.initialize() );
    ATH_CHECK( m_FPGATrackKey.initialize() );
    ATH_CHECK( m_inputTruthParticleContainerKey.initialize(m_runOnRDO) );
    ATH_CHECK( m_FPGAHitKey.initialize() );
    ATH_CHECK( m_FPGATruthTrackKey.initialize() );
    ATH_CHECK( m_FPGAOfflineTrackKey.initialize() );

    ATH_MSG_DEBUG("initialize() Finished");

    return StatusCode::SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
//                          MAIN EXECUTE ROUTINE                             //
///////////////////////////////////////////////////////////////////////////////

StatusCode FPGATrackSimLogicalHitsProcessAlg::execute()
{
#ifdef BENCHMARK_LOGICALHITSALG
    std::chrono::time_point<std::chrono::steady_clock> t_0, t_1;
    t_0 = std::chrono::steady_clock::now();
#endif
    const EventContext& ctx = getContext();

    // Get reference to hits from StoreGate.
    // Hits have been procesed by the DataPrep algorithm. Now, we need to read them.
    // If they aren't passed, assume this means we are done.
    SG::ReadHandle<FPGATrackSimHitCollection> FPGAHits(m_FPGAHitKey.at(0), ctx);
    if (!FPGAHits.isValid()) {
        if (m_evt == 0) {
            ATH_MSG_WARNING("Didn't receive FPGAHits_1st on first event; assuming no input events.");
        }
        SmartIF<IEventProcessor> appMgr{service("ApplicationMgr")};
        if (!appMgr) {
            ATH_MSG_ERROR("Failed to retrieve ApplicationMgr as IEventProcessor");
            return StatusCode::FAILURE;
        }
        return appMgr->stopRun();
    }
 
    // Set up write handles.
    SG::WriteHandle<FPGATrackSimRoadCollection> FPGARoads_1st (m_FPGARoadKey, ctx);
    SG::WriteHandle<FPGATrackSimHitContainer> FPGAHitsInRoads_1st (m_FPGAHitInRoadsKey, ctx);

    ATH_CHECK( FPGARoads_1st.record (std::make_unique<FPGATrackSimRoadCollection>()));
    ATH_CHECK( FPGAHitsInRoads_1st.record (std::make_unique<FPGATrackSimHitContainer>()));

    SG::WriteHandle<FPGATrackSimTrackCollection> FPGATracks_1stHandle (m_FPGATrackKey, ctx);
    ATH_CHECK(FPGATracks_1stHandle.record (std::make_unique<FPGATrackSimTrackCollection>()));

    SG::WriteHandle<FPGATrackSimHitCollection> FPGAHitsFiltered_1st (m_FPGAHitFilteredKey, ctx);
    ATH_CHECK( FPGAHitsFiltered_1st.record (std::make_unique<FPGATrackSimHitCollection>()));
 
    // Query the event selection service to make sure this event passed cuts.
    if (!m_evtSel->getSelectedEvent()) {
        ATH_MSG_DEBUG("Event skipped by: " << m_evtSel->name());
        return StatusCode::SUCCESS;
    }
 
    // Event passes cuts, count it. technically, DataPrep does this now.
    m_evt++;

    // If we get here, FPGAHits_1st is valid, copy it over.
    std::vector<std::shared_ptr<const FPGATrackSimHit>> phits_1st;
    phits_1st.reserve(FPGAHits->size());
    for (const auto& hit : *FPGAHits) {
        phits_1st.push_back(std::make_shared<const FPGATrackSimHit>(hit));
    }

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
 
    // Get roads
    std::vector<std::shared_ptr<const FPGATrackSimRoad>> prefilter_roads;
    std::vector<std::shared_ptr<const FPGATrackSimRoad>> roads_1st = prefilter_roads;
    ATH_CHECK(m_roadFinderTool->getRoads(phits_1st, roads_1st, *FPGATruthTracks));

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
    
    TIME(m_troads);
    // Standard road Filter
    std::vector<std::shared_ptr<const FPGATrackSimRoad>> postfilter_roads;
    if (m_filterRoads)
    {
        ATH_CHECK(m_roadFilterTool->filterRoads(roads_1st, postfilter_roads));
        roads_1st = postfilter_roads;
    }
    if (m_doOverlapRemoval) ATH_CHECK(m_overlapRemovalTool_1st->runOverlapRemoval(roads_1st));       
    // Road Filter2
    std::vector<std::shared_ptr<const FPGATrackSimRoad>> postfilter2_roads;
    if (m_filterRoads2) {   
        ATH_CHECK(m_roadFilterTool2->filterRoads(roads_1st, postfilter2_roads));
        roads_1st = postfilter2_roads;
    }

    auto mon_nroads_1st_postfilter = Monitored::Scalar<unsigned>("nroads_1st_postfilter", roads_1st.size());
    Monitored::Group(m_monTool, mon_nroads_1st_postfilter);
    
    TIME(m_troad_filter);
    // Get tracks
    std::vector<FPGATrackSimTrack> tracks_1st;
    if (m_doTracking) {
        if (m_doNNTrack) {
            ATH_MSG_DEBUG("Performing NN tracking");
            ATH_CHECK(m_NNTrackTool->getTracks(roads_1st, tracks_1st));
        }
        else {
            ATH_MSG_DEBUG("Performing Linear tracking");
            ATH_CHECK(m_trackFitterTool_1st->getTracks(roads_1st, tracks_1st));
            float bestchi2 = 1.e15;
            for (const FPGATrackSimTrack& track : tracks_1st) {
                float chi2 = track.getChi2ndof();
                if (chi2 < bestchi2) bestchi2 = chi2;
                auto mon_chi2_1st = Monitored::Scalar<float>("chi2_1st_all", chi2);
                Monitored::Group(m_monTool, mon_chi2_1st);
            }
            auto mon_best_chi2_1st = Monitored::Scalar<float>("best_chi2_1st",bestchi2);
            Monitored::Group(m_monTool,mon_best_chi2_1st);
        }
    }
    else { // we are not doing tracking so get the number of combinations for monitoring
        ATH_MSG_DEBUG("No tracking. Adding dummy tracks...");
      int ntrackDummy = 0;
      for (const std::shared_ptr<const FPGATrackSimRoad>& road : roads_1st) {
	ntrackDummy += road->getNHitCombos();
      }
      tracks_1st.resize(ntrackDummy); // just filled with dummy tracks for monitoring
    }

    // loop over roads and store them in SG (This better be done after track finding to also copy the sector information)
    for (auto const &road:roads_1st){
        std::vector<FPGATrackSimHit> road_hits;
        ATH_MSG_DEBUG("Hough Road X Y: " << road->getX() << " " << road->getY());
        for (size_t l = 0; l < road->getNLayers(); ++l) {
            for (const auto &layerH : road->getHits(l)) {
                road_hits.push_back(*layerH);
            }
        }
        FPGAHitsInRoads_1st->push_back(road_hits);
        FPGARoads_1st->push_back(*road);
    }


    auto mon_ntracks_1st = Monitored::Scalar<unsigned>("ntrack_1st", tracks_1st.size());
    Monitored::Group(m_monTool,mon_ntracks_1st);
    
    TIME(m_ttracks);

    // Overlap removal
    if (m_doOverlapRemoval)  ATH_CHECK(m_overlapRemovalTool_1st->runOverlapRemoval(tracks_1st));
    unsigned ntrackOLRChi2 = 0;
    for (const FPGATrackSimTrack& track : tracks_1st) {
        if (track.getChi2ndof() < m_trackScoreCut) {
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
    auto mon_ntracks_1st_olr = Monitored::Scalar<unsigned>("ntrack_1st_afterOLR", ntrackOLRChi2);
    Monitored::Group(m_monTool,mon_ntracks_1st_olr);
    
    m_nRoadsTot += roads_1st.size();
    m_nTracksTot += tracks_1st.size();
    
    // Do some simple monitoring of efficiencies. okay, we need truth tracks here.
    std::vector<FPGATrackSimTruthTrack> truthtracks = *FPGATruthTracks;
    std::vector<FPGATrackSimOfflineTrack> offlineTracks = *FPGAOfflineTracks;
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
        bool passchi2 = false;
        bool passchi2OLR = false;
        if (tracks_1st.size() > 0) {
            m_nTracksFound++;
            for (const auto& track : tracks_1st) {
                if (track.getChi2ndof() < 10) {
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
        auto passtrackchi2 = Monitored::Scalar<bool>("eff_track_chi2",passchi2);
        Monitored::Group(m_monTool,passroad,passtrack,truthpT_zoom,truthpT,trutheta,truthphi,truthd0,truthz0,passtrackchi2);
    }

    for (const FPGATrackSimTrack& track : tracks_1st) FPGATracks_1stHandle->push_back(track);
   
    TIME(m_tOR);

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

    TIME(m_tlrt);

    auto dataFlowInfo = std::make_unique<FPGATrackSimDataFlowInfo>();

    // Write the output and reset
    if (m_writeOutputData)  {
        ATH_CHECK(writeOutputData(roads_1st, tracks_1st, dataFlowInfo.get()));
    }

    // This one we can do-- by passing in truth and offline tracks via storegate above.
    if (m_doHoughRootOutput) {
      ATH_CHECK(m_houghRootOutputTool->fillTree(roads_1st, truthtracks, offlineTracks, phits_1st, m_writeOutNonSPStripHits, m_trackScoreCut, m_NumOfHitPerGrouping));
    }

    // Reset data pointers
    m_slicedHitHeader->reset();
    m_logicEventOutputHeader->reset();

    TIME(m_tfin);
    
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
#ifdef BENCHMARK_LOGICALHITSALG
    ATH_MSG_INFO("Timings:" <<
            "\nroads:        " << std::setw(10) << m_troads <<
            "\nroad filter:  " << std::setw(10) << m_troad_filter <<
            "\nllp:          " << std::setw(10) << m_tlrt <<
            "\ntracks:       " << std::setw(10) << m_ttracks <<
            "\nOR:           " << std::setw(10) << m_tOR <<
            (m_runSecondStage ? : ("\n2ndStage:           " << std::setw(10) << m_t2ndStage) : "") <<
            "\nmon:          " << std::setw(10) << m_tmon <<
            "\nfin:          " << std::setw(10) << m_tfin
    );
#endif
    
    
    ATH_MSG_INFO("PRINTING FPGATRACKSIM SIMPLE STATS");
    ATH_MSG_INFO("========================================================================================");    
    ATH_MSG_INFO("Inclusive efficiency to find a road = " << m_nRoadsFound/m_evt_truth);
    ATH_MSG_INFO("Inclusive efficiency to find a track = " << m_nTracksFound/m_evt_truth);
    ATH_MSG_INFO("Inclusive efficiency to find a track passing chi2 = " << m_nTracksChi2Found/m_evt_truth);
    ATH_MSG_INFO("Inclusive efficiency to find a track passing chi2 and OLR = " << m_nTracksChi2OLRFound/m_evt_truth);

    
    ATH_MSG_INFO("Number of 1st stage roads/event = " << m_nRoadsTot/m_evt);
    ATH_MSG_INFO("Number of 1st stage track combinations/event = " << m_nTracksTot/m_evt);
    ATH_MSG_INFO("Number of 1st stage tracks passing chi2/event = " << m_nTracksChi2Tot/m_evt);
    ATH_MSG_INFO("Number of 1st stage tracks passing chi2 and OLR/event = " << m_nTracksChi2OLRTot/m_evt);    
    ATH_MSG_INFO("========================================================================================");
    
    return StatusCode::SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
// Helpers

void FPGATrackSimLogicalHitsProcessAlg::printHitSubregions(std::vector<FPGATrackSimHit> const & hits)
{
    ATH_MSG_WARNING("Hit regions:");
    for (auto hit : hits)
    {
        std::vector<uint32_t> regions = m_FPGATrackSimMapping->SubRegionMap()->getRegions(hit);
        std::stringstream ss;
        for (auto r : regions)
            ss << r << ",";
        ATH_MSG_WARNING("\t[" << ss.str() << "]");
    }
}
