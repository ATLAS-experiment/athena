/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MaterialTrackReader.h"
#include "GaudiKernel/IInterface.h"

#include "GeoPrimitives/GeoPrimitivesToStringConverter.h"

#include "TTree.h"
#include "TFile.h"


ActsTrk::MaterialTrackReader::~MaterialTrackReader() = default;

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
    m_inputChain->SetBranchAddress("event_id", &m_currEvent);
    // get the number of events, which also loads the tree
    m_nTreeEntries = m_inputChain->GetEntries();
  
    if (m_skipEvents >0ul) {
        std::optional<std::uint32_t> evt{0ul};
        std::size_t procEvts{0ul};
        ATH_MSG_DEBUG("Skip "<<m_skipEvents<<" events. ");
        for (; m_currEntry < m_nTreeEntries; ++m_currEntry) {
            m_inputChain->GetEntry(m_currEntry);
            if (!evt) {
                evt = m_currEvent;
            } else if ((*evt) != m_currEvent) {
                ++procEvts;
                evt = m_currEvent;
            }
            if (procEvts == m_skipEvents) {
                break;
            }
        }
        ATH_MSG_DEBUG("Skipped "<<procEvts<<" events. Corresponding to "
                      <<m_currEntry<<" tree entries");
    }
    ATH_MSG_DEBUG("The full chain has " << m_nTreeEntries << " entries for " << m_maxEvents
                  << " events this corresponds to a batch size of: " << m_batchSize);
    return StatusCode::SUCCESS;
}

StatusCode ActsTrk::MaterialTrackReader::finalize() {
    if (m_inputChain) {
        ATH_MSG_ERROR("Not all entries / events have been processed. Processed entries: "
                <<(m_currEntry+ 1)<<"/"<<m_nTreeEntries<<", processed events: "
                <<m_procEvents<<"/"<<m_maxEvents.value());
    }
    return StatusCode::SUCCESS;
}

StatusCode
ActsTrk::MaterialTrackReader::execute () {
    const EventContext& ctx{Gaudi::Hive::currentContext()};
    // Write to the collection to the EventStore
    SG::WriteHandle materialTracks{m_materialTrackCollectionKey, ctx};

    // Record the collection once per event if not already there
    if (!materialTracks.isPresent()) {
        ATH_CHECK(materialTracks.record(std::make_unique<ActsTrk::RecordedMaterialTrackCollection>()));
    }

    if (m_currEntry >= m_nTreeEntries) {
        m_inputChain.reset();
        return StatusCode::SUCCESS;
    }

    if (m_inputChain == nullptr) {
        ATH_MSG_DEBUG("Invalid pointer to input chain");
        return StatusCode::FAILURE;
    }

    std::size_t nProcEvents{0ul};
    std::size_t nCurrentEvt{m_currEvent};

    for (; m_currEntry< m_nTreeEntries; ++m_currEntry) {       
        // get the correspoing entry and read it
        m_inputChain->GetEntry(m_currEntry);
        Acts::RecordedMaterialTrack rmTrack = m_accessor.read();

        ATH_MSG_DEBUG("Track vertex:  " << Amg::toString(rmTrack.first.first));
        ATH_MSG_DEBUG("Track momentum:" << Amg::toString(rmTrack.first.second));

        // filling the collection
        materialTracks->push_back(std::move(rmTrack));
        if (nCurrentEvt != m_currEvent) {
            ++nProcEvents;
            ++m_procEvents;
            nCurrentEvt = m_currEvent;
        }

        if (m_procEvents >= m_maxEvents) {
            m_currEntry = m_nTreeEntries;
            return StatusCode::SUCCESS;
        }
 
        if (nProcEvents >=m_batchSize) {
            break;
        }
 
    }
    return StatusCode::SUCCESS;
}

