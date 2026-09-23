/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "DerivationFrameworkInDet/InDetTrackSelectionToolWrapper.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteDecorHandle.h"
#include <vector>
#include <string>
#include <string_view>

namespace DerivationFramework {

  StatusCode InDetTrackSelectionToolWrapper::initialize()
  {
    if (m_decorationKey.empty()) {
      ATH_MSG_ERROR("No decoration prefix name provided for the output of InDetTrackSelectionToolWrapper!");
      return StatusCode::FAILURE;
    }

    ATH_CHECK( m_tracksKey.initialize() );
    ATH_CHECK(m_decorationKey.initialize());
    ATH_MSG_INFO("Using " << m_tracksKey << "as the source collection for inner detector track particles");
    ATH_CHECK(m_tool.retrieve());
    ATH_MSG_INFO(" InDetTrackSelectionToolWrapper::initialize i: " << inputHandles().size() << " o:" << outputHandles().size() );
    return StatusCode::SUCCESS;
  }


  StatusCode InDetTrackSelectionToolWrapper::execute(const EventContext& ctx) const
  {

    // retrieve track container
    SG::ReadHandle<xAOD::TrackParticleContainer> tracks(m_tracksKey, ctx);
    if (!tracks.isValid()) {
      //if( ! tracks ) {
      ATH_MSG_ERROR ("Couldn't retrieve TrackParticles with key: " << tracks.key() );
      return StatusCode::FAILURE;
    }
    // Run tool for each element and decorate with the decision
    SG::WriteDecorHandle<xAOD::TrackParticleContainer,bool > accept(m_decorationKey, ctx);
    for (const auto *trItr : *tracks) {
      accept( *trItr ) = m_tool->accept(trItr).getCutResult(0);
    } // end of loop over tracks

    return StatusCode::SUCCESS;
  }

}
