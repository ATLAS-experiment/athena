/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MaterialTrackReader.h"
#include "GaudiKernel/IInterface.h"

#include "TTree.h"
#include "TFile.h"


ActsTrk::MaterialTrackReader::MaterialTrackReader(const std::string& name, ISvcLocator* pSvcLocator) :
    AthReentrantAlgorithm(name, pSvcLocator),
    m_accessor({false, m_readCachedSurfaceInformation})
{}

ActsTrk::MaterialTrackReader::~MaterialTrackReader()
{


}

StatusCode ActsTrk::MaterialTrackReader::initialize()
{
    ATH_CHECK(m_materialTrackCollectionKey.initialize());

    if (m_fileNames.empty()) {
        ATH_MSG_ERROR("No input files given... Please check!!");
        return StatusCode::FAILURE;
    }

    // set up the input chain
    m_inputChain = std::make_unique<TChain>(m_treeName.value().c_str());

    // loop over the input files
    for (const auto& inputFile : m_fileNames) {
        // add file to the input chain
        m_inputChain->Add(inputFile.c_str());
        ATH_MSG_DEBUG("Adding File " << inputFile << " to tree '" << m_treeName << "'.");
    }

    // Connect the branches
    m_accessor.connectForRead(*m_inputChain);

    // get the number of events, which also loads the tree
    std::size_t nentries = m_inputChain->GetEntries();
    m_events = static_cast<std::size_t>(m_inputChain->GetMaximum("event_id") + 1);
    m_batchSize = nentries / m_events;

    ATH_MSG_DEBUG("The full chain has " << nentries << " entries for " << m_events
                  << " events this corresponds to a batch size of: " << m_batchSize);

    return StatusCode::SUCCESS;
}

StatusCode ActsTrk::MaterialTrackReader::finalize()
{
   return StatusCode::SUCCESS;
}

StatusCode
ActsTrk::MaterialTrackReader::execute (const EventContext& ctx) const
{
    if (m_inputChain == nullptr) {
        ATH_MSG_DEBUG("Invalid pointer to input chain");
        return StatusCode::SUCCESS;
    }

    if (ctx.evt() >= m_events) {
        ATH_MSG_INFO("Maximum number of events in file reached, nothing to be done...");
        return StatusCode::SUCCESS;
    }

    // Write to the collection to the EventStore
    SG::WriteHandle<ActsTrk::RecordedMaterialTrackCollection> materialTracks(m_materialTrackCollectionKey, ctx);

    // Record the collection once per event if not already there
    if (!materialTracks.isPresent()) {
        auto coll = std::make_unique<ActsTrk::RecordedMaterialTrackCollection>();
        ATH_CHECK(materialTracks.record(std::move(coll)));
    }

    // Add the track to the recorded collection
    auto* coll = materialTracks.ptr();
    if (!coll) {
        ATH_MSG_ERROR("RecordedMaterialTrackCollection ptr() is null for key "
                      << m_materialTrackCollectionKey.key());
        return StatusCode::FAILURE;
    }

    // lock the mutex
    std::lock_guard<std::mutex> lock(m_readMutex);
    for (std::size_t index = 0; index < m_batchSize; index++) {
        // evaluate the entry number based on the batch size
        auto entry = m_batchSize * ctx.evt() + index;

        ATH_MSG_DEBUG("Reading event / entry (in batch) : " << ctx.evt() << " / " << entry << "(" << index << ")");

        // get the correspoing entry and read it
        m_inputChain->GetEntry(entry);
        Acts::RecordedMaterialTrack rmTrack = m_accessor.read();

        ATH_MSG_DEBUG("Track vertex:  " << rmTrack.first.first);
        ATH_MSG_DEBUG("Track momentum:" << rmTrack.first.second);

        // filling the collection
        coll->push_back(std::move(rmTrack));
    }

    return StatusCode::SUCCESS;

}

