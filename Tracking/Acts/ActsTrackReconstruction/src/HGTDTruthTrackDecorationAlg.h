/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration.
*/

#ifndef HGTDTRACKEXTENSIONALG_TRUTHDECORATION_H
#define HGTDTRACKEXTENSIONALG_TRUTHDECORATION_H

// Base Class
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "StoreGate/WriteDecorHandleKey.h"

namespace ActsTrk {
  
  class HGTDTruthTrackDecorationAlg
    : public AthReentrantAlgorithm {

  public:
    using AthReentrantAlgorithm::AthReentrantAlgorithm;
    virtual ~HGTDTruthTrackDecorationAlg() = default;
    
    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext&) const override;
    
  private:
    SG::ReadHandleKey<xAOD::TrackParticleContainer> m_trackParticleContainerName {this, "TrackParticleContainerName", "", "Name of the TrackParticle container"};
    
    // Truth decorations
    SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_layerClusterTruthClassKey { this, "LayerClusterTruthClass", m_trackParticleContainerName, "layerClusterTruthClass", "Decoration for cluster truth classification" };
    SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_layerClusterShadowedKey { this, "LayerClusterShadowed", m_trackParticleContainerName, "layerClusterShadowed", "Decoration for shadowed cluster" };
    SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_layerClusterMergedKey { this, "LayerClusterMerged", m_trackParticleContainerName, "layerClusterMerged", "Decoration for merged cluster" };
    SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_layerPrimaryExpectedKey { this, "LayerPrimaryExpected", m_trackParticleContainerName, "layerPrimaryExpected", "Decoration for primary expected cluster" };  
    
    /// @brief Data structure to hold HGTD track extension results
    /// Contains information about hits, timing, and extrapolation for each HGTD layer
    struct TruthTrackExtensionData {
      std::vector<int> truthClassVec = {-1, -1, -1, -1};             ///< Truth classification per HGTD layer
      std::vector<bool> isShadowedVec = {false, false, false, false}; ///< Whether cluster is shadowed per layer
      std::vector<bool> isMergedVec = {false, false, false, false};   ///< Whether cluster is merged per layer
      std::vector<bool> primaryExistsVec = {false, false, false, false}; ///< Whether primary is expected per layer
    };
    
  };
  
} // namespace ActsTrk

#endif


