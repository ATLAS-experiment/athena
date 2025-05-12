// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

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


constexpr bool enableBenchmark = 
#ifdef BENCHMARK_FPGATRACKSIM
    true;
#else
    false;
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
    ATH_CHECK(m_hitMapTools.retrieve());
    ATH_CHECK(m_hitFilteringTool.retrieve(EnableTool{m_doHitFiltering}));
    ATH_CHECK(m_clusteringTool.retrieve(EnableTool{m_clustering > 0}));
    
    ATH_CHECK(m_writeOutputTool.retrieve());
    ATH_CHECK(m_eventSelectionTools.retrieve());


    ATH_MSG_DEBUG("initialize() Instantiating root objects");
    m_logicEventHeader_precluster = m_writeOutputTool->addInputBranch(m_preClusterBranch.value(), true);
    m_logicEventHeader_cluster = m_writeOutputTool->addInputBranch(m_clusterBranch.value(), true);
    m_logicEventHeader = m_writeOutputTool->addInputBranch(m_postClusterBranch.value(), true);
    
    ATH_MSG_DEBUG("initialize() Setting branch");

    if (!m_monTool.empty())
        ATH_CHECK(m_monTool.retrieve());

    ATH_CHECK( m_FPGAClusterKey.initialize() );
    ATH_CHECK( m_FPGAHitKey.initialize() );
    ATH_CHECK( m_FPGAHitUnmappedKey.initialize() );
    ATH_CHECK( m_inputTruthParticleContainerKey.initialize(m_useInternalTruthTracks) );
    ATH_CHECK( m_truthLinkContainerKey.initialize() );
    ATH_CHECK( m_FPGATruthTrackKey.initialize() );
    ATH_CHECK( m_FPGAOfflineTrackKey.initialize() );

    ATH_CHECK( m_chrono.retrieve() );
    ATH_MSG_DEBUG("initialize() Finished");

    
    return StatusCode::SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
//                          MAIN EXECUTE ROUTINE                             //
///////////////////////////////////////////////////////////////////////////////

StatusCode FPGATrackSimDataPrepAlg::execute()
{
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

    SG::WriteHandle<FPGATrackSimHitCollection> FPGAHits (m_FPGAHitKey, ctx);
    ATH_CHECK( FPGAHits.record (std::make_unique<FPGATrackSimHitCollection>()));

    SG::WriteHandle<FPGATrackSimHitCollection> FPGAHitUnmapped (m_FPGAHitUnmappedKey, ctx);
    ATH_CHECK( FPGAHitUnmapped.record (std::make_unique<FPGATrackSimHitCollection>()));

    SG::WriteHandle<FPGATrackSimClusterCollection> FPGAClusters (m_FPGAClusterKey.at(0), ctx);
    ATH_CHECK( FPGAClusters.record (std::make_unique<FPGATrackSimClusterCollection>()));

    SG::WriteHandle<xAODTruthParticleLinkVector> truthLinkVec(m_truthLinkContainerKey);
    ATH_CHECK(truthLinkVec.record(std::make_unique<xAODTruthParticleLinkVector>()));

    SG::WriteHandle<FPGATrackSimTruthTrackCollection> FPGATruthTracks (m_FPGATruthTrackKey);
    ATH_CHECK(FPGATruthTracks.record(std::make_unique<FPGATrackSimTruthTrackCollection>()));

    SG::WriteHandle<FPGATrackSimOfflineTrackCollection> FPGAOfflineTracks (m_FPGAOfflineTrackKey);
    ATH_CHECK(FPGAOfflineTracks.record(std::make_unique<FPGATrackSimOfflineTrackCollection>()));       
    
    // Apply event selection based on truth tracks    
    if (m_doEvtSel) {
        bool acceptEvent = false;
        if constexpr (enableBenchmark) m_chrono->chronoStart("DataPrep: EventSelection");
        for (auto eventSelector : m_eventSelectionTools)
        {
            if (eventSelector->selectEvent(m_eventHeader)) {
                ATH_MSG_DEBUG("Event accepted by: " << eventSelector->name());
                acceptEvent = true;
            }
        }
        if constexpr (enableBenchmark) m_chrono->chronoStop("DataPrep: EventSelection");
        if (m_useInternalTruthTracks && acceptEvent) {
            if constexpr (enableBenchmark) m_chrono->chronoStart("DataPrep: TruthMatching");
            SG::ReadHandle<xAOD::TruthParticleContainer> truthParticleContainer(m_inputTruthParticleContainerKey, ctx); // Read offline TruthParticles
            if (!truthParticleContainer.isValid()) {
                ATH_MSG_ERROR("No valid truth particle container with key " << truthParticleContainer.key());
                return StatusCode::FAILURE;
            }
            ATH_MSG_DEBUG("making mp of truth particles");
            std::unordered_map<HepMcParticleLink::barcode_type, std::pair<const xAOD::TruthParticle*, size_t>> truthParticlesMap;
            size_t truthParticleIndex = 0;
            for (const xAOD::TruthParticle* truthParticle : *truthParticleContainer) {
                truthParticlesMap.insert(std::make_pair(HepMC::barcode(truthParticle), std::make_pair(truthParticle,truthParticleIndex)));
                truthParticleIndex++;
            }
            
            const FPGATrackSimTruthTrackCollection& fpgaTruthTracks = m_eventHeader.optional().getTruthTracks();
            truthLinkVec->reserve(fpgaTruthTracks.size());
            ATH_MSG_DEBUG("begin truth matching for " << fpgaTruthTracks.size() << " FPGA truth tracks");
            for (const FPGATrackSimTruthTrack& fpgaTruthTrack : fpgaTruthTracks) {
                auto it = truthParticlesMap.find(fpgaTruthTrack.getBarcode());
                if (it != truthParticlesMap.end()) {
                    ElementLink<xAOD::TruthParticleContainer> truthParticleLink(*truthParticleContainer, it->second.second);
                    // TODO: check if we can avoid using the previously-created map and look directly for the barcode in the link vector container
                    truthLinkVec->push_back(new xAODTruthParticleLink(HepMcParticleLink(HepMC::barcode(it->second.first), 0,
                        HepMcParticleLink::IS_POSITION, HepMcParticleLink::IS_BARCODE), truthParticleLink));
                    ATH_MSG_DEBUG("Truth link added");
                }
            }
            if (truthLinkVec->empty()) {
                ATH_MSG_DEBUG("No truth particles selected. Skipping event...");
                return StatusCode::SUCCESS;
            }
            std::stable_sort(truthLinkVec->begin(), truthLinkVec->end(), SortTruthParticleLink());
            if constexpr (enableBenchmark) m_chrono->chronoStop("DataPrep: TruthMatching");
        }
    }
    else {
        ATH_MSG_DEBUG("No Event Selection applied");
        for (auto eventSelector : m_eventSelectionTools) {
            eventSelector->setSelectedEvent(true);
        }
    }
    
    // Event passes cuts, count it
    m_evt++;
    
    // Map, cluster, and filter hits
    ATH_CHECK(processInputs(FPGAHitUnmapped, FPGAClusters));

    if constexpr (enableBenchmark) m_chrono->chronoStart("DataPrep: get truth/offline tracks");
    // Now that this is done, push truth tracks back to storegate.
    for (const auto& truthtrack : m_logicEventHeader->optional().getTruthTracks()) {
        FPGATruthTracks->push_back(truthtrack);
    }
    
    // Need to do the same for offline tracks.
    for (const auto& offlineTrack : m_logicEventHeader->optional().getOfflineTracks()) {
        FPGAOfflineTracks->push_back(offlineTrack);
    }
    
    if constexpr (enableBenchmark) m_chrono->chronoStop("DataPrep: get truth/offline tracks");

    // Get reference to hits

    // Recording Data
    for (auto eventSelector : m_eventSelectionTools)
    {
        // Get reference to hits
        const unsigned& regionID = eventSelector->getRegionID();

        auto mon_regionID = Monitored::Scalar<unsigned>("regionID", regionID);
        Monitored::Group(m_monTool, mon_regionID);
    }

    std::vector<FPGATrackSimHit> const& hits = m_logicEventHeader->towers().at(0).hits();
    if (m_recordHits) {
        if constexpr (enableBenchmark) m_chrono->chronoStart("DataPrep: record hits");
        // If and when we set up code to run over more than one region/tower at a time this will need to be updated
        FPGAHits->reserve(hits.size());
        for (const auto& hit : hits) {
            if (hit.isReal()) FPGAHits->push_back(hit);
        }
        if constexpr (enableBenchmark) m_chrono->chronoStop("DataPrep: record hits");
    }

    auto mon_nhits = Monitored::Scalar<unsigned>("nHits", hits.size());
    auto mon_nhits_unmapped = Monitored::Scalar<unsigned>("nHits_unmapped", m_hits_miss.size());
    Monitored::Group(m_monTool, mon_nhits, mon_nhits_unmapped);
    
    // Write the output and reset
    if (m_writeOutputData)
        ATH_CHECK(m_writeOutputTool->writeData());

    // Reset data pointers
    m_eventHeader.reset();
    m_logicEventHeader->reset();
    m_logicEventHeader_precluster->reset();
    m_logicEventHeader_cluster->reset();
    
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
            ATH_MSG_DEBUG("Cannot read more events from file, returning");
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
                                                            SG::WriteHandle<FPGATrackSimClusterCollection> &FPGAClusters)
{
    m_clusters->clear();
    m_hits_miss.clear();

    // Map hits
    ATH_MSG_DEBUG("Running hits conversion");
    m_logicEventHeader->reset();
    m_logicEventHeader_precluster->reset();
    m_logicEventHeader_cluster->reset();
    if constexpr (enableBenchmark) m_chrono->chronoStart("DataPrep: RawToLogical");
    for (auto hitMapTool : m_hitMapTools){
        ATH_CHECK(hitMapTool->convert(1, m_eventHeader, *m_logicEventHeader));
    }
    if constexpr (enableBenchmark) m_chrono->chronoStop("DataPrep: RawToLogical");

    
    for (const FPGATrackSimHit& hit : m_hits_miss) FPGAHitUnmapped->push_back(hit);


    ATH_MSG_DEBUG("Hits conversion done, #unmapped hists = " << m_hits_miss.size());

    // At this stage, copy the logicEventHeader.
    if(m_writeOutputData) *m_logicEventHeader_precluster = *m_logicEventHeader;

    if constexpr (enableBenchmark) m_chrono->chronoStart("DataPrep: Clustering");
    // Clustering
    for (int ic = 0; ic < m_clustering; ic++) {
        ATH_MSG_DEBUG("Running clustering");
        ATH_CHECK(m_clusteringTool->DoClustering(*m_logicEventHeader, *m_clusters));
        // I think I also want to pass m_clusters to random removal (but won't work currently)
        if (m_doHitFiltering) ATH_CHECK(m_hitFilteringTool->DoRandomRemoval(*m_logicEventHeader, false));
        unsigned npix(0), nstrip(0);
        for (const FPGATrackSimCluster& cluster : *m_clusters) {
            if (cluster.getClusterEquiv().isPixel()) npix++;
            else nstrip++;
        }
        m_nPixClusters += npix;
        m_nStripClusters += nstrip;
        if (npix > m_nMaxPixClusters) m_nMaxPixClusters = npix;
        if (nstrip > m_nMaxStripClusters) m_nMaxStripClusters = nstrip;
        if (m_clusters->size() > m_nMaxClusters) m_nMaxClusters = m_clusters->size();
    }
    FPGAClusters->insert(
        FPGAClusters->end(),
        std::make_move_iterator(m_clusters->begin()),
        std::make_move_iterator(m_clusters->end()));
    
    if constexpr (enableBenchmark) m_chrono->chronoStop("DataPrep: Clustering");

    // At this stage, copy the logicEventHeader.
    // TODO: no longer needed because we don't do SPs here.
    if(m_writeOutputData) *m_logicEventHeader_cluster = *m_logicEventHeader;


    return StatusCode::SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// Finalize

StatusCode FPGATrackSimDataPrepAlg::finalize()
{
    ATH_MSG_INFO("PRINTING FPGATRACKSIM SIMPLE DATAPREP STATS");
    ATH_MSG_INFO("========================================================================================");
    ATH_MSG_INFO("Number of pixel clusters/event = " << m_nPixClusters/m_evt);
    ATH_MSG_INFO("Number of strip clusters/event = " << m_nStripClusters/m_evt);    
    ATH_MSG_INFO("Max number of pixel clusters in an event = " << m_nMaxPixClusters);
    ATH_MSG_INFO("Max number of strip clusters in an event = " << m_nMaxStripClusters);
    ATH_MSG_INFO("Max number of clusters in an event = " << m_nMaxClusters);
        
    return StatusCode::SUCCESS;
}
