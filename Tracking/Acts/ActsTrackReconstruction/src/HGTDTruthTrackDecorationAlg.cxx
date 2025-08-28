/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration.
*/

#include "src/HGTDTruthTrackDecorationAlg.h"
#include "StoreGate/WriteDecorHandle.h"

namespace ActsTrk{

  StatusCode HGTDTruthTrackDecorationAlg::initialize()
  {
    ATH_MSG_DEBUG("Initializing " << name() << "...");

    ATH_CHECK( m_trackParticleContainerName.initialize() );

    ATH_CHECK( m_layerClusterTruthClassKey.initialize() );
    ATH_CHECK( m_layerClusterShadowedKey.initialize() );
    ATH_CHECK( m_layerClusterMergedKey.initialize() );
    ATH_CHECK( m_layerPrimaryExpectedKey.initialize() );

    return StatusCode::SUCCESS;
  }

  StatusCode HGTDTruthTrackDecorationAlg::execute(const EventContext& ctx) const
  {
    ATH_MSG_DEBUG("Executing " << name() << "...");

    const xAOD::TrackParticleContainer* trackParticles {nullptr};
    ATH_CHECK(SG::get(trackParticles, m_trackParticleContainerName, ctx));

    ATH_MSG_DEBUG("Size of trackParticles collection " << trackParticles->size());

    // Create WriteDecorHandles for all decorations
    SG::WriteDecorHandle<xAOD::TrackParticleContainer, std::vector<int>> layerClusterTruthClassHandle(m_layerClusterTruthClassKey, ctx);
    SG::WriteDecorHandle<xAOD::TrackParticleContainer, std::vector<bool>> layerClusterShadowedHandle(m_layerClusterShadowedKey, ctx);
    SG::WriteDecorHandle<xAOD::TrackParticleContainer, std::vector<bool>> layerClusterMergedHandle(m_layerClusterMergedKey, ctx);
    SG::WriteDecorHandle<xAOD::TrackParticleContainer, std::vector<bool>> layerPrimaryExpectedHandle(m_layerPrimaryExpectedKey, ctx);

    TruthTrackExtensionData data;
    for (const xAOD::TrackParticle* trackParticle : *trackParticles) {
      layerClusterTruthClassHandle(*trackParticle) = data.truthClassVec;
      layerClusterShadowedHandle(*trackParticle) = data.isShadowedVec;
      layerClusterMergedHandle(*trackParticle) = data.isMergedVec;
      layerPrimaryExpectedHandle(*trackParticle) = data.primaryExistsVec;
    }
  
    return StatusCode::SUCCESS;
  }

}
