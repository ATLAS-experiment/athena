/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MaterialTrackReader.h"

#include <optional>

#include "GaudiKernel/IInterface.h"
#include "GeoPrimitives/GeoPrimitivesToStringConverter.h"
#include "TFile.h"
#include "TTree.h"

ActsTrk::MaterialTrackReader::~MaterialTrackReader() = default;

StatusCode ActsTrk::MaterialTrackReader::initialize() {

  std::cout
      << "DEBUGFIX marker: MaterialTrackReader running the event_id-grouping "
         "patch"
      << std::endl;
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
    ATH_MSG_DEBUG("Adding File " << inputFile << " to tree '" << m_treeName
                                 << "'.");
    if (!m_inputChain->Add(inputFile.c_str())) {
      ATH_MSG_ERROR("Failed to load file " << inputFile);
      return StatusCode::FAILURE;
    }
  }

  // Connect the branches
  m_accessor.connectForRead(*m_inputChain);
  // get the number of events, which also loads the tree
  m_nTreeEntries = m_inputChain->GetEntries();

  if (m_skipEvents > 0ul) {
    std::optional<std::uint32_t> evt{0ul};
    std::size_t procEvts{0ul};
    ATH_MSG_DEBUG("Skip " << m_skipEvents << " events. ");
    for (; m_currEntry < m_nTreeEntries; ++m_currEntry) {
      m_inputChain->GetEntry(m_currEntry);
      if (!evt) {
        evt = m_accessor.eventId();
      } else if ((*evt) != m_accessor.eventId()) {
        ++procEvts;
        evt = m_accessor.eventId();
      }
      if (procEvts == m_skipEvents) {
        break;
      }
    }
    ATH_MSG_INFO("Skipped " << procEvts << " events. Corresponding to "
                            << m_currEntry << " tree entries");
  }
  if (!m_nTreeEntries) {
    ATH_MSG_ERROR("Input does not contain any recorded track");
    return StatusCode::FAILURE;
  }
  ATH_MSG_INFO("Material files contain "
               << m_nTreeEntries << " entries. Process " << m_batchSize.value()
               << " material events per athena event. Until "
               << m_maxEvents.value()
               << " events are processed or the tree is finished");
  return StatusCode::SUCCESS;
}

StatusCode ActsTrk::MaterialTrackReader::finalize() {
  if (m_inputChain) {
    ATH_MSG_ERROR(
        "Not all entries / events have been processed. Processed entries: "
        << (m_currEntry + 1) << "/" << m_nTreeEntries << ", processed events: "
        << m_procEvents << "/" << m_maxEvents.value());
  }
  return StatusCode::SUCCESS;
}

StatusCode ActsTrk::MaterialTrackReader::execute(const EventContext& ctx) {
  // Write to the collection to the EventStore
  SG::WriteHandle materialTracks{m_materialTrackCollectionKey, ctx};

  // Record the collection once per event if not already there
  if (!materialTracks.isPresent()) {
    ATH_CHECK(materialTracks.record(
        std::make_unique<ActsTrk::RecordedMaterialTrackCollection>()));
  }

  if (m_currEntry >= m_nTreeEntries) {
    m_inputChain.reset();
    return StatusCode::SUCCESS;
  }

  std::size_t nProcEvents{0ul};
  std::size_t nCurrentEvt{m_accessor.eventId()};

  // Each tree entry is a single material step, not a full track: many
  // consecutive entries share the same event_id and together make up one
  // physical track. Accumulate them into pendingTrack and only push the
  // merged track once the event_id changes (or we run out of entries),
  // otherwise the material mapper's per-track averaging normalizes by the
  // number of steps instead of the number of tracks.
  std::optional<Acts::RecordedMaterialTrack> pendingTrack{};
  auto flushPending = [&]() {
    if (pendingTrack) {
      materialTracks->push_back(std::move(*pendingTrack));
      pendingTrack.reset();
    }
  };

  for (; m_currEntry < m_nTreeEntries; ++m_currEntry) {
    ATH_MSG_VERBOSE("Fetched entry " << m_currEntry
                                     << ", eventId: " << m_accessor.eventId());
    // get the correspoing entry and read it
    m_inputChain->GetEntry(m_currEntry);
    Acts::RecordedMaterialTrack rmTrack = m_accessor.read();
    m_accessor.eventId();

    ATH_MSG_VERBOSE("Track vertex:  " << Amg::toString(rmTrack.first.first)
                                      << ", momentum:"
                                      << Amg::toString(rmTrack.first.second));

    if (nCurrentEvt != m_accessor.eventId()) {
      // previous event_id's steps are complete: flush the assembled track
      flushPending();
      ++nProcEvents;
      ++m_procEvents;
      nCurrentEvt = m_accessor.eventId();
    }

    if (m_procEvents >= m_maxEvents) {
      flushPending();
      ATH_MSG_INFO("All " << m_maxEvents << " events have been processed");
      m_currEntry = m_nTreeEntries;
      return StatusCode::SUCCESS;
    }

    if (nProcEvents >= m_batchSize) {
      ATH_MSG_DEBUG("Batch processing " << nProcEvents << " completed. ");
      break;
    }

    // merge this step into the track being assembled for the current event_id
    if (!pendingTrack) {
      pendingTrack = std::move(rmTrack);
    } else {
      auto& interactions = pendingTrack->second.materialInteractions;
      interactions.insert(
          interactions.end(),
          std::make_move_iterator(rmTrack.second.materialInteractions.begin()),
          std::make_move_iterator(rmTrack.second.materialInteractions.end()));
      pendingTrack->second.materialInX0 += rmTrack.second.materialInX0;
      pendingTrack->second.materialInL0 += rmTrack.second.materialInL0;
      if (interactions.size() % 50 == 0) {
        ATH_MSG_INFO("DEBUGFIX merge: pending track now has "
                     << interactions.size() << " interactions (event_id "
                     << nCurrentEvt << ")");
      }
    }
  }
  // end of tree reached while a track was still being assembled
  flushPending();
  return StatusCode::SUCCESS;
}
