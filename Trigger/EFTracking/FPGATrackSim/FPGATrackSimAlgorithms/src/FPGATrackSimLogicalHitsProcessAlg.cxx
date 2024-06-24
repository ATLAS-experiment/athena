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

#include "FPGATrackSimDataFlowTool.h"
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


    ATH_CHECK(m_hitSGInputTool.retrieve(EnableTool{!m_hitSGInputTool.empty()}));

    ATH_CHECK(m_hitInputTool.retrieve(EnableTool{!m_hitInputTool.empty()}));
    ATH_CHECK(m_hitInputTool2.retrieve(EnableTool{m_secondInputToolN > 0 && !m_hitInputTool2.empty()}));
    ATH_CHECK(m_hitMapTool.retrieve());
    ATH_CHECK(m_hitFilteringTool.retrieve(EnableTool{m_doHitFiltering}));
    ATH_CHECK(m_clusteringTool.retrieve(EnableTool{m_clustering}));
    ATH_CHECK(m_spacepointsTool.retrieve(EnableTool{m_doSpacepoints}));
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
    ATH_CHECK(m_trackFitterTool_2nd.retrieve(EnableTool{m_runSecondStage && m_doTracking}));
    ATH_CHECK(m_overlapRemovalTool_2nd.retrieve(EnableTool{m_runSecondStage}));
    ATH_CHECK(m_dataFlowTool.retrieve());
    ATH_CHECK(m_writeOutputTool.retrieve());
    ATH_CHECK(m_FPGATrackSimMapping.retrieve());
    if ( m_doEvtSel ) {
        ATH_CHECK(m_evtSel.retrieve());
    }

    ATH_MSG_DEBUG("initialize() Instantiating root objects");
    m_logicEventHeader_1st   = m_writeOutputTool->getLogicalEventInputHeader_1st();
    

    m_logicEventOutputHeader = m_writeOutputTool->getLogicalEventOutputHeader();
    if (m_runSecondStage) m_logicEventHeader_2nd = m_writeOutputTool->getLogicalEventInputHeader_2nd();

    ATH_MSG_DEBUG("initialize() Setting branch");
    if (m_outputHitTxt) {
      ATH_MSG_INFO("writing road hits to " << m_outputHitTxtName);
      m_outputHitTxtStream.open(m_outputHitTxtName);
    }

    if (!m_monTool.empty())
        ATH_CHECK(m_monTool.retrieve());

    ATH_CHECK( m_FPGAClusterKey.initialize() );
    ATH_CHECK( m_FPGAClusterFilteredKey.initialize() );
    ATH_CHECK( m_FPGAHitKey.initialize() );
    ATH_CHECK( m_FPGAHitInRoadsKey.initialize() );
    ATH_CHECK( m_FPGAHitFilteredKey.initialize() );
    ATH_CHECK( m_FPGASpacePointsKey.initialize() );
    ATH_CHECK( m_FPGAHitUnmappedKey.initialize() );
    ATH_CHECK( m_FPGARoadKey.initialize() );
    ATH_CHECK( m_FPGATrackKey.initialize() );

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
    // Read inputs
    bool done = false;
    ATH_CHECK(readInputs(done));

    if (done) {
      IEventProcessor* appMgr = nullptr;
      ATH_CHECK(service("ApplicationMgr",appMgr));
      if (!appMgr) {
          ATH_MSG_ERROR("Failed to retrieve ApplicationMgr as IEventProcessor");
          return StatusCode::FAILURE;
      }
      return appMgr->stopRun();
    }

    // Apply truth track cuts
    if ( m_doEvtSel ){
        if (!m_evtSel->selectEvent(&m_eventHeader))
        {
            ATH_MSG_DEBUG("Event skipped by: " << m_evtSel->name());
            return StatusCode::SUCCESS;
        }
        ATH_MSG_DEBUG("Event accepted by: " << m_evtSel->name());
    } else {
        ATH_MSG_DEBUG("No Event Election applied");
    }
    TIME(m_tread);

    // Event passes cuts, count it
    m_evt++;
       
    // Map, cluster, and filter hits
    ATH_CHECK(processInputs());
    // Get reference to hits
    unsigned regionID = m_evtSel->getRegionID();
    // Recording Data
    auto mon_regionID = Monitored::Scalar<unsigned>("regionID", regionID);
    Monitored::Group(m_monTool, mon_regionID);

    TIME(m_tprocess);

    // Get roads
    std::vector<FPGATrackSimRoad*> prefilter_roads;
    std::vector<FPGATrackSimRoad*>& roads_1st = prefilter_roads;
    std::vector<const FPGATrackSimHit*> phits_1st;

    // If and when we set up code to run over more than one region/tower at a time this will need to be updated
    std::vector<FPGATrackSimHit> const & hits_1st = m_logicEventHeader_1st->towers().at(0).hits();

    for (FPGATrackSimHit const& h : hits_1st) { 
        if (h.isReal()) phits_1st.push_back(&h);
    }

    SG::WriteHandle<FPGATrackSimHitCollection> FPGAHits_1st (m_FPGAHitKey.at(0), ctx);
    ATH_CHECK( FPGAHits_1st.record (std::make_unique<FPGATrackSimHitCollection>()));
    for (const FPGATrackSimHit* Hit : phits_1st) FPGAHits_1st->push_back(*Hit);
 
    auto mon_nhits_1st = Monitored::Scalar<unsigned>("nHits_1st", hits_1st.size());
    auto mon_nhits_1st_unmapped = Monitored::Scalar<unsigned>("nHits_1st_unmapped", m_hits_1st_miss.size());
    Monitored::Group(m_monTool, mon_nhits_1st, mon_nhits_1st_unmapped);
    ATH_CHECK(m_roadFinderTool->getRoads(phits_1st, roads_1st));


    SG::WriteHandle<FPGATrackSimRoadCollection> FPGARoads_1st (m_FPGARoadKey, ctx);
    SG::WriteHandle<FPGATrackSimHitContainer> FPGAHitsInRoads_1st (m_FPGAHitInRoadsKey, ctx);

    ATH_CHECK( FPGARoads_1st.record (std::make_unique<FPGATrackSimRoadCollection>()));
    ATH_CHECK( FPGAHitsInRoads_1st.record (std::make_unique<FPGATrackSimHitContainer>()));
;
    for (const FPGATrackSimRoad *road:roads_1st){
        std::vector<FPGATrackSimHit> road_hits;
        ATH_MSG_DEBUG("Hough Road X Y: " << road->getX() << " " << road->getY());
        for (size_t l = 0; l < road->getNLayers(); ++l) {
            for (const FPGATrackSimHit* layerH : road->getHits(l)) {
                road_hits.push_back(*layerH);
            }
        }
        FPGAHitsInRoads_1st->push_back(road_hits);
        FPGARoads_1st->push_back(*road);
    }

    auto mon_nroads_1st = Monitored::Scalar<unsigned>("nroads_1st", roads_1st.size());
    for (FPGATrackSimRoad *road : roads_1st) {
      unsigned bitmask = road->getHitLayers();
      for (size_t l = 0; l < m_FPGATrackSimMapping->PlaneMap_1st()->getNLogiLayers(); l++) {
        if (bitmask & (1 << l)) {
            auto mon_layerIDs_1st = Monitored::Scalar<unsigned>("layerIDs_1st",l);
            Monitored::Group(m_monTool,mon_layerIDs_1st);
        }
      }
    }
    Monitored::Group(m_monTool, mon_nroads_1st);
    
    TIME(m_troads);
    // Standard road Filter
    std::vector<FPGATrackSimRoad*> postfilter_roads;
    if (m_filterRoads)
    {
        ATH_CHECK(m_roadFilterTool->filterRoads(roads_1st, postfilter_roads));
        roads_1st = postfilter_roads;
    }
    ATH_CHECK(m_overlapRemovalTool_1st->runOverlapRemoval(roads_1st));
    // Road Filter2
    std::vector<FPGATrackSimRoad*> postfilter2_roads;
    if (m_filterRoads2) {
        ATH_CHECK(m_roadFilterTool2->filterRoads(roads_1st, postfilter2_roads));
        roads_1st = postfilter2_roads;
    }
    // Spacepoint road filter tool. Needed when fitting to spacepoints.
    std::vector<FPGATrackSimRoad*> postfilter3_roads;
    if (m_doSpacepoints) {
        ATH_CHECK(m_spRoadFilterTool->filterRoads(roads_1st, postfilter3_roads));
        roads_1st = postfilter3_roads;
    }
    auto mon_nroads_1st_postfilter = Monitored::Scalar<unsigned>("nroads_1st_postfilter", roads_1st.size());
    Monitored::Group(m_monTool, mon_nroads_1st_postfilter);


    
    TIME(m_troad_filter);
    // Get tracks
    std::vector<FPGATrackSimTrack> tracks_1st;
    if (m_doTracking) {
        if (m_doNNTrack) {
            ATH_MSG_DEBUG("Performing NN tracking");
            const FPGATrackSimNNMap* nnMap = m_FPGATrackSimMapping->NNMap();
            ATH_CHECK(m_NNTrackTool->getTracks(roads_1st, tracks_1st, nnMap));
        }
        else {
	  ATH_CHECK(m_trackFitterTool_1st->getTracks(roads_1st, tracks_1st));
	  float bestchi2 = 1.e15;
	  for (const FPGATrackSimTrack& track : tracks_1st) {
	    float chi2 = track.getChi2ndof();
	    if (chi2 < bestchi2) bestchi2 = chi2;
	    auto mon_chi2_1st = Monitored::Scalar<float>("chi2_1st_all",chi2);
	    Monitored::Group(m_monTool,mon_chi2_1st);	    
	  }
	  auto mon_best_chi2_1st = Monitored::Scalar<float>("best_chi2_1st",bestchi2);
	  Monitored::Group(m_monTool,mon_best_chi2_1st);
        }
    }
    auto mon_ntracks_1st = Monitored::Scalar<unsigned>("ntrack_1st", tracks_1st.size());
    Monitored::Group(m_monTool,mon_ntracks_1st);
    
    TIME(m_ttracks);

    // Overlap removal
    ATH_CHECK(m_overlapRemovalTool_1st->runOverlapRemoval(tracks_1st));
    unsigned ntrackOLRChi2 = 0;
    for (auto track : tracks_1st) {
        if (track.getChi2ndof() < 10) {
            m_nTracksChi2Tot++;
	    if (track.passedOR()) {
	        ntrackOLRChi2++;
	        m_nTracksChi2OLRTot++;
	    }
	}
    }
    auto mon_ntracks_1st_olr = Monitored::Scalar<unsigned>("ntrack_1st_afterOLR", ntrackOLRChi2);
    Monitored::Group(m_monTool,mon_ntracks_1st_olr);
    
    m_nRoadsTot += roads_1st.size();
    m_nTracksTot += tracks_1st.size();
    
    // Do some simple monitoring of efficiencies
    std::vector<FPGATrackSimTruthTrack> const & truthtracks = m_logicEventHeader_1st->optional().getTruthTracks();    
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
	    for (auto track : tracks_1st) {
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

    SG::WriteHandle<FPGATrackSimTrackCollection> FPGATracks_1stHandle (m_FPGATrackKey, ctx);
    ATH_CHECK(FPGATracks_1stHandle.record (std::make_unique<FPGATrackSimTrackCollection>()));
    for (const FPGATrackSimTrack& track : tracks_1st) FPGATracks_1stHandle->push_back(track);
   
    TIME(m_tOR);

    // Now, we may want to do large-radius tracking on the hits not used by the first stage tracking.
    // This follows overlap removal.
    std::vector<FPGATrackSimRoad*> roadsLRT;
    std::vector<FPGATrackSimTrack> tracksLRT; // currently empty
    if (m_doLRT) {
        // Filter out hits that are on successful first-stage tracks
        std::vector<const FPGATrackSimHit*> remainingHits;

        if (m_doLRTHitFiltering) {
            ATH_MSG_DEBUG("Doing hit filtering based on prompt tracks.");
            ATH_CHECK(m_LRTRoadFilterTool->filterUsedHits(tracks_1st, phits_1st, remainingHits));
            SG::WriteHandle<FPGATrackSimHitCollection> FPGAHitsFiltered_1st (m_FPGAHitFilteredKey, ctx);
            ATH_CHECK( FPGAHitsFiltered_1st.record (std::make_unique<FPGATrackSimHitCollection>()));

            for (const FPGATrackSimHit* Hit : remainingHits) FPGAHitsFiltered_1st->push_back(*Hit); 

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

    // Second stage fitting
    std::vector<FPGATrackSimRoad*> roads_2nd;
    std::vector<FPGATrackSimTrack> tracks_2nd;
    if (m_runSecondStage) {
        ATH_CHECK(secondStageProcessing(tracks_1st, roads_2nd, tracks_2nd));
        TIME(m_t2ndStage);
    }

    // Calculate data flow quantities
    if (m_writeOutputData)  {
      ATH_CHECK(m_dataFlowTool->calculateDataFlow(dataFlowInfo.get(), m_logicEventHeader_1st, m_clusters_1st, roads_1st, tracks_1st, roads_2nd, tracks_2nd));
    }
    
    // Write the output and reset
    ATH_CHECK(writeOutputData(roads_1st, tracks_1st, roads_2nd, tracks_2nd, dataFlowInfo.get()));

    if (m_doHoughRootOutput) {
      ATH_CHECK(m_houghRootOutputTool->fillTree(roads_1st, m_logicEventHeader_1st->optional().getTruthTracks(), m_logicEventHeader_1st->optional().getOfflineTracks()));
    }

    // dump hit identifiers per road/track to text file
    if(m_outputHitTxt) {

      ATH_MSG_DEBUG("Dumping hits in roads to text file " << m_outputHitTxtName);
      m_outputHitTxtStream << std::endl << "event " << m_ev << ", "
                           << roads_1st.size() << " roads"; // endl moved below to deal with trailing comma

      // m_clusters_original exists, we just need to know which ones to access
      // FPGATrackSimRoad::getHits returns a vector of FPGATrackSimHit, but each of them is actually a cluster
      // m_parentageMask has been co-opted to store the cluster index (set in clustering tool)
      // this lets us get hold of the actual FPGATrackSimCluster via m_clusters_original[index]

      int roadCount = 0;
      for(auto &road : roads_1st) {
        // to avoid empty entries if a layer has no hits in a road, save the hits to vector first
        std::vector< std::vector<FPGATrackSimHit> > clusters_in_road;
        for(int layer = 0; layer < int(road->getNLayers()); layer ++) {
          for(auto &cluster_as_hit : road->getHits(layer)) {
            unsigned clusterIndex = (unsigned)cluster_as_hit->getParentageMask();
            FPGATrackSimCluster thisCluster = m_clusters_1st_original.at(clusterIndex);

            // check that they match (might slow things down unnecessarily - remove later perhaps)
            if( (cluster_as_hit->getR() != thisCluster.getClusterEquiv().getR()) ||
                (cluster_as_hit->getZ() != thisCluster.getClusterEquiv().getZ()) ||
                (cluster_as_hit->getGPhi() != thisCluster.getClusterEquiv().getGPhi()) ){
              ATH_MSG_ERROR("cluster retrieved from index seems not to match");
              ATH_MSG_DEBUG("road ID " << road->getRoadID() << ", layer " << layer
			    << ", cluster index " << clusterIndex << "road.getHits(layer):  " << cluster_as_hit->getR() << " "
			    << cluster_as_hit->getZ() << " " << cluster_as_hit->getGPhi());
              ATH_MSG_DEBUG("m_cluster[index]: " << thisCluster.getClusterEquiv().getR() << " "
			    << thisCluster.getClusterEquiv().getZ() << " "
			    << thisCluster.getClusterEquiv().getGPhi());
            }
            clusters_in_road.push_back(thisCluster.getHitList());
          }
        }

        if(roadCount != 0) m_outputHitTxtStream << ",";
        m_outputHitTxtStream << std::endl << "[ ";
        bool firstCluster = true;
        for(auto &clusterHits : clusters_in_road) {
          if(firstCluster) firstCluster = false;
          else             m_outputHitTxtStream << ", ";
          bool firstHit = true;
          m_outputHitTxtStream << "[ ";
          for(auto &hit : clusterHits) {
            if(firstHit) firstHit = false;
            else         m_outputHitTxtStream << ", ";

            m_outputHitTxtStream << "[" << hit.isStrip() << ", " << hit.getIdentifierHash() << ", "
                                 << hit.getEtaIndex() << ", " << hit.getPhiIndex() << "]";
          }
          m_outputHitTxtStream << " ]";
        }
        m_outputHitTxtStream << " ]";
        roadCount++;
      }
    }


    // Reset data pointers
    m_eventHeader.reset();
    m_logicEventHeader_1st->reset();
    m_logicEventOutputHeader->reset();
    if (m_runSecondStage) m_logicEventHeader_2nd->reset();

    TIME(m_tfin);

    return StatusCode::SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
//                  INPUT PASSING, READING AND PROCESSING                    //
///////////////////////////////////////////////////////////////////////////////

StatusCode FPGATrackSimLogicalHitsProcessAlg::readInputs(bool & done)
{

    if ( !m_hitSGInputTool.empty()) {
        ATH_CHECK(m_hitSGInputTool->readData(&m_eventHeader, Gaudi::Hive::currentContext()));
        ATH_MSG_DEBUG("Loaded " << m_eventHeader.nHits() << " hits in event header from SG");

        return StatusCode::SUCCESS;
    }

    if (m_ev % m_firstInputToolN == 0)
    {
        // Read primary input
        ATH_CHECK(m_hitInputTool->readData(&m_firstInputHeader, done));
        if (done)
        {
            ATH_MSG_INFO("Cannot read more events from file, returning");
            return StatusCode::SUCCESS; // end of loop over events
        }
    }

    m_eventHeader = m_firstInputHeader;

    // Read secondary input
    for (int i = 0; i < m_secondInputToolN; i++)
    {
        ATH_CHECK(m_hitInputTool2->readData(&m_eventHeader, done, false));
        if (done)
        {
            ATH_MSG_INFO("Cannot read more events from file, returning");
            return StatusCode::SUCCESS;
        }
    }

    m_ev++;

    return StatusCode::SUCCESS;
}


// Applies clustering, mapping, hit filtering, and space points
StatusCode FPGATrackSimLogicalHitsProcessAlg::processInputs()
{
    m_clusters_1st.clear();
    m_spacepoints_1st.clear();
    m_hits_1st_miss.clear();

    const EventContext& ctx = getContext();

    // Map hits
    ATH_MSG_DEBUG("Running hits conversion");
    m_logicEventHeader_1st->reset();
    ATH_CHECK(m_hitMapTool->convert(1, m_eventHeader, *m_logicEventHeader_1st));
    if (!m_runSecondStage) m_eventHeader.clearHits();

    ATH_CHECK(m_hitMapTool->getUnmapped(m_hits_1st_miss));
    SG::WriteHandle<FPGATrackSimHitCollection> FPGAHitUnmapped_1st (m_FPGAHitUnmappedKey, ctx);
    ATH_CHECK( FPGAHitUnmapped_1st.record (std::make_unique<FPGATrackSimHitCollection>()));
    for (const FPGATrackSimHit& hit : m_hits_1st_miss) FPGAHitUnmapped_1st->push_back(hit); 



    ATH_MSG_DEBUG("Hits conversion done");
    // Random removal of hits
    if (m_doHitFiltering) {
        ATH_MSG_DEBUG("Running hits filtering");
        ATH_CHECK(m_hitFilteringTool->DoRandomRemoval(*m_logicEventHeader_1st, true));
    }

    // Clustering
    if (m_clustering)
    {
        ATH_MSG_DEBUG("Running clustering");
        ATH_CHECK(m_clusteringTool->DoClustering(*m_logicEventHeader_1st, m_clusters_1st));
        m_clusters_1st_original = m_clusters_1st;

        // I think I also want to pass m_clusters to random removal (but won't work currently)
        if (m_doHitFiltering) ATH_CHECK(m_hitFilteringTool->DoRandomRemoval(*m_logicEventHeader_1st, false));

        SG::WriteHandle<FPGATrackSimClusterCollection> FPGAClusters_1st (m_FPGAClusterKey.at(0), ctx);
        ATH_CHECK( FPGAClusters_1st.record (std::make_unique<FPGATrackSimClusterCollection>()));
        for (const FPGATrackSimCluster& cluster : m_clusters_1st_original) FPGAClusters_1st->push_back(cluster); 

    }

    // Filter hits/clusters (untested for hits, ie with m_clustering = false)
    if (m_doHitFiltering)
    {
        // get the sets of layers that we want to filter hits from
        std::vector<int> filter_pixel_physLayers, filter_strip_physLayers;
        const FPGATrackSimPlaneMap *planeMap_1st = m_FPGATrackSimMapping->PlaneMap_1st();
        ATH_CHECK(m_hitFilteringTool->GetPairedStripPhysLayers(planeMap_1st, filter_strip_physLayers));
        m_clusters_1st.clear();
        ATH_CHECK(m_hitFilteringTool->DoHitFiltering(*m_logicEventHeader_1st, filter_pixel_physLayers, filter_strip_physLayers, m_clusters_1st));
 
        SG::WriteHandle<FPGATrackSimClusterCollection> FPGAClustersFiltered_1st (m_FPGAClusterFilteredKey, ctx);
        ATH_CHECK( FPGAClustersFiltered_1st.record (std::make_unique<FPGATrackSimClusterCollection>()));
        for (const FPGATrackSimCluster &cluster : m_clusters_1st) FPGAClustersFiltered_1st->push_back(cluster); 
 
    }

    // Space points
    if (m_doSpacepoints) {
        ATH_CHECK(m_spacepointsTool->DoSpacePoints(*m_logicEventHeader_1st, m_spacepoints_1st));

        SG::WriteHandle<FPGATrackSimClusterCollection> FPGASpacePoints_1st (m_FPGASpacePointsKey.at(0), ctx);
        ATH_CHECK( FPGASpacePoints_1st.record (std::make_unique<FPGATrackSimClusterCollection>()));
        for (const FPGATrackSimCluster& cluster : m_spacepoints_1st) FPGASpacePoints_1st->push_back(cluster); 
    }

    return StatusCode::SUCCESS;
}


StatusCode FPGATrackSimLogicalHitsProcessAlg::secondStageProcessing(std::vector<FPGATrackSimTrack> const & tracks_1st,
                                                           std::vector<FPGATrackSimRoad*> & roads_2nd, std::vector<FPGATrackSimTrack> & tracks_2nd)
{
    m_clusters_2nd.clear();
    m_spacepoints_2nd.clear();
    m_hits_2nd_miss.clear();


    const EventContext& ctx = getContext();

    // Map hits
    m_logicEventHeader_2nd->reset();
    ATH_CHECK(m_hitMapTool->convert(2, m_eventHeader, *m_logicEventHeader_2nd));
    m_eventHeader.clearHits();

    if (m_clustering) ATH_CHECK(m_clusteringTool->DoClustering(*m_logicEventHeader_2nd, m_clusters_2nd));

    SG::WriteHandle<FPGATrackSimClusterCollection> FPGAClusters_2nd (m_FPGAClusterKey.at(1), ctx);
    ATH_CHECK( FPGAClusters_2nd.record (std::make_unique<FPGATrackSimClusterCollection>()));
    for (const FPGATrackSimCluster& cluster : m_clusters_2nd) FPGAClusters_2nd->push_back(cluster); 

    // Space points
    if (m_doSpacepoints) {
        ATH_CHECK(m_spacepointsTool->DoSpacePoints(*m_logicEventHeader_2nd, m_spacepoints_2nd));

        SG::WriteHandle<FPGATrackSimClusterCollection> FPGASpacePoints_2nd (m_FPGASpacePointsKey.at(1), ctx);
        ATH_CHECK( FPGASpacePoints_2nd.record (std::make_unique<FPGATrackSimClusterCollection>()));
        for (const FPGATrackSimCluster& cluster : m_spacepoints_2nd) FPGASpacePoints_2nd->push_back(cluster); 
    }

    std::vector<FPGATrackSimHit> const & hits_2nd = m_logicEventHeader_2nd->towers().at(0).hits();

    ATH_MSG_DEBUG("Number of second stage hits = " << hits_2nd.size());

    std::vector<const FPGATrackSimHit*> phits_2nd;
    for (FPGATrackSimHit const & h : hits_2nd) phits_2nd.push_back(&h);

    SG::WriteHandle<FPGATrackSimHitCollection> FPGAHits_2nd (m_FPGAHitKey.at(1), ctx);
    ATH_CHECK( FPGAHits_2nd.record (std::make_unique<FPGATrackSimHitCollection>()));
    for (const FPGATrackSimHit* Hit : phits_2nd) FPGAHits_2nd->push_back(*Hit); 

    // Get the first stage tracks after OR
    std::vector<FPGATrackSimTrack> tracks_1st_OR;
    for (auto track : tracks_1st) {
        if (track.passedOR() == 1) tracks_1st_OR.push_back(track);
    }

    ATH_MSG_DEBUG("Number of roads for second stage fitting " << roads_2nd.size());

    // Get second stage tracks
    if (m_doTracking) ATH_CHECK(m_trackFitterTool_2nd->getTracks(roads_2nd, tracks_2nd));

    ATH_MSG_DEBUG("Number of tracks from the second stage fitting = " << tracks_2nd.size() << " and truth tracks = " << m_logicEventHeader_1st->optional().getTruthTracks().size());

    // Overlap removal
    ATH_CHECK(m_overlapRemovalTool_2nd->runOverlapRemoval(tracks_2nd));

    return StatusCode::SUCCESS;
}

StatusCode FPGATrackSimLogicalHitsProcessAlg::writeOutputData(std::vector<FPGATrackSimRoad*> const & roads_1st, std::vector<FPGATrackSimTrack> const & tracks_1st,
                                                     std::vector<FPGATrackSimRoad*> const & roads_2nd, std::vector<FPGATrackSimTrack> const & tracks_2nd,
                                                     FPGATrackSimDataFlowInfo const * dataFlowInfo)
{
  m_logicEventOutputHeader->reset();
  
  ATH_MSG_DEBUG("NFPGATrackSimRoads_1st = " << roads_1st.size() << ", NFPGATrackSimTracks_1st = " << tracks_1st.size() << ", NFPGATrackSimRoads_2nd = " << roads_2nd.size() << ", NFPGATrackSimTracks_2nd = " << tracks_2nd.size());
  
  if (!m_writeOutputData) return StatusCode::SUCCESS;
  m_logicEventOutputHeader->reserveFPGATrackSimRoads_1st(roads_1st.size());
  m_logicEventOutputHeader->addFPGATrackSimRoads_1st(roads_1st);
  if (m_doTracking) {
    m_logicEventOutputHeader->reserveFPGATrackSimTracks_1st(tracks_1st.size());
    m_logicEventOutputHeader->addFPGATrackSimTracks_1st(tracks_1st);
  }

  
  if (m_runSecondStage) {
    m_logicEventOutputHeader->reserveFPGATrackSimRoads_2nd(roads_2nd.size());
    m_logicEventOutputHeader->reserveFPGATrackSimTracks_2nd(tracks_2nd.size());
    
    m_logicEventOutputHeader->addFPGATrackSimRoads_2nd(roads_2nd);
    m_logicEventOutputHeader->addFPGATrackSimTracks_2nd(tracks_2nd);
  }

  m_logicEventOutputHeader->setDataFlowInfo(*dataFlowInfo);
  ATH_MSG_DEBUG(m_logicEventOutputHeader->getDataFlowInfo());

  ATH_CHECK(m_writeOutputTool->writeData(m_logicEventHeader_1st, m_logicEventHeader_2nd, m_logicEventOutputHeader));



  return StatusCode::SUCCESS;
}




///////////////////////////////////////////////////////////////////////////////
// Finalize

StatusCode FPGATrackSimLogicalHitsProcessAlg::finalize()
{
#ifdef BENCHMARK_LOGICALHITSALG
    ATH_MSG_INFO("Timings:" <<
            "\nread input:   " << std::setw(10) << m_tread <<
            "\nprocess hits: " << std::setw(10) << m_tprocess <<
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
    
    
    if (m_outputHitTxt) m_outputHitTxtStream.close();

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
