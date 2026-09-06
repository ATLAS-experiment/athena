/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// Header file
#include "FlavorTagDiscriminants/TruthPVzRelativeToBeamspotAlg.h"

// Read and write handles
#include "StoreGate/ReadHandle.h"
#include "StoreGate/ReadDecorHandle.h"
#include "StoreGate/WriteDecorHandle.h"

#include <cmath>
#include <limits>


namespace FlavorTagDiscriminants {

  TruthPVzRelativeToBeamspotAlg::TruthPVzRelativeToBeamspotAlg(
    const std::string& name, ISvcLocator* loc)
    : AthReentrantAlgorithm(name, loc) {}


  StatusCode TruthPVzRelativeToBeamspotAlg::initialize() {
    ATH_MSG_DEBUG("Initializing " << name());

    ATH_CHECK(m_jetCollectionKey.initialize());
    ATH_CHECK(m_truthPVzKey.initialize());
    ATH_CHECK(m_outputKey.initialize());
    ATH_CHECK(m_eventInfoKey.initialize());

    return StatusCode::SUCCESS;
  }


  StatusCode TruthPVzRelativeToBeamspotAlg::execute(const EventContext& ctx) const {
    ATH_MSG_DEBUG("Executing " << name());

    // Read out jets
    SG::ReadHandle<xAOD::IParticleContainer> jets(m_jetCollectionKey, ctx);
    if(!jets.isValid()) {
      ATH_MSG_ERROR("Failed to retrieve jet container with key " << m_jetCollectionKey.key());
      return StatusCode::FAILURE;
    }

    // Read out the beamspot position
    SG::ReadHandle<xAOD::EventInfo> eventInfo(m_eventInfoKey, ctx);
    if(!eventInfo.isValid()) {
      ATH_MSG_ERROR("Failed to retrieve EventInfo with key " << m_eventInfoKey.key());
      return StatusCode::FAILURE;
    }
    const double beamPosZ = eventInfo->beamPosZ();

    SG::ReadDecorHandle<xAOD::IParticleContainer, float> truthPVz(m_truthPVzKey, ctx);
    SG::WriteDecorHandle<xAOD::IParticleContainer, float> relToBeamspot(m_outputKey, ctx);

    for(const xAOD::IParticle* jet : *jets) {
      const double zed = truthPVz(*jet);
      relToBeamspot(*jet) = std::isfinite(zed)
        ? static_cast<float>(zed - beamPosZ)
        : std::numeric_limits<float>::quiet_NaN();
    }

    return StatusCode::SUCCESS;
  }
}
