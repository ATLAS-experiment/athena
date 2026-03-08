// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#include "FPGATrackSimAlgorithms/FPGATrackSimRegionMergingAlg.h"

constexpr bool enableBenchmark = 
#ifdef BENCHMARK_FPGATRACKSIM
    true;
#else
    false;
#endif

FPGATrackSim::FPGATrackSimRegionMergingAlg::FPGATrackSimRegionMergingAlg(const std::string& name, ISvcLocator* pSvcLocator) : AthReentrantAlgorithm(name, pSvcLocator) {
}

StatusCode FPGATrackSim::FPGATrackSimRegionMergingAlg::initialize()
{
    ATH_CHECK(m_FPGATrackCollectionKeys.initialize());
    ATH_CHECK(m_FPGARoadCollectionKeys.initialize());

    ATH_CHECK(m_FinalFPGARoadkCollectionKey.initialize());
    ATH_CHECK(m_FinalFPGATrackCollectionKey.initialize());

    ATH_CHECK(m_overlapRemovalTool.retrieve());

    ATH_CHECK(m_chrono.retrieve());
    return StatusCode::SUCCESS;
}

StatusCode FPGATrackSim::FPGATrackSimRegionMergingAlg::execute(const EventContext& ctx) const
{
    
    std::vector<SG::ReadHandle<FPGATrackSimTrackCollection>> incomingFPGATrackSimTrackCollections = m_FPGATrackCollectionKeys.makeHandles(ctx);
    SG::WriteHandle<FPGATrackSimTrackCollection> finalFPGATracksHandle (m_FinalFPGATrackCollectionKey, ctx);

    std::vector<SG::ReadHandle<FPGATrackSimRoadCollection>> incomingFPGARoadSimTrackCollections = m_FPGARoadCollectionKeys.makeHandles(ctx);
    SG::WriteHandle<FPGATrackSimRoadCollection> finalFPGARoads (m_FinalFPGARoadkCollectionKey, ctx);
    
    std::vector<const FPGATrackSimTrackCollection*> incomingFPGATrackSimTrackCollectionsPtrs;
    for (SG::ReadHandle<FPGATrackSimTrackCollection>& trackCollection : incomingFPGATrackSimTrackCollections)
    {
        if (!trackCollection.isValid())
        {
            ATH_MSG_ERROR("Invalid FPGATrackCollection key with name " << trackCollection.key());
            return StatusCode::FAILURE;
        }
        incomingFPGATrackSimTrackCollectionsPtrs.push_back(trackCollection.cptr());
    }

    std::unique_ptr<FPGATrackSimTrackCollection> finalFPGATracksPtr = std::make_unique<FPGATrackSimTrackCollection>(); // temporary collection
    ATH_CHECK(mergeTracks(incomingFPGATrackSimTrackCollectionsPtrs, finalFPGATracksPtr));
    finalFPGATracksHandle = std::move(finalFPGATracksPtr);
    
    if(m_useRoads){
        std::vector<const FPGATrackSimRoadCollection*> incomingFPGARoadSimTrackCollectionsPtrs;
        for (SG::ReadHandle<FPGATrackSimRoadCollection>& roadCollection : incomingFPGARoadSimTrackCollections)
        {
            if (!roadCollection.isValid())
            {
                ATH_MSG_ERROR("Invalid FPGARoadCollection key with name " << roadCollection.key());
                return StatusCode::FAILURE;
            }
            incomingFPGARoadSimTrackCollectionsPtrs.push_back(roadCollection.cptr());
        }

        std::unique_ptr<FPGATrackSimRoadCollection> finalFPGARoadsPtr = std::make_unique<FPGATrackSimRoadCollection>(); // temporary collection
        ATH_CHECK(mergeRoads(incomingFPGARoadSimTrackCollectionsPtrs, finalFPGARoadsPtr));
                             
        finalFPGARoads = std::move(finalFPGARoadsPtr);
    }
        


    return StatusCode::SUCCESS;
}

StatusCode FPGATrackSim::FPGATrackSimRegionMergingAlg::finalize()
{
    ATH_MSG_INFO("Number of tracks: incoming / passed region OR / passed globalOR: " << m_allIncomingTracks << " / " << m_nPreORTracks << " / " << m_nPostORTracks);
    return StatusCode::SUCCESS;
}

StatusCode FPGATrackSim::FPGATrackSimRegionMergingAlg::mergeTracks(const std::vector<const FPGATrackSimTrackCollection*>& inputTracksPtrs,
                                                                   std::unique_ptr<FPGATrackSimTrackCollection>& outputTracks) const {
    size_t numberOfAllTracks = 0;
    for (const FPGATrackSimTrackCollection* tracksInRegion : inputTracksPtrs)
    {
        numberOfAllTracks += tracksInRegion->size();
    }

    ATH_MSG_INFO("Filtering and concatenating " << numberOfAllTracks << " tracks from " << inputTracksPtrs.size() << " regions");
    if constexpr (enableBenchmark) m_chrono->chronoStart("RegionMerging: Merging Tracks");
    FPGATrackSimTrackCollection regionFilteredTracks;
    regionFilteredTracks.reserve(numberOfAllTracks);
    for (const FPGATrackSimTrackCollection* tracksInRegion : inputTracksPtrs)
    {
        for (const FPGATrackSimTrack& track : *tracksInRegion)
        {
            if (track.passedOR())
            {
                regionFilteredTracks.push_back(track);
            }
        }
    }
    if constexpr (enableBenchmark) m_chrono->chronoStop("RegionMerging: Merging Tracks");

    // Overlap removal: It runs only on the tracks that passed the in-region OR
    if constexpr (enableBenchmark) m_chrono->chronoStart("RegionMerging: Overlap removal (tracks)");
    if (m_doOverlapRemoval) ATH_CHECK(m_overlapRemovalTool->runOverlapRemoval(regionFilteredTracks));
    if constexpr (enableBenchmark) m_chrono->chronoStop("RegionMerging: Overlap removal (tracks)");

    if constexpr (enableBenchmark) m_chrono->chronoStart("RegionMerging: Copying final tracks");
    outputTracks->reserve(numberOfAllTracks);
    for (const FPGATrackSimTrack & track: regionFilteredTracks)
    {
        if(track.passedOR()) outputTracks->push_back(track);
    }
    if constexpr (enableBenchmark) m_chrono->chronoStop("RegionMerging: Copying final tracks");

    m_nPreORTracks.fetch_add(regionFilteredTracks.size(), std::memory_order_relaxed); // number of tracks that passed the region OR only (if configured upstream)
    m_allIncomingTracks.fetch_add(numberOfAllTracks, std::memory_order_relaxed); // number of all tracks from all regions
    m_nPostORTracks.fetch_add(outputTracks->size(), std::memory_order_relaxed); // number of tracks that passed the global OR

    return StatusCode::SUCCESS;
}


StatusCode FPGATrackSim::FPGATrackSimRegionMergingAlg::mergeRoads(const std::vector<const FPGATrackSimRoadCollection*>& inputRoads,
    std::unique_ptr<FPGATrackSimRoadCollection>& outputRoads) const{
        size_t numberOfAllRoads = 0;
        for (const FPGATrackSimRoadCollection* roadCollection : inputRoads) {
            numberOfAllRoads += roadCollection->size();
        }

        ATH_MSG_INFO("Filtering and concatenating " << numberOfAllRoads << " roads from " << inputRoads.size() << " regions");
        if constexpr (enableBenchmark) m_chrono->chronoStart("Merging Roads");
        outputRoads->reserve(numberOfAllRoads);

        for (const FPGATrackSimRoadCollection* roadCollection : inputRoads) {
            outputRoads->insert(outputRoads->end(), roadCollection->begin(), roadCollection->end());
        }
        if constexpr (enableBenchmark) m_chrono->chronoStop("Merging Roads");

        return StatusCode::SUCCESS;
    }