/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "src/PixelClusterTruthDecoratorAlg.h"
#include "TruthUtils/HepMCHelpers.h"
#include "PixelReadoutGeometry/PixelModuleDesign.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"

namespace ActsTrk {
  
  PixelClusterTruthDecoratorAlg::PixelClusterTruthDecoratorAlg(const std::string& name,
							       ISvcLocator *pSvcLocator) :
    AthReentrantAlgorithm(name, pSvcLocator)
  {}

  StatusCode PixelClusterTruthDecoratorAlg::initialize() {
    ATH_MSG_DEBUG("Initialize " << name() << " ...");
    
    // Read keys
    ATH_CHECK(m_clustercontainer_key.initialize());
    ATH_CHECK(m_associationMap_key.initialize(m_useTruthInfo));
    ATH_CHECK(m_pixelDetEleCollKey.initialize());
    
    // Write keys
    ATH_CHECK(m_write_xaod_key.initialize());

    // Decorators
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

    ATH_CHECK(m_measurement_tots.initialize());
    
    ATH_CHECK( m_lorentzAngleTool.retrieve() );
    ATH_CHECK( detStore()->retrieve(m_PixelHelper, "PixelID") );
    
    return StatusCode::SUCCESS;
  }
  
  StatusCode PixelClusterTruthDecoratorAlg::execute(const EventContext& ctx) const {

  //Mandatory. Require if the algorithm is scheduled.
  SG::ReadHandle<xAOD::PixelClusterContainer> PixelClusterContainer = SG::makeHandle(m_clustercontainer_key,ctx);
  ATH_CHECK(PixelClusterContainer.isValid());
  const xAOD::PixelClusterContainer* clusters = PixelClusterContainer.cptr();

  SG::ReadCondHandle<InDetDD::SiDetectorElementCollection> pixelDetEleHandle = SG::makeHandle( m_pixelDetEleCollKey, ctx );
  ATH_CHECK(pixelDetEleHandle.isValid());
  const InDetDD::SiDetectorElementCollection* pixelDetEle = pixelDetEleHandle.cptr();
    
  // Get the truth association map
  const MeasurementToTruthParticleAssociation* measToTruth(nullptr);
  if (m_useTruthInfo) {
    SG::ReadHandle<MeasurementToTruthParticleAssociation> measToTruthHandle = SG::makeHandle(m_associationMap_key,ctx);
    ATH_CHECK(measToTruthHandle.isValid());
    measToTruth = measToTruthHandle.cptr();
  }

  // Setup outputs
  // Create the xAOD container and its auxiliary store:
  SG::WriteHandle<xAOD::TrackMeasurementValidationContainer> xaod = SG::makeHandle(m_write_xaod_key,ctx);
  ATH_CHECK(xaod.record(std::make_unique<xAOD::TrackMeasurementValidationContainer>(),
                        std::make_unique<xAOD::TrackMeasurementValidationAuxContainer>()));

  // Decorations
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

  SG::WriteDecorHandle<xAOD::TrackMeasurementValidationContainer, std::vector<int>> decor_tots ( m_measurement_tots, ctx );
  
  // Create output collection
  xAOD::TrackMeasurementValidationContainer *measurements = xaod.ptr();
  std::vector<xAOD::TrackMeasurementValidation*> toAdd(clusters->size(), nullptr);
  for (std::size_t i(0); i<toAdd.size(); ++i) {
    toAdd[i] = new xAOD::TrackMeasurementValidation();
  }
  measurements->insert(measurements->end(), toAdd.begin(), toAdd.end());

  
  // loop over collection and convert to xAOD::TrackMeasurementValidation
  for (std::size_t i(0); i<clusters->size(); ++i) {
    const xAOD::PixelCluster* cluster = clusters->at(i);
    xAOD::TrackMeasurementValidation* measurement = measurements->at(i);
    
    xAOD::DetectorIdentType clusterId = cluster->identifier();
    xAOD::DetectorIDHashType hashId = cluster->identifierHash();
    
    const InDetDD::SiDetectorElement *element = pixelDetEle->getDetectorElement(hashId);
    if ( not element ) {
      ATH_MSG_FATAL( "Invalid pixel detector element for hash " << hashId );
      return StatusCode::FAILURE;
    }

    const std::vector<Identifier> rdoList = cluster->rdoList();
    std::vector< std::uint64_t > rdoIdentifierList;
    rdoIdentifierList.reserve(rdoList.size());
    int rowmin = std::numeric_limits<int>::max();
    int rowmax = std::numeric_limits<int>::min();
    int colmin = std::numeric_limits<int>::max();
    int colmax = std::numeric_limits<int>::min();
    for( const Identifier& hitIdentifier : rdoList ){
      rdoIdentifierList.push_back( hitIdentifier.get_compact() );
      //May want to addinformation about the individual hits here
      int row = m_PixelHelper->phi_index(hitIdentifier);
      int col = m_PixelHelper->eta_index(hitIdentifier);
      rowmin = std::min(rowmin, row);
      rowmax = std::max(rowmax, row);
      colmin = std::min(colmin, col);
      colmax = std::max(colmax, col);
    }

    // compute centroid
    const InDetDD::PixelModuleDesign* design = dynamic_cast<const InDetDD::PixelModuleDesign*>(&element->design());
    if (not design) {
      ATH_MSG_ERROR("Cannot retrieve detector design");
      return StatusCode::FAILURE;
    }
    InDetDD::SiLocalPosition pos1 = design->positionFromColumnRow(colmin,rowmin);
    InDetDD::SiLocalPosition pos2 = design->positionFromColumnRow(colmax,rowmin);
    InDetDD::SiLocalPosition pos3 = design->positionFromColumnRow(colmin,rowmax);
    InDetDD::SiLocalPosition pos4 = design->positionFromColumnRow(colmax,rowmax);
    InDetDD::SiLocalPosition centroid = 0.25 * (pos1 + pos2 + pos3 + pos4);
          
    //Set Identifier
    measurement->setIdentifier( clusterId );
    measurement->setRdoIdentifierList(std::move(rdoIdentifierList));
    
    //Set Global Position
    auto gpos = cluster->globalPosition();
    measurement->setGlobalPosition(gpos.x(), gpos.y(), gpos.z());

    // Set local position and error matrix
    auto locpos = cluster->localPosition<2>();
    measurement->setLocalPosition(locpos[0],  locpos[1]); 

    auto localCov = cluster->localCovariance<2>();
    measurement->setLocalPositionError( localCov(0,0), localCov(1,1), localCov(0,1) );

    const Identifier waferId = m_PixelHelper->wafer_id(hashId);
    decor_detectorElementID(*measurement) = hashId;
    decor_waferID(*measurement) = waferId.get_compact();
    decor_bec(*measurement) = m_PixelHelper->barrel_ec(waferId);
    decor_layer(*measurement) = m_PixelHelper->layer_disk(waferId);
    decor_sizePhi(*measurement) = cluster->channelsInPhi();
    decor_sizeZ(*measurement) = cluster->channelsInEta();
    decor_SiWidth(*measurement) = cluster->channelsInPhi();
    decor_eta_module(*measurement) = m_PixelHelper->eta_module(waferId);
    decor_phi_module(*measurement) = m_PixelHelper->phi_module(waferId);
    decor_omegax(*measurement) = cluster->omegaX();
    decor_omegay(*measurement) = cluster->omegaY();
    decor_LorentzShift(*measurement) = static_cast<float>( m_lorentzAngleTool->getLorentzShift(cluster->identifierHash(), ctx) );
    decor_centroid_xphi(*measurement) = centroid.xPhi();
    decor_centroid_xeta(*measurement) = centroid.xEta();
    decor_side(*measurement) = 0;
    decor_tots(*measurement) = cluster->totList();
  }

  
  // handle the truth
  if (m_useTruthInfo) {
    SG::WriteDecorHandle<xAOD::TrackMeasurementValidationContainer, std::vector<unsigned int>> decor_truth_indices( m_measurement_truth_indices, ctx );
    SG::WriteDecorHandle<xAOD::TrackMeasurementValidationContainer, std::vector<unsigned int>> decor_truth_barcode( m_measurement_truth_barcodes, ctx );
  
    for (std::size_t i(0); i<clusters->size(); ++i) {
      const xAOD::PixelCluster* cluster = clusters->at(i);
      xAOD::TrackMeasurementValidation* measurement = measurements->at(i);

      // Use the MultiTruth Collection 
      // to get a list of all true particle contributing to the cluster      
      if (cluster->index() >= measToTruth->size()) {
	ATH_MSG_ERROR( "PRD index "<< cluster->index() << " not present in the measurement to truth vector with size " << measToTruth->size() );
	return StatusCode::FAILURE;
      }

      auto tps = measToTruth->at(cluster->index());

      std::vector<unsigned int> tp_indices;
      std::vector<unsigned int> tp_barcodes;
      for (const auto& tp : tps) {
        tp_indices.push_back(tp->index());
        tp_barcodes.push_back(HepMC::barcode(tp));
      }
      
      // decorate
      decor_truth_indices(*measurement) = std::move(tp_indices);
      decor_truth_barcode(*measurement) = std::move(tp_barcodes);
      
    } // loop on clusters/measurements
  } // if do truth

  ATH_MSG_DEBUG( " recorded PixelPrepData objects: size " << measurements->size() );
  return StatusCode::SUCCESS;
}

}


