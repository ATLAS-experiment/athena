/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "src/StripClusterTruthDecoratorAlg.h"
#include "TruthUtils/HepMCHelpers.h"

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
        
    // Write keys
    ATH_CHECK(m_write_xaod_key.initialize());

    // Decorator
    m_measurement_truth_indices = m_write_xaod_key.key() + "." + m_measurement_truth_indices.key();
    m_measurement_truth_barcodes = m_write_xaod_key.key() + "." + m_measurement_truth_barcodes.key();
    ATH_CHECK(m_measurement_truth_indices.initialize(m_useTruthInfo));
    ATH_CHECK(m_measurement_truth_barcodes.initialize(m_useTruthInfo));
    
    return StatusCode::SUCCESS;
  }
  
  StatusCode StripClusterTruthDecoratorAlg::execute(const EventContext& ctx)  const {
    
    SG::ReadHandle<xAOD::StripClusterContainer> StripClusterContainer = SG::makeHandle(m_clustercontainer_key,ctx);
    ATH_CHECK(StripClusterContainer.isValid());
    const xAOD::StripClusterContainer *stripClusters = StripClusterContainer.cptr();

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
    xAOD::TrackMeasurementValidationContainer* measurements = xaod.ptr();

    std::vector<xAOD::TrackMeasurementValidation*> toAdd(stripClusters->size(), nullptr);
    for (std::size_t i(0); i<toAdd.size(); ++i) {
      toAdd[i] = new xAOD::TrackMeasurementValidation();
    }
    measurements->insert(measurements->end(), toAdd.begin(), toAdd.end());
    
    // loop over collection and convert to xAOD::TrackMeasurementValidation
    for (std::size_t i(0); i<stripClusters->size(); ++i) {
      const xAOD::StripCluster* cluster = stripClusters->at(i);
      xAOD::TrackMeasurementValidation *measurement = measurements->at(i);

      //This needs to be taken care of
      Identifier clusterId = Identifier(static_cast<int>(cluster->identifier()));
      if ( !clusterId.is_valid() ) {
	ATH_MSG_ERROR("Strip cluster identifier is not valid");
	return StatusCode::FAILURE;
      }

      //Set Identifier
      measurement->setIdentifier( clusterId.get_compact() );
      
      //Set Global Position
      auto gpos = cluster->globalPosition();
      measurement->setGlobalPosition(gpos.x(), gpos.y(), gpos.z());

      // Set local position and error matrix
      auto locpos = cluster->localPosition<1>();
      measurement->setLocalPosition(locpos[0],  locpos[1]); 
            
      auto localCov = cluster->localCovariance<1>();
      measurement->setLocalPositionError( localCov(0,0), 0., 0. ); 
    }

      
    // Get a list of all true particle contributing to the cluster
    if (m_useTruthInfo) {
      SG::WriteDecorHandle<xAOD::TrackMeasurementValidationContainer, std::vector<unsigned int>> decor_truth_indices( m_measurement_truth_indices, ctx );
      SG::WriteDecorHandle<xAOD::TrackMeasurementValidationContainer, std::vector<unsigned int>> decor_truth_barcode( m_measurement_truth_barcodes, ctx );
    
      for (std::size_t i(0); i<stripClusters->size(); ++i) {
	const xAOD::StripCluster* cluster = stripClusters->at(i);
	xAOD::TrackMeasurementValidation *measurement = measurements->at(i);
	
	if (cluster->index() >= measToTruth->size()) {
	  ATH_MSG_ERROR("PRD index "<< cluster->index() << " not present in the measurement to truth vector with size " << measToTruth->size());
	  return StatusCode::FAILURE;
	}
	
	auto tps = measToTruth->at(cluster->index());

	std::vector<unsigned int> tp_indices;
	std::vector<unsigned int> tp_barcodes;
	for (auto tp : tps) {
	  tp_indices.push_back(tp->index());
	  tp_barcodes.push_back(HepMC::barcode(tp));
	}
	
	decor_truth_indices(*measurement) = std::move(tp_indices);
	decor_truth_barcode(*measurement) = std::move(tp_barcodes);
      } // loop on clusters
    }
    
    ATH_MSG_DEBUG( " recorded StripPrepData objects: size " << measurements->size() );    
    return StatusCode::SUCCESS;
  }
  
} // ActsTrk
