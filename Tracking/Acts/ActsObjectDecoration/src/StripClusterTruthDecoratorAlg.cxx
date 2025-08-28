/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "src/StripClusterTruthDecoratorAlg.h"
#include "TruthUtils/HepMCHelpers.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadDecorHandle.h"
#include "ActsEvent/TrackContainer.h"
#include "ActsGeometry/ATLASSourceLink.h"

namespace ActsTrk {

  StripClusterTruthDecoratorAlg::StripClusterTruthDecoratorAlg(const std::string& name,
							       ISvcLocator *pSvcLocator) :
    AthReentrantAlgorithm(name, pSvcLocator)
  {}

  
  StatusCode StripClusterTruthDecoratorAlg::initialize() {
    ATH_MSG_DEBUG("Initialize " << name() << " ...");
    
    // Read keys
    ATH_CHECK(m_clustercontainer_key.initialize());
    ATH_CHECK(m_associationMap_key.initialize(m_useTruthInfo));
    ATH_CHECK(m_stripDetEleCollKey.initialize());

    // Tracks only needed if we want on track clusters only
    ATH_CHECK(m_trackParticlesKey.initialize(m_keepOnlyOnTrackMeasurements));
    
    // Write keys
    ATH_CHECK(m_write_xaod_key.initialize());

    // Decorator
    ATH_CHECK(m_trackMeasurement_link.initialize());
    
    ATH_CHECK(m_measurement_truth_indices.initialize(m_useTruthInfo));
    ATH_CHECK(m_measurement_truth_barcodes.initialize(m_useTruthInfo));

    ATH_CHECK(m_measurement_detectorElementID.initialize());
    ATH_CHECK(m_measurement_waferID.initialize());
    ATH_CHECK(m_measurement_bec.initialize());
    ATH_CHECK(m_measurement_layer.initialize());
    ATH_CHECK(m_measurement_sizePhi.initialize());
    ATH_CHECK(m_measurement_sizeZ.initialize());
    ATH_CHECK(m_measurement_SiWidth.initialize());
    ATH_CHECK(m_measurement_eta_module.initialize());
    ATH_CHECK(m_measurement_phi_module.initialize());
    ATH_CHECK(m_measurement_omegax.initialize());
    ATH_CHECK(m_measurement_omegay.initialize());
    ATH_CHECK(m_measurement_LorentzShift.initialize());
    ATH_CHECK(m_measurement_centroid_xphi.initialize());
    ATH_CHECK(m_measurement_centroid_xeta.initialize());
    ATH_CHECK(m_measurement_side.initialize());

    ATH_CHECK(detStore()->retrieve(m_stripHelper, "SCT_ID"));
    
    return StatusCode::SUCCESS;
  }
  
  StatusCode StripClusterTruthDecoratorAlg::execute(const EventContext& ctx)  const {
    
    SG::ReadHandle<xAOD::StripClusterContainer> StripClusterContainer = SG::makeHandle(m_clustercontainer_key,ctx);
    ATH_CHECK(StripClusterContainer.isValid());
    const xAOD::StripClusterContainer *stripClusters = StripClusterContainer.cptr();

    SG::ReadCondHandle<InDetDD::SiDetectorElementCollection> stripDetEleHandle = SG::makeHandle( m_stripDetEleCollKey, ctx );
    ATH_CHECK(stripDetEleHandle.isValid());
    const InDetDD::SiDetectorElementCollection* stripElements = stripDetEleHandle.cptr();
    
    const ActsTrk::MeasurementToTruthParticleAssociation* measToTruth(nullptr);
    if (m_useTruthInfo) {
      SG::ReadHandle<ActsTrk::MeasurementToTruthParticleAssociation> measToTruthHandle = SG::makeHandle(m_associationMap_key,ctx);
      ATH_CHECK( measToTruthHandle.isValid());
      measToTruth = measToTruthHandle.ptr();
    }

    // Setup outputs
    // Create the xAOD container and its auxiliary store:
    SG::WriteHandle<xAOD::TrackMeasurementValidationContainer> xaod = SG::makeHandle( m_write_xaod_key,ctx );
    ATH_CHECK(xaod.record(std::make_unique<xAOD::TrackMeasurementValidationContainer>(),
			  std::make_unique<xAOD::TrackMeasurementValidationAuxContainer>()));

    SG::WriteDecorHandle<xAOD::StripClusterContainer,
			 ElementLink< xAOD::TrackMeasurementValidationContainer > > decorator_measurement_link( m_trackMeasurement_link, ctx );
    
    SG::WriteDecorHandle<xAOD::TrackMeasurementValidationContainer, std::uint64_t> decor_detectorElementID ( m_measurement_detectorElementID, ctx );
    SG::WriteDecorHandle<xAOD::TrackMeasurementValidationContainer, int> decor_waferID ( m_measurement_waferID, ctx );
    SG::WriteDecorHandle<xAOD::TrackMeasurementValidationContainer, int> decor_bec ( m_measurement_bec, ctx );
    SG::WriteDecorHandle<xAOD::TrackMeasurementValidationContainer, int> decor_layer ( m_measurement_layer, ctx );
    SG::WriteDecorHandle<xAOD::TrackMeasurementValidationContainer, int> decor_sizePhi ( m_measurement_sizePhi, ctx );
    SG::WriteDecorHandle<xAOD::TrackMeasurementValidationContainer, int> decor_sizeZ ( m_measurement_sizeZ, ctx );
    SG::WriteDecorHandle<xAOD::TrackMeasurementValidationContainer, int> decor_SiWidth( m_measurement_SiWidth, ctx );
    SG::WriteDecorHandle<xAOD::TrackMeasurementValidationContainer, int> decor_eta_module ( m_measurement_eta_module, ctx );
    SG::WriteDecorHandle<xAOD::TrackMeasurementValidationContainer, int> decor_phi_module ( m_measurement_phi_module, ctx );
    SG::WriteDecorHandle<xAOD::TrackMeasurementValidationContainer, float> decor_omegax ( m_measurement_omegax, ctx );
    SG::WriteDecorHandle<xAOD::TrackMeasurementValidationContainer, float> decor_omegay ( m_measurement_omegay, ctx );
    SG::WriteDecorHandle<xAOD::TrackMeasurementValidationContainer, float> decor_LorentzShift ( m_measurement_LorentzShift, ctx );
    SG::WriteDecorHandle<xAOD::TrackMeasurementValidationContainer, float> decor_centroid_xphi ( m_measurement_centroid_xphi, ctx );
    SG::WriteDecorHandle<xAOD::TrackMeasurementValidationContainer, float> decor_centroid_xeta ( m_measurement_centroid_xeta, ctx );
    SG::WriteDecorHandle<xAOD::TrackMeasurementValidationContainer, int> decor_side ( m_measurement_side, ctx );

    // collection
    std::vector<bool> keepClusterCollection {};
    ATH_CHECK( labelMeasurementToKeep(ctx, *stripClusters, keepClusterCollection) );
    xAOD::TrackMeasurementValidationContainer *measurements = xaod.ptr();

    // loop over collection and convert to xAOD::TrackMeasurementValidation
    for (std::size_t i(0); i<stripClusters->size(); ++i) {
      if (not keepClusterCollection[i]) continue;
      const xAOD::StripCluster* cluster = stripClusters->at(i);
      
      measurements->push_back( new xAOD::TrackMeasurementValidation() );
      xAOD::TrackMeasurementValidation* measurement = measurements->back();
      ElementLink< xAOD::TrackMeasurementValidationContainer > mlink( measurements,
								      measurements->back()->index() );
      ATH_CHECK( mlink.isValid() );
      decorator_measurement_link(*cluster) = std::move(mlink);
    
      xAOD::DetectorIdentType clusterId = cluster->identifier();
      xAOD::DetectorIDHashType hashId = cluster->identifierHash();

      const InDetDD::SiDetectorElement *element = stripElements->getDetectorElement(hashId);
      if ( not element ) {
	ATH_MSG_FATAL( "Invalid strip detector element for hash " << hashId );
	return StatusCode::FAILURE;
      }
      
      const std::vector<Identifier> rdoList = cluster->rdoList();
      std::vector< std::uint64_t > rdoIdentifierList;
      rdoIdentifierList.reserve(rdoList.size());
      for( const Identifier& hitIdentifier : rdoList ){
	rdoIdentifierList.push_back( hitIdentifier.get_compact() );
      }
    
      //Set Identifier
      measurement->setIdentifier( clusterId );
      measurement->setRdoIdentifierList(std::move(rdoIdentifierList));
      
      //Set Global Position
      auto gpos = cluster->globalPosition();
      measurement->setGlobalPosition(gpos.x(), gpos.y(), gpos.z());

      // Set local position and error matrix
      auto locpos = cluster->localPosition<1>();
      measurement->setLocalPosition(locpos[0],  locpos[1]); 
            
      auto localCov = cluster->localCovariance<1>();
      measurement->setLocalPositionError( localCov(0,0), 0., 0. );

      const Identifier waferId = m_stripHelper->wafer_id(hashId);
      decor_detectorElementID(*measurement) = hashId;
      decor_waferID(*measurement) = waferId.get_compact();
      decor_bec(*measurement) = m_stripHelper->barrel_ec(waferId);
      decor_layer(*measurement) = m_stripHelper->layer_disk(waferId);
      decor_sizePhi(*measurement) = cluster->channelsInPhi();
      decor_sizeZ(*measurement) = 0;
      decor_SiWidth(*measurement) = cluster->channelsInPhi();
      decor_eta_module(*measurement) = m_stripHelper->eta_module(waferId);
      decor_phi_module(*measurement) = m_stripHelper->phi_module(waferId);
      decor_omegax(*measurement) = 0;
      decor_omegay(*measurement) = 0;
      decor_LorentzShift(*measurement) = 0;
      decor_centroid_xphi(*measurement) = 0;
      decor_centroid_xeta(*measurement) = 0;
      decor_side(*measurement) = m_stripHelper->side(waferId);
    }

      
    // Get a list of all true particle contributing to the cluster
    if (m_useTruthInfo) {
      SG::WriteDecorHandle<xAOD::TrackMeasurementValidationContainer, std::vector<unsigned int>> decor_truth_indices( m_measurement_truth_indices, ctx );
      SG::WriteDecorHandle<xAOD::TrackMeasurementValidationContainer, std::vector<unsigned int>> decor_truth_barcode( m_measurement_truth_barcodes, ctx );

      std::size_t measurementIndex = 0;
      for (std::size_t i(0); i<stripClusters->size(); ++i) {
	if (not keepClusterCollection[i]) continue;
	const xAOD::StripCluster* cluster = stripClusters->at(i);
	xAOD::TrackMeasurementValidation* measurement = measurements->at(measurementIndex);

	// Check the two match
	if (cluster->identifier() != measurement->identifier()) {
	  ATH_MSG_ERROR("Cluster and Measurement are not matching!");
	  return StatusCode::FAILURE;
	}
	
	if (cluster->index() >= measToTruth->size()) {
	  ATH_MSG_ERROR("PRD index "<< cluster->index() << " not present in the measurement to truth vector with size " << measToTruth->size());
	  return StatusCode::FAILURE;
	}
	
	auto tps = measToTruth->at(cluster->index());

	std::vector<unsigned int> tp_indices;
	std::vector<unsigned int> tp_barcodes;
	for (auto tp : tps) {
	  tp_indices.push_back(tp->index());
	  tp_barcodes.push_back(HepMC::uniqueID(tp));
	}
	
	decor_truth_indices(*measurement) = std::move(tp_indices);
	decor_truth_barcode(*measurement) = std::move(tp_barcodes);
	++measurementIndex;
      } // loop on clusters
    }
    
    ATH_MSG_DEBUG( " recorded StripPrepData objects: size " << measurements->size() );    
    return StatusCode::SUCCESS;
  }

StatusCode StripClusterTruthDecoratorAlg::labelMeasurementToKeep(const EventContext& ctx,
                                                                 const xAOD::StripClusterContainer& clusters,
                                                                 std::vector<bool>& labels) const
{
  labels.clear();
  if (not m_keepOnlyOnTrackMeasurements) {
    labels.resize(clusters.size(), true);
    return StatusCode::SUCCESS;
  }
  labels.resize(clusters.size(), false);

   static const SG::ConstAccessor< ElementLink<ActsTrk::TrackContainer> > decorator_trackLink("actsTrack");
   
  // get the tracks
  for (const SG::ReadHandleKey<xAOD::TrackParticleContainer>& trackParticlesKey : m_trackParticlesKey) {
    SG::ReadHandle<xAOD::TrackParticleContainer> trackParticleHandle = SG::makeHandle( trackParticlesKey, ctx );
    ATH_CHECK(trackParticleHandle.isValid());
    const xAOD::TrackParticleContainer* trackParticles = trackParticleHandle.cptr();
    
    for (const xAOD::TrackParticle* trackParticle : *trackParticles) {
      // Get the ACTS track object
      ATH_CHECK( decorator_trackLink.isAvailable(*trackParticle) );
      ElementLink<ActsTrk::TrackContainer> trackLink = decorator_trackLink(*trackParticle);
      ATH_CHECK(trackLink.isValid());
      
      std::optional<ActsTrk::TrackContainer::ConstTrackProxy> optional_track = *trackLink;
      if ( not optional_track.has_value() ) {
	ATH_MSG_ERROR("Invalid track link for particle  " << trackParticle->index());
	return StatusCode::FAILURE;
      }
      ActsTrk::TrackContainer::ConstTrackProxy track = optional_track.value();
      
      // loop on track states
      track.container().trackStateContainer()
	.visitBackwards(track.tipIndex(),
			[&labels]
			(const typename ActsTrk::TrackContainer::ConstTrackStateProxy& state)
			{
			  auto flags = state.typeFlags();
			  if (not flags.test(Acts::TrackStateFlag::MeasurementFlag) and
			      not flags.test(Acts::TrackStateFlag::OutlierFlag)) return;
			  
			  auto sl = state.getUncalibratedSourceLink().template get<ATLASUncalibSourceLink>();
			  if (sl == nullptr) return;
			  
			  const xAOD::UncalibratedMeasurement &cluster = getUncalibratedMeasurement(sl);
			  if (cluster.type() != xAOD::UncalibMeasType::StripClusterType) return;
			  labels.at(cluster.index()) = true;
			});    
    } // loop on tracks
  } // loop on read handle keys
  
  return StatusCode::SUCCESS;
}
  
} // ActsTrk
