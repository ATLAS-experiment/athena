// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#include "FPGATrackSimDataPrepAlg.h"

#include "FPGATrackSimObjects/FPGATrackSimCluster.h"
#include "FPGATrackSimObjects/FPGATrackSimHit.h"
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

size_t m_tread = 0;
size_t m_tprocess = 0;
size_t m_tfin = 0;
#else
#define TIME(name)
#endif


///////////////////////////////////////////////////////////////////////////////
// Initialize

FPGATrackSimDataPrepAlg::FPGATrackSimDataPrepAlg (const std::string& name, ISvcLocator* pSvcLocator) :
    AthAlgorithm(name, pSvcLocator)
{
}


StatusCode FPGATrackSimDataPrepAlg::initialize()
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
    ATH_CHECK(m_clusteringTool.retrieve(EnableTool{m_clustering > 0}));
    ATH_CHECK(m_spacepointsTool.retrieve(EnableTool{m_doSpacepoints}));
    
    ATH_CHECK(m_writeOutputTool.retrieve());
    ATH_CHECK(m_FPGATrackSimMapping.retrieve());
    if ( m_doEvtSel ) {
        ATH_CHECK(m_evtSel.retrieve());
    }

    ATH_MSG_DEBUG("initialize() Instantiating root objects");
    m_logicEventHeader_precluster = m_writeOutputTool->addInputBranch(m_preClusterBranch.value(), true);
    m_logicEventHeader = m_writeOutputTool->addInputBranch(m_postClusterBranch.value(), true);
    
    ATH_MSG_DEBUG("initialize() Setting branch");

    if (!m_monTool.empty())
        ATH_CHECK(m_monTool.retrieve());

    ATH_CHECK( m_FPGAClusterKey.initialize() );
    ATH_CHECK( m_FPGAClusterFilteredKey.initialize() );
    ATH_CHECK( m_FPGAHitKey.initialize() );
    ATH_CHECK( m_FPGASpacePointsKey.initialize() );
    ATH_CHECK( m_FPGAHitUnmappedKey.initialize() );
    ATH_CHECK( m_inputTruthParticleContainerKey.initialize(m_runOnRDO) );
    ATH_CHECK( m_truthLinkContainerKey.initialize() );
    ATH_CHECK( m_FPGATruthTrackKey.initialize() );
    ATH_CHECK( m_FPGAOfflineTrackKey.initialize() );

    ATH_MSG_DEBUG("initialize() Finished");

    
    return StatusCode::SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
//                          MAIN EXECUTE ROUTINE                             //
///////////////////////////////////////////////////////////////////////////////

StatusCode FPGATrackSimDataPrepAlg::execute()
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
      SmartIF<IEventProcessor> appMgr{service("ApplicationMgr")};
      if (!appMgr) {
          ATH_MSG_ERROR("Failed to retrieve ApplicationMgr as IEventProcessor");
          return StatusCode::FAILURE;
      }
      return appMgr->stopRun();
    }

    SG::WriteHandle<FPGATrackSimHitCollection> FPGAHits_1st (m_FPGAHitKey.at(0), ctx);
    ATH_CHECK( FPGAHits_1st.record (std::make_unique<FPGATrackSimHitCollection>()));
    SG::WriteHandle<FPGATrackSimHitCollection> FPGAHits_2nd (m_FPGAHitKey.at(1), ctx);
    ATH_CHECK( FPGAHits_2nd.record (std::make_unique<FPGATrackSimHitCollection>()));

    SG::WriteHandle<FPGATrackSimHitCollection> FPGAHitUnmapped (m_FPGAHitUnmappedKey, ctx);
    ATH_CHECK( FPGAHitUnmapped.record (std::make_unique<FPGATrackSimHitCollection>()));

    SG::WriteHandle<FPGATrackSimClusterCollection> FPGAClusters (m_FPGAClusterKey.at(0), ctx);
    ATH_CHECK( FPGAClusters.record (std::make_unique<FPGATrackSimClusterCollection>()));

    SG::WriteHandle<FPGATrackSimClusterCollection> FPGAClustersFiltered (m_FPGAClusterFilteredKey, ctx);
    ATH_CHECK( FPGAClustersFiltered.record (std::make_unique<FPGATrackSimClusterCollection>()));

    SG::WriteHandle<FPGATrackSimClusterCollection> FPGASpacePoints (m_FPGASpacePointsKey.at(0), ctx);
    ATH_CHECK( FPGASpacePoints.record (std::make_unique<FPGATrackSimClusterCollection>()));

    SG::WriteHandle<xAODTruthParticleLinkVector> truthLinkVec(m_truthLinkContainerKey);
    ATH_CHECK(truthLinkVec.record(std::make_unique<xAODTruthParticleLinkVector>()));

    SG::WriteHandle<FPGATrackSimTruthTrackCollection> FPGATruthTracks (m_FPGATruthTrackKey);
    ATH_CHECK(FPGATruthTracks.record(std::make_unique<FPGATrackSimTruthTrackCollection>()));

    SG::WriteHandle<FPGATrackSimOfflineTrackCollection> FPGAOfflineTracks (m_FPGAOfflineTrackKey);
    ATH_CHECK(FPGAOfflineTracks.record(std::make_unique<FPGATrackSimOfflineTrackCollection>()));

    // Apply truth track cuts    
    if ( m_doEvtSel ){
        if (!m_evtSel->selectEvent(&m_eventHeader))
        {
            ATH_MSG_DEBUG("Event skipped by: " << m_evtSel->name());
            return StatusCode::SUCCESS;
        }
        else {
            ATH_MSG_DEBUG("Event accepted by: " << m_evtSel->name());
            // Make a new truth link vector based on FPGATrackSim selections 
            if (m_runOnRDO) {
                SG::ReadHandle<xAOD::TruthParticleContainer> truthParticleContainer(m_inputTruthParticleContainerKey, ctx); // Read offline TruthParticles
                if (!truthParticleContainer.isValid()) {
                    ATH_MSG_ERROR("No valid truth particle container with key " << truthParticleContainer.key());
                    m_evtSel->setSelectedEvent(false);
                    return StatusCode::FAILURE;
                }
                const ElementLink<xAOD::TruthParticleContainer> eltp(*truthParticleContainer, truthParticleContainer->size() - 1);
                std::vector<FPGATrackSimTruthTrack> const& truthtracks = m_eventHeader.optional().getTruthTracks();
                for (const xAOD::TruthParticle* truthParticle : *truthParticleContainer) // loop over offline truth particles
                {
                    for (const FPGATrackSimTruthTrack& fpgaTruthTrack : truthtracks) // loop over FPGA truth tracks
                    {
                        if (fpgaTruthTrack.getBarcode() == static_cast<HepMcParticleLink::barcode_type>(truthParticle->barcode()))
                        {
                            truthLinkVec->push_back(new xAODTruthParticleLink(HepMcParticleLink(truthParticle->barcode(), 0,
                                HepMcParticleLink::IS_POSITION, HepMcParticleLink::IS_BARCODE), eltp));
                            ATH_MSG_VERBOSE("Truth link added");
                            break;
                        }
                    }
                }
                std::stable_sort(truthLinkVec->begin(), truthLinkVec->end(), SortTruthParticleLink());
                ATH_MSG_VERBOSE("Truth link size: " << truthLinkVec->size());
                if (truthLinkVec->size() == 0)
                {
                    ATH_MSG_DEBUG("No truth particles selected. Event skipped...");
                    m_evtSel->setSelectedEvent(false);
                    return StatusCode::SUCCESS;
                }
            }
        }
    } else {
        ATH_MSG_DEBUG("No Event Selection applied");
    }
    TIME(m_tread);

    // Event passes cuts, count it
    m_evt++;
    
    // Map, cluster, and filter hits
    ATH_CHECK(processInputs(FPGAHitUnmapped, FPGAClusters, FPGAClustersFiltered, FPGASpacePoints));
    
    // Now that this is done, push truth tracks back to storegate.
    for (const auto& truthtrack : m_logicEventHeader->optional().getTruthTracks()) {
        FPGATruthTracks->push_back(truthtrack);
    }

    // Need to do the same for offline tracks.
    for (const auto& offlineTrack : m_logicEventHeader->optional().getOfflineTracks()) {
        FPGAOfflineTracks->push_back(offlineTrack);
    }

    // Get reference to hits
    unsigned regionID = m_evtSel->getRegionID();
    // Recording Data
    auto mon_regionID = Monitored::Scalar<unsigned>("regionID", regionID);
    Monitored::Group(m_monTool, mon_regionID);

    TIME(m_tprocess);

    // If and when we set up code to run over more than one region/tower at a time this will need to be updated
    std::vector<FPGATrackSimHit> const & hits = m_logicEventHeader->towers().at(0).hits();

    std::vector<std::shared_ptr<const FPGATrackSimHit>> phits;
    phits.reserve(hits.size());
    for (FPGATrackSimHit const& h : hits) {
        if (h.isReal()) phits.emplace_back(std::make_shared<const FPGATrackSimHit>(h));
    }

    // Split the hits here by first stage vs second stage.
    const FPGATrackSimRegionMap* rmap_1st = m_FPGATrackSimMapping->SubRegionMap();
    for (const auto & hit : phits) {
        // If the hit falls within the boundaries of ANY subregion in the first stage, it's 1st stage.
        if (rmap_1st->getRegions(*hit).size() > 0) {
            FPGAHits_1st->push_back(*hit);
        } else {
            FPGAHits_2nd->push_back(*hit);
        }
    }
 
    auto mon_nhits = Monitored::Scalar<unsigned>("nHits", hits.size());
    auto mon_nhits_unmapped = Monitored::Scalar<unsigned>("nHits_unmapped", m_hits_miss.size());
    Monitored::Group(m_monTool, mon_nhits, mon_nhits_unmapped);

    // Write the output and reset
    ATH_CHECK(m_writeOutputTool->writeData());

    // Reset data pointers
    m_eventHeader.reset();
    m_logicEventHeader->reset();
    m_logicEventHeader_precluster->reset()

    TIME(m_tfin);
    
    return StatusCode::SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
//                  INPUT PASSING, READING AND PROCESSING                    //
///////////////////////////////////////////////////////////////////////////////

StatusCode FPGATrackSimDataPrepAlg::readInputs(bool & done)
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
StatusCode FPGATrackSimDataPrepAlg::processInputs(SG::WriteHandle<FPGATrackSimHitCollection> &FPGAHitUnmapped,
                                                            SG::WriteHandle<FPGATrackSimClusterCollection> &FPGAClusters,
                                                            SG::WriteHandle<FPGATrackSimClusterCollection> &FPGAClustersFiltered,
                                                            SG::WriteHandle<FPGATrackSimClusterCollection> &FPGASpacePoints)
{
    m_clusters.clear();
    m_spacepoints.clear();
    m_hits_miss.clear();

    // Map hits
    ATH_MSG_DEBUG("Running hits conversion");
    m_logicEventHeader->reset();
    m_logicEventHeader_precluster->reset();
    ATH_CHECK(m_hitMapTool->convert(1, m_eventHeader, *m_logicEventHeader));

    for (const FPGATrackSimHit& hit : m_hits_miss) FPGAHitUnmapped->push_back(hit);



    ATH_MSG_DEBUG("Hits conversion done, #unmapped hists = " << m_hits_miss.size());
    // Random removal of hits
    if (m_doHitFiltering) {
        ATH_MSG_DEBUG("Running hits filtering");
        ATH_CHECK(m_hitFilteringTool->DoRandomRemoval(*m_logicEventHeader, true));
    }

    // At this stage, copy the logicEventHeader.
    *m_logicEventHeader_precluster = *m_logicEventHeader;

    // Clustering
    for (int ic = 0; ic < m_clustering; ic++) {
      ATH_MSG_DEBUG("Running clustering");
      ATH_CHECK(m_clusteringTool->DoClustering(*m_logicEventHeader, m_clusters));
      m_clusters_original = m_clusters;
      
      // I think I also want to pass m_clusters to random removal (but won't work currently)
      if (m_doHitFiltering) ATH_CHECK(m_hitFilteringTool->DoRandomRemoval(*m_logicEventHeader, false));
      for (const FPGATrackSimCluster& cluster : m_clusters_original) FPGAClusters->push_back(cluster);      
    }
    
    // Filter hits/clusters (untested for hits, ie with m_clustering = false)
    if (m_doHitFiltering)
    {
        // get the sets of layers that we want to filter hits from
        std::vector<int> filter_pixel_physLayers;
        std::vector<int> filter_strip_physLayers;
        const FPGATrackSimPlaneMap *planeMap = m_FPGATrackSimMapping->PlaneMap_1st(0);
        //TODO This needs to change to deal with multimaps when multi logi layer maps start to use strips
        //Currntly the maps generated that can use diffrent logical layers for the same phys layers only use the pixles
        //This code will be updated in the future when maps are made with strips that can use the diffrent logical layers 
        ATH_CHECK(m_hitFilteringTool->GetPairedStripPhysLayers(planeMap, filter_strip_physLayers));
        m_clusters.clear();
        ATH_CHECK(m_hitFilteringTool->DoHitFiltering(*m_logicEventHeader, filter_pixel_physLayers, filter_strip_physLayers, m_clusters));
        for (const FPGATrackSimCluster &cluster : m_clusters) FPGAClustersFiltered->push_back(cluster);
 
    }

    // Space points
    if (m_doSpacepoints) {
        ATH_CHECK(m_spacepointsTool->DoSpacePoints(*m_logicEventHeader, m_spacepoints));
        for (const FPGATrackSimCluster& cluster : m_spacepoints) FPGASpacePoints->push_back(cluster);
    }

    return StatusCode::SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// Finalize

StatusCode FPGATrackSimDataPrepAlg::finalize()
{
#ifdef BENCHMARK_LOGICALHITSALG
    ATH_MSG_INFO("Timings:" <<
            "\nread input:   " << std::setw(10) << m_tread <<
            "\nprocess hits: " << std::setw(10) << m_tprocess <<
            "\nfin:          " << std::setw(10) << m_tfin
    );
#endif

    // TODO print out some data prep alg related stats here.s
    
    return StatusCode::SUCCESS;
}
