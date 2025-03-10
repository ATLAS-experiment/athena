/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "PixelToTPIDDualAlg.h"

#include "AthContainers/ConstDataVector.h"
#include "TrkTrack/Track.h"
#include "xAODTracking/TrackParticleContainer.h"
namespace CP {

  PixelToTPIDDualAlg::PixelToTPIDDualAlg(const std::string& name, ISvcLocator* svcloc) :
    AthAlgorithm(name, svcloc), m_tool("CP::PixelToTPIDDualTool") /*public tool*/ {
    declareProperty("Input", m_inputTracks = "");
    declareProperty("Tool", m_tool);
  }

  StatusCode PixelToTPIDDualAlg::initialize() {
    if (m_inputTracks.empty()) {
      ATH_MSG_ERROR("You must specify an INPUT track collection");
      return StatusCode::FAILURE;
    }

    CHECK(m_tool.retrieve());

    return StatusCode::SUCCESS;
  }

  StatusCode PixelToTPIDDualAlg::execute() {
    // fetch input collection
    const xAOD::TrackParticleContainer* tracks = nullptr;
    CHECK(evtStore()->retrieve(tracks, m_inputTracks));

    for (const xAOD::TrackParticle* iTrack : *tracks) {
      int nUsedHits = 0;
      int nUsedIBLOverflowHits = 0;
      m_tool->dEdx(*iTrack, nUsedHits, nUsedIBLOverflowHits);
    }

    return StatusCode::SUCCESS;
  }

}  // namespace CP
