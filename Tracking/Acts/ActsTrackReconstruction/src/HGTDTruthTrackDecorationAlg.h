/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration.
*/

#ifndef HGTDTRACKEXTENSIONALG_TRUTHDECORATION_H
#define HGTDTRACKEXTENSIONALG_TRUTHDECORATION_H

// Base Class
#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "xAODTracking/TrackParticleContainer.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "StoreGate/ReadDecorHandleKey.h"
#include "StoreGate/ReadHandleKey.h"
#include "ActsEvent/TrackContainer.h"
#include "ActsEvent/MeasurementToTruthParticleAssociation.h"
#include "Identifier/Identifier.h"

#include "src/detail/AtlasMeasurementSelector.h"
#include "src/detail/OnTrackCalibrator.h"
#include "src/detail/TrackFindingMeasurements.h"
#include "src/detail/MeasurementIndex.h"
#include "src/detail/Definitions.h"

#include "ActsCalibrators/xAODUncalibMeasSurfAcc.h"
#include "ActsGeometryInterfaces/ITrackingGeometrySvc.h"


#include "xAODTruth/TruthParticleContainer.h"

namespace ActsTrk {
  
  using AtlUncalibSourceLinkAccessor = detail::UncalibSourceLinkAccessor;

  enum class ClusterTruthOrigin {
    UNIDENTIFIED = 0,       // ITk track is not associated with a truth particle, so not identified
    TRUTH_PARTICLE = 1,     // originates from the tested truth particle
    UNRELATED_PARTICLE = 2, // originates from a particle that is unrelated to the ITk track
    SECONDARY = 3           // originates from some secondary interaction
  };

  class HGTDTruthTrackDecorationAlg
    : public AthReentrantAlgorithm {

  public:
    using AthReentrantAlgorithm::AthReentrantAlgorithm;
    virtual ~HGTDTruthTrackDecorationAlg() = default;
    
    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext&) const override;
    
  private:

    //HGTD Clusters and Cluster to Truth
    SG::ReadHandleKey<xAOD::TrackParticleContainer> m_trackParticleContainerName {this, "TrackParticleContainerName", "", "Name of the TrackParticle container"};
    SG::ReadHandleKey<ActsTrk::TrackContainer> m_trackContainerKey{this, "ACTSTracksLocation", "HgtdTracks", "Output track collection (ActsTrk variant)"};
    SG::ReadHandleKey<xAOD::UncalibratedMeasurementContainer> m_uncalibratedMeasurementContainerKey_HGTD{this, "UncalibratedMeasurementContainerKey_HGTD", "HGTD_Clusters", "input cluster collections for HGTD"};
    SG::ReadHandleKey<MeasurementToTruthParticleAssociation>  m_hgtdClustersToTruth { this, "HgtdClustersToTruthAssociationMap", "HgtdClustersToTruthParticles", "Association map from HGTD measurements to generator particles." };

    SG::ReadDecorHandleKey<xAOD::TrackParticleContainer> m_hgtdTrackLinkKey {this, "hgtdTrackLink", m_trackParticleContainerName, "hgtdTrackLink", "Link to Hgtd track"};
    SG::ReadDecorHandleKey<xAOD::TrackParticleContainer> m_truthParticleLinkKey {this, "truthParticleLink", m_trackParticleContainerName, "truthParticleLink", "Link to Truth particle"};
    
    // Truth decorations
    SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_layerClusterTruthClassKey { this, "HGTD_cluster_truth_class", m_trackParticleContainerName, "HGTD_cluster_truth_class", "Decoration for cluster truth classification" };
    SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_layerClusterShadowedKey { this, "HGTD_cluster_shadowed", m_trackParticleContainerName, "HGTD_cluster_shadowed", "Decoration for shadowed cluster" };
    SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_layerClusterMergedKey { this, "HGTD_cluster_merged", m_trackParticleContainerName, "HGTD_cluster_merged", "Decoration for merged cluster" };
    SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_layerPrimaryExpectedKey { this, "HGTD_primary_expected", m_trackParticleContainerName, "HGTD_primary_expected", "Decoration for primary expected cluster" };  
      
    ActsTrk::detail::xAODUncalibMeasSurfAcc m_surfAcc{};
   ServiceHandle<ActsTrk::ITrackingGeometrySvc> m_trackingGeometrySvc{this, "TrackingGeometrySvc", "ActsTrackingGeometrySvc"};

    std::unique_ptr<SG::AuxElement::Accessor<int>> m_acc_nHgtdHits;

    /// @brief Data structure to hold truth information about the HGTD track extension
    struct TruthTrackExtensionData {
      std::vector<int> truthClassVec = {-1, -1, -1, -1};                 ///< Truth classification per HGTD layer
      std::vector<char> isShadowedVec = {false, false, false, false};    ///< Whether cluster is shadowed per layer
      std::vector<char> isMergedVec = {false, false, false, false};      ///< Whether cluster is merged per layer
      std::vector<char> primaryExistsVec = {false, false, false, false}; ///< Whether primary is expected per layer
    };
    
    struct ClusterTruthInfo {
      ClusterTruthOrigin origin = ClusterTruthOrigin::UNIDENTIFIED;
      /**
       * @brief Shadowing means that a deposit was left by the truth
       * particle, but it was not the first deposit and is thus not used for the
       * time measurement -> I will have an incorrect time
       */
      bool is_shadowed = false;
      /**
       * @brief A cluster is considered to be merged if more than one particle
       * deposited energy in a given pad.
       */
      bool is_merged = false;
    };

    /**
      * @brief Generate TruthTrackExtensionData from extension
      *
      * @param truthParticle - truth particle associated with track
      * @param trackProxy - measurement container with HGTD clusters
      * @param association_map - hgtd cluster to truth particles map  
      */
    TruthTrackExtensionData createTruthDecoration(
      const xAOD::TruthParticle* truthParticle,
      const typename ActsTrk::TrackContainer::ConstTrackProxy trackProxy,
      const ActsTrk::MeasurementToTruthParticleAssociation* association_map) const; 
    
    /**
      * @brief Checks if truth particle produced hits at each one of the HGTD layers
      *
      * @param truthParticle - truth particle associated with track
      * @param measurementContainer - measurement container with HGTD clusters
      * @param association_map - hgtd cluster to truth particles map 
      * @param isPrimaryExistsVec - vector to be return with the information about the  
      */
    StatusCode isPrimaryExpected(
      const xAOD::TruthParticle* truthParticle,
      const xAOD::UncalibratedMeasurementContainer measurementContainer,
      const ActsTrk::MeasurementToTruthParticleAssociation* association_map,
      std::vector<char> &isPrimaryExistsVec) const;    

    /**
      * @brief returns the index of HGTD layer where surfaces lies.
      * This index is used at to locate where in the vectors of
      * TrackExtensionData the hit information should be written
      * Returns 99 if surface is outiside of HGTD.
      *
      * @param geoID - surface geometry id
      */  
    std::size_t getHGTDLayerIndex(const Acts::GeometryIdentifier& geoID) const;
    
  };
  
} // namespace ActsTrk

#endif


