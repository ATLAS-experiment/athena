/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TruthTrackBuilderTool.h"
#include "xAODTruth/TruthVertex.h"

namespace ActsTrk{

  TruthTrackBuilderTool::TruthTrackBuilderTool( const std::string& type,
                                                const std::string& name,
                                                const IInterface* parent)
    : AthAlgTool(type, name, parent){}
    
  StatusCode TruthTrackBuilderTool::initialize(){

    // initialize all the conatiners needed
    ATH_CHECK(m_pixelClustersKey.initialize());
    ATH_CHECK(m_pixelTruthAssociationKey.initialize());
    ATH_CHECK(m_stripClustersKey.initialize());
    ATH_CHECK(m_stripTruthAssociationKey.initialize());

    return StatusCode::SUCCESS;
  }

  template <typename ClusterContainer>
  void TruthTrackBuilderTool::addClusterToTruthTracks( const ClusterContainer& clusters,
                                                       const MeasurementToTruthParticleAssociation& truthAssociations,
                                                       TruthTracks& truthTracks) const {

    for (const auto* cluster : clusters){

      const auto& matchedTruthParticles = truthAssociations.at(cluster->index());

      // if no truth particles for cluster, skip
      if (matchedTruthParticles.empty()) {
          ATH_MSG_WARNING("empty truth particle vector for cluster, skipping");
          continue;
      }

      // only taking leading order truth particle 
      // (most likely to be the correct truth match as it has largest depsoit in cluster)
      const xAOD::TruthParticle* truthParticle = matchedTruthParticles.front();

      // obtain global position of cluster before upcasting (makes sorting easier)
      const auto& globalPosition = cluster->globalPosition();
      TruthHit hit{cluster, globalPosition};


      truthTracks[truthParticle].push_back(hit);
    }
  }

  StatusCode TruthTrackBuilderTool::buildTruthTracks(const EventContext& ctx, TruthTracks& truthTracks) const{

    // obtain truth map and clusters
    // for every cluster, obtain its truth particle and add to map
    // This can be optional for both pixel and strip clusters
    if (m_usePixelClusters) {
      SG::ReadHandle<xAOD::PixelClusterContainer> 
        pixelClusters{ m_pixelClustersKey, ctx};
      
      SG::ReadHandle<MeasurementToTruthParticleAssociation> 
        pixelTruthAssociations{m_pixelTruthAssociationKey, ctx};

      if (!pixelClusters.isValid()) {

        ATH_MSG_ERROR("Could not read pixel clusters: " << m_pixelClustersKey.key());
        return StatusCode::FAILURE;
      }

      if (!pixelTruthAssociations.isValid()) {

        ATH_MSG_ERROR( "Could not read pixel truth associations: " << m_pixelTruthAssociationKey.key());
        return StatusCode::FAILURE;
      }

      addClusterToTruthTracks(*pixelClusters,
                            *pixelTruthAssociations,
                            truthTracks);
    }

    if(m_useStripClusters) {

      SG::ReadHandle<xAOD::StripClusterContainer> 
      stripClusters{m_stripClustersKey, ctx};

      SG::ReadHandle<MeasurementToTruthParticleAssociation> 
        stripTruthAssociations{m_stripTruthAssociationKey, ctx};

      if (!stripClusters.isValid()) {

        ATH_MSG_ERROR("Could not read strip clusters: " << m_stripClustersKey.key());
        return StatusCode::FAILURE;
      }

      if (!stripTruthAssociations.isValid()) {

        ATH_MSG_ERROR("Could not read strip truth associations: " << m_stripTruthAssociationKey.key());
        return StatusCode::FAILURE;
      }

      addClusterToTruthTracks(*stripClusters,
                            *stripTruthAssociations,
                            truthTracks);
    }
    
    // reorder clusters for each associated truth particle
    // This is done by ordering by distance away from truth particle production vertex (assuming small bending)
    for (auto it = truthTracks.begin(); it != truthTracks.end();){
            
      auto& [truthParticle, truthClusters] = *it;

      if (truthParticle == nullptr || !truthParticle->hasProdVtx()) {

        ATH_MSG_WARNING(
            "Truth particle does not have a production vertex, erasing");
        it = truthTracks.erase(it);
        continue;
      }

      const xAOD::TruthVertex* vertex = truthParticle->prodVtx();
      const Vector3 vertexPosition{vertex->x(), vertex->y(), vertex->z()};
      std::sort(truthClusters.begin(), truthClusters.end(), 
                [&vertexPosition](const TruthHit& firstCluster, const TruthHit& secondCluster){

                  const Vector3 firstClusterDis = firstCluster.globalPosition - vertexPosition;
                  const Vector3 secondClusterDis = secondCluster.globalPosition - vertexPosition;

                  const float firstClusterNorm = firstClusterDis.squaredNorm();
                  const float secondClusterNorm = secondClusterDis.squaredNorm();

                  return firstClusterNorm < secondClusterNorm;

                  });
                  
      ++it;
    }
    
    return StatusCode::SUCCESS;
  }


}