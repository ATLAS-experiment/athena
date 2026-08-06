/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
*/

#include "src/HGTDTruthTrackDecorationAlg.h"
#include "StoreGate/WriteDecorHandle.h"

#include "ActsGeometry/ATLASMagneticFieldWrapper.h"
#include "ActsGeometryInterfaces/GeometryContext.h"
#include "ActsGeometry/ActsDetectorElement.h"
#include "ActsGeometry/SurfaceOfMeasurementUtil.h"
#include "GeoPrimitives/GeoPrimitivesToStringConverter.h"
#include "ActsGeometryInterfaces/ITrackingGeometrySvc.h"
#include "ActsCalibrators/xAODUncalibMeasSurfAcc.h"


// ActsTrk
#include "ActsCalibBase/CalibrationContext.h"
#include "ActsEvent/TrackContainer.h"
#include "src/detail/AtlasMeasurementSelector.h"
#include "src/detail/OnTrackCalibrator.h"
#include "src/detail/TrackFindingMeasurements.h"
#include "src/detail/MeasurementIndex.h"
#include "ActsEvent/ExpectedHitUtils.h"
#include "ActsEvent/TrackContainerHandlesHelper.h"
#include "ActsEvent/Decoration.h"

// STL
#include "ActsCalibrators/xAODUncalibMeasSurfAcc.h"

#include "StoreGate/ReadDecorHandle.h"
#include "Acts/Utilities/Helpers.hpp"


namespace ActsTrk{

  StatusCode HGTDTruthTrackDecorationAlg::initialize()
  {
    ATH_MSG_DEBUG("Initializing " << name() << "...");

    ATH_CHECK( m_trackParticleContainerName.initialize() );

    ATH_CHECK( m_layerClusterTruthClassKey.initialize() );
    ATH_CHECK( m_layerClusterShadowedKey.initialize() );
    ATH_CHECK( m_layerClusterMergedKey.initialize() );
    ATH_CHECK( m_layerPrimaryExpectedKey.initialize() );
    ATH_CHECK( m_trackContainerKey.initialize() );
    ATH_CHECK( m_hgtdTrackLinkKey.initialize() );
    ATH_CHECK( m_truthParticleLinkKey.initialize() );
    ATH_CHECK( m_trackingGeometrySvc.retrieve());
    ATH_CHECK( m_uncalibratedMeasurementContainerKey_HGTD.initialize() );

    // Initialize surface accessor
    m_surfAcc = ActsTrk::detail::xAODUncalibMeasSurfAcc{m_trackingGeometrySvc.get()};
     ATH_CHECK(m_hgtdClustersToTruth.initialize(SG::AllowEmpty));

    return StatusCode::SUCCESS;
  }

  StatusCode HGTDTruthTrackDecorationAlg::execute(const EventContext& ctx) const
  {
    ATH_MSG_DEBUG("Executing " << name() << "...");

    const xAOD::TrackParticleContainer* trackParticles {nullptr};
    ATH_CHECK(SG::get(trackParticles, m_trackParticleContainerName, ctx));

    ATH_MSG_DEBUG("Size of trackParticles collection " << trackParticles->size());
    // ================================================== //
    // ============ RETRIEVE MEASUREMENTS =============== //
    // ================================================== //

    ATH_MSG_DEBUG("Reading input collection with key " << m_uncalibratedMeasurementContainerKey_HGTD.key());

    const xAOD::UncalibratedMeasurementContainer* uncalibratedMeasurementContainer{nullptr};
    ATH_CHECK(SG::get(uncalibratedMeasurementContainer ,m_uncalibratedMeasurementContainerKey_HGTD, ctx));
    
    ATH_MSG_DEBUG("Retrieved " << uncalibratedMeasurementContainer->size()
                    << " input elements from key " << m_uncalibratedMeasurementContainerKey_HGTD.key());


    // ================================================== //
    // ============ TRUTH MATCHING BLOCK ================ //
    // ================================================== //

    SG::ReadDecorHandle<xAOD::TrackParticleContainer, ElementLink<ActsTrk::TrackContainer>> hgtdTrackLink( m_hgtdTrackLinkKey, ctx );

    SG::ReadDecorHandle<xAOD::TrackParticleContainer, ElementLink<xAOD::TruthParticleContainer>> truthParticleLink( m_truthParticleLinkKey, ctx );

    std::array<const ActsTrk::MeasurementToTruthParticleAssociation *,
    Acts::toUnderlying(xAOD::UncalibMeasType::nTypes)>
    measurement_to_truth_association_maps{};

    if (!m_hgtdClustersToTruth.key().empty()) {
    const ActsTrk::MeasurementToTruthParticleAssociation* hgtdClustersToTruthAssociation{};
    ATH_CHECK(SG::get(hgtdClustersToTruthAssociation, m_hgtdClustersToTruth, ctx));
       measurement_to_truth_association_maps[Acts::toUnderlying(xAOD::UncalibMeasType::HGTDClusterType)]=hgtdClustersToTruthAssociation;
    }
    auto assocSize = [&measurement_to_truth_association_maps](xAOD::UncalibMeasType type) {
    const ActsTrk::MeasurementToTruthParticleAssociation *assoc = measurement_to_truth_association_maps[Acts::toUnderlying(type)];
    return assoc ? assoc->size() : 0ul;
    };

    ATH_MSG_DEBUG("Measurement association entries: "  << assocSize(xAOD::UncalibMeasType::PixelClusterType)
        << " + " << assocSize(xAOD::UncalibMeasType::StripClusterType)
        << " + " << assocSize(xAOD::UncalibMeasType::HGTDClusterType)
        );


    SG::ReadHandle<ActsTrk::TrackContainer> tracksContainer = SG::makeHandle( m_trackContainerKey, ctx);
    if (!tracksContainer.isValid()) {
      ATH_MSG_ERROR("No tracks for key " << m_trackContainerKey.key() );
      return StatusCode::FAILURE;
    }
    // Create WriteDecorHandles for all decorations
    SG::WriteDecorHandle<xAOD::TrackParticleContainer, std::vector<int>> layerClusterTruthClassHandle(m_layerClusterTruthClassKey, ctx);
    SG::WriteDecorHandle<xAOD::TrackParticleContainer, std::vector<char>> layerClusterShadowedHandle(m_layerClusterShadowedKey, ctx);
    SG::WriteDecorHandle<xAOD::TrackParticleContainer, std::vector<char>> layerClusterMergedHandle(m_layerClusterMergedKey, ctx);
    SG::WriteDecorHandle<xAOD::TrackParticleContainer, std::vector<char>> layerPrimaryExpectedHandle(m_layerPrimaryExpectedKey, ctx);

    TruthTrackExtensionData data;

    for (const xAOD::TrackParticle* trackParticle : *trackParticles) {

      // get the truth particle
      ElementLink<xAOD::TruthParticleContainer> link_to_truth = truthParticleLink(*trackParticle);
      const xAOD::TruthParticle* truthParticle = nullptr;
      if (!link_to_truth.isValid()) {
        layerClusterTruthClassHandle(*trackParticle) = {-1, -1, -1, -1};
        layerClusterShadowedHandle(*trackParticle) = {false, false, false, false};
        layerClusterMergedHandle(*trackParticle)  = {false, false, false, false};
        layerPrimaryExpectedHandle(*trackParticle) = {false, false, false, false};
        ATH_MSG_WARNING("TrackParticle " << trackParticle->index() << ": invalid truth link");
        continue;
      }
      else {
        truthParticle = *link_to_truth;
      }
      // Check if the TrackParticle has a link to an hgtd extension track
      if(hgtdTrackLink.isAvailable()){
        ElementLink<ActsTrk::TrackContainer> link_to_track = hgtdTrackLink(*trackParticle);
        if (!link_to_track.isValid()) {
          ATH_MSG_DEBUG("TrackParticle " << trackParticle->index() << ": invalid extension link");
          data.truthClassVec = {-1, -1, -1, -1};
          data.isShadowedVec = {false, false, false, false};
          data.isMergedVec   = {false, false, false, false};
        }
        else{
          std::optional<ActsTrk::TrackContainer::ConstTrackProxy> optional_track = *link_to_track;
          const ActsTrk::TrackContainer::ConstTrackProxy& track = optional_track.value();
          ATH_MSG_DEBUG("TrackParticle " << trackParticle->index() << ": extension  with  chi2 " <<  track.chi2() << " and nHits " << track.nMeasurements());
          data = createTruthDecoration(truthParticle, track, measurement_to_truth_association_maps[Acts::toUnderlying(xAOD::UncalibMeasType::HGTDClusterType)]);
        }
      }
      

      ATH_CHECK(isPrimaryExpected(truthParticle,
        *uncalibratedMeasurementContainer,
        measurement_to_truth_association_maps[Acts::toUnderlying(xAOD::UncalibMeasType::HGTDClusterType)],
        data.primaryExistsVec));

      layerClusterTruthClassHandle(*trackParticle) = data.truthClassVec;
      layerClusterShadowedHandle(*trackParticle) = data.isShadowedVec;
      layerClusterMergedHandle(*trackParticle) = data.isMergedVec;
      layerPrimaryExpectedHandle(*trackParticle) = data.primaryExistsVec;

    }
    ATH_MSG_DEBUG("Finshed truth matching");
  
    return StatusCode::SUCCESS;
  }

  HGTDTruthTrackDecorationAlg::TruthTrackExtensionData HGTDTruthTrackDecorationAlg::createTruthDecoration(
    const xAOD::TruthParticle* truthParticle,
    const typename ActsTrk::TrackContainer::ConstTrackProxy trackProxy,
    const ActsTrk::MeasurementToTruthParticleAssociation* association_map) const{
  
    TruthTrackExtensionData data;

    std::vector<int> truthClassPerLayer = {-1, -1, -1, -1};
    std::vector<char> isShadowedPerLayer = {false, false, false, false};
    std::vector<char> isMergedPerLayer = {false, false, false, false};
    std::vector<char> isPrimaryExistsVec = {false, false, false, false};

    for (auto state : trackProxy.trackStatesReversed()) {
      auto flags = state.typeFlags();
      if (flags.isMeasurement()) {
        // Check if this is an HGTD hit 
        const auto& surface = state.referenceSurface();
        Acts::GeometryIdentifier geoID = surface.geometryId();
                              
        std::size_t layerIndex = getHGTDLayerIndex(geoID);
        ClusterTruthInfo cluster_truth_info;
        
        assert( state.hasUncalibratedSourceLink() );
        auto uncalibMeas = detail::xAODUncalibMeasCalibrator::unpack(state.getUncalibratedSourceLink());
      
        if (association_map->at(uncalibMeas->index()).empty()) {
          cluster_truth_info.origin = ActsTrk::ClusterTruthOrigin::SECONDARY;
          ATH_MSG_DEBUG("    \\__Layer "<< layerIndex << " SECONDARY: hit doesnt have truth particle associated with it");
        }
        else if(association_map->at(uncalibMeas->index()).size() == 1) {
          const xAOD::TruthParticle *truth_particle = association_map->at(uncalibMeas->index()).at(0);
          if(truthParticle->index() == truth_particle->index()){
            cluster_truth_info.origin = ActsTrk::ClusterTruthOrigin::TRUTH_PARTICLE;
            ATH_MSG_DEBUG("    \\__Layer "<< layerIndex<< " TRUTH_PARTICLE: hit matches ITk track");
          }
          else{
            cluster_truth_info.origin = ActsTrk::ClusterTruthOrigin::UNRELATED_PARTICLE;
            ATH_MSG_DEBUG("    \\__Layer "<< layerIndex<< " UNRELATED_PARTICLE: hit doesnt match ITk track");
          }
        }
        else {
          cluster_truth_info.is_merged = true;
          cluster_truth_info.origin = ActsTrk::ClusterTruthOrigin::UNRELATED_PARTICLE;
          ATH_MSG_DEBUG("    \\__Layer "<< layerIndex<< " MERGED: hit with more than one contributing particle");
          for (const xAOD::TruthParticle *truth_particle : association_map->at(uncalibMeas->index()) ) {
            if(truthParticle->index() == truth_particle->index()){
              cluster_truth_info.origin = ActsTrk::ClusterTruthOrigin::TRUTH_PARTICLE;
              ATH_MSG_DEBUG("         \\__TRUTH_PARTICLE: hit matches ITk track");
            }   
          }
        }
      
        
        truthClassPerLayer[layerIndex]      = (int) cluster_truth_info.origin;
        isShadowedPerLayer[layerIndex]      = cluster_truth_info.is_shadowed;
        isMergedPerLayer[layerIndex]        = cluster_truth_info.is_merged;
      }
    }

    // Fill the data structure with results
    data.truthClassVec      = std::move(truthClassPerLayer);
    data.isShadowedVec      = std::move(isShadowedPerLayer);
    data.isMergedVec        = std::move(isMergedPerLayer);
    data.primaryExistsVec   = std::move(isPrimaryExistsVec);
    return data;
  }
  
  StatusCode HGTDTruthTrackDecorationAlg::isPrimaryExpected(
    const xAOD::TruthParticle* truthParticle,
    const xAOD::UncalibratedMeasurementContainer & measurementContainer,
    const ActsTrk::MeasurementToTruthParticleAssociation* association_map,
    std::vector<char> &isPrimaryExistsVec) const{

    isPrimaryExistsVec = {false, false, false, false};
  
    for(auto uncalibMeas: measurementContainer) {

      auto measurementTruthParticles = association_map->at(uncalibMeas->index());
      const Acts::Surface* surface = m_surfAcc.get(uncalibMeas);
      Acts::GeometryIdentifier geoID = surface->geometryId();
      std::size_t layerIndex = getHGTDLayerIndex(geoID);
      if(measurementTruthParticles.size() > 0){
        for(auto measTruthParticle : measurementTruthParticles){
          if ( truthParticle->index() == measTruthParticle->index()){
            isPrimaryExistsVec[layerIndex] = true;
            ATH_MSG_DEBUG("         \\__HIT Exepected at " << layerIndex);
          }
        }
      }
    }
  
    return StatusCode::SUCCESS; 
  }

  std::size_t HGTDTruthTrackDecorationAlg::getHGTDLayerIndex(const Acts::GeometryIdentifier& geoID) const {
    // Get volume and layer ID
    std::uint32_t volume = geoID.volume();
    std::uint32_t layer = geoID.layer();
    
    // Check if we're in the positive or negative endcap 
    bool isPositiveEndcap = (volume == 25); 
    bool isNegativeEndcap = (volume == 2); 
    
    // Different mapping for different sides to maintain consistent physical ordering
    if (isPositiveEndcap) {
      // Mapping for positive endcap
      switch(layer) {
        case 2: return 0;  // First HGTD layer (closest to IP)
        case 4: return 1;  // Second HGTD layer
        case 6: return 2;  // Third HGTD layer
        case 8: return 3;  // Fourth HGTD layer (farthest from IP)
        default: return 99; // Invalid layer
      }
    } else if (isNegativeEndcap) {
      // Mapping for negative endcap - potentially different ordering
      switch(layer) {
        case 2: return 3; 
        case 4: return 2;  
        case 6: return 1;
        case 8: return 0;
        default: return 99; // Invalid layer
      }
    } else {
      return 99; // Not an HGTD volume
    }
  }
  
}
