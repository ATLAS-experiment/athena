/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSTRK_TRUTHTRACKBUILDERTOOL_H
#define ACTSTRK_TRUTHTRACKBUILDERTOOL_H

#include <Gaudi/Property.h>
#include "ActsEvent/MeasurementToTruthParticleAssociation.h"
#include "StoreGate/ReadHandleKey.h"
#include "xAODInDetMeasurement/PixelClusterContainer.h"
#include "xAODInDetMeasurement/StripClusterContainer.h"
#include "xAODTruth/TruthParticle.h"
#include "AthenaBaseComps/AthAlgTool.h"

namespace ActsTrk{
    
  // truth track builder class
  class TruthTrackBuilderTool final : public AthAlgTool {

    public:

    // defining all structs and type def used for naming
    using Vector3 = Eigen::Vector3f;

    struct TruthHit{

      const xAOD::UncalibratedMeasurement* hit;
      Vector3 globalPosition;
    };
        
    using TruthHits = std::vector<TruthHit>;
    using TruthTracks = std::unordered_map<const xAOD::TruthParticle*, TruthHits>;

    TruthTrackBuilderTool(const std::string& type,
                    const std::string& name,
                    const IInterface* parent);
                        
    StatusCode initialize() override;

    StatusCode buildTruthTracks(const EventContext& ctx,
                                TruthTracks& truthTracks) const ;

    private:
    // read handles for cluster to truth particle association
    // along with actual cluster information for coorindates
    // these will be used in all training algorithms
    // pixel
    SG::ReadHandleKey<xAOD::PixelClusterContainer> m_pixelClustersKey{this, "PixelClusters", "ITkPixelClusters", "Input reconstructed pixel clusters"};
    SG::ReadHandleKey<MeasurementToTruthParticleAssociation> m_pixelTruthAssociationKey{this, "PixelClustersToTruthAssociationMap", "ITkPixelClustersToTruthParticles", "Pixel cluster-to-truth-particle associations"};
    // strips
    SG::ReadHandleKey<xAOD::StripClusterContainer> m_stripClustersKey{this, "StripClusters", "ITkStripClusters", "Input reconstructed strip clusters"};
    SG::ReadHandleKey<MeasurementToTruthParticleAssociation> m_stripTruthAssociationKey{this, "StripClustersToTruthAssociationMap", "ITkStripClustersToTruthParticles", "Strip cluster-to-truth-particle associations"};
        
    // configurable gaudi properties for useinf strip, pixel or both clusters
    Gaudi::Property<bool> m_usePixelClusters{this, "usePixelClusters", true, "option for using pixel clusters"};
    Gaudi::Property<bool> m_useStripClusters{this, "useStripClusters", true, "option for using strip clusters"};

    // templated function to fill truth tracks map for both pixel and strip clusters
    // defined in the source file, where it is instantiated for the pixel and strip containers
    template <typename ClusterContainer>
    void addClusterToTruthTracks( const ClusterContainer& clusters,
                                 const MeasurementToTruthParticleAssociation& truthAssociations,
                                 TruthTracks& truthTracks) const;

  };
}

#endif