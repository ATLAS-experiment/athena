#include "PixelClusterTruthDecorator.h"

//output
#include "xAODTracking/TrackMeasurementValidation.h"
#include "xAODTracking/TrackMeasurementValidationContainer.h"
#include "xAODTracking/TrackMeasurementValidationAuxContainer.h"

#define AUXDATA(OBJ, TYP, NAME) \
  static const SG::AuxElement::Accessor<TYP> acc_##NAME (#NAME);  acc_##NAME(*(OBJ))

namespace ActsTrk {

  PixelClusterTruthDecorator::PixelClusterTruthDecorator(const std::string& name, ISvcLocator *pSvcLocator) :
    AthReentrantAlgorithm(name, pSvcLocator)
  {}

  StatusCode PixelClusterTruthDecorator::initialize() {
    
    // Read keys
    ATH_CHECK(m_clustercontainer_key.initialize());
    ATH_CHECK(m_associationMap_key.initialize());
    
    // Write keys
    ATH_CHECK(m_write_xaod_key.initialize());
    //ATH_CHECK(m_write_offsets.initialize()); 

    return StatusCode::SUCCESS;
  }
  
StatusCode PixelClusterTruthDecorator::execute(const EventContext& ctx) const {

  //Mandatory. Require if the algorithm is scheduled.
  SG::ReadHandle<xAOD::PixelClusterContainer> PixelClusterContainer(m_clustercontainer_key,ctx);

  // Get the truth association map
  const MeasurementToTruthParticleAssociation* measToTruth(nullptr);
  if (m_useTruthInfo) {
    SG::ReadHandle<MeasurementToTruthParticleAssociation> measToTruthHandle(m_associationMap_key,ctx);
    
    if (measToTruthHandle.isValid()) {
      measToTruth = &*measToTruthHandle;
    } else {
      ATH_MSG_ERROR("MeasurementToTruthParticlesAssociation "<<m_associationMap_key.key()<<" not found!");
    }
  }

  // Setup outputs
  // Create the xAOD container and its auxiliary store:
  SG::WriteHandle<xAOD::TrackMeasurementValidationContainer> xaod(m_write_xaod_key,ctx);
  ATH_CHECK(xaod.record(std::make_unique<xAOD::TrackMeasurementValidationContainer>(),
                        std::make_unique<xAOD::TrackMeasurementValidationAuxContainer>()));
  

  // loop over collection and convert to xAOD::TrackMeasurementValidation
  for( const auto* prd : *PixelClusterContainer ){

    //This needs to be taken care of
    Identifier clusterId = Identifier((int)prd->identifier());
    if ( !clusterId.is_valid() ) {
      ATH_MSG_WARNING("Pixel cluster identifier is not valid");
    }

    xAOD::TrackMeasurementValidation* xprd = new xAOD::TrackMeasurementValidation();
    //unsigned int cluster_idx = xaod->size();
    xaod->push_back(xprd);
    //Set Identifier
    xprd->setIdentifier( clusterId.get_compact() );

    //Set Global Position
    auto gpos = prd->globalPosition();
    xprd->setGlobalPosition(gpos.x(),gpos.y(),gpos.z());

    auto locpos = prd->localPosition<2>();
    // Set local error matrix
    xprd->setLocalPosition(locpos[0],  locpos[1]); 

    auto localCov = prd->localCovariance<2>();
    if(localCov.size() == 1){
      xprd->setLocalPositionError( localCov(0,0), 0., 0. ); 
    } else if(localCov.size() == 4){
      xprd->setLocalPositionError( localCov(0,0), localCov(1,1), localCov(0,1) );     
    } else {
      xprd->setLocalPositionError(0.,0.,0.);
    }
    
    // Use the MultiTruth Collection 
    // to get a list of all true particle contributing to the cluster
    if (measToTruth) {
      
      if (prd->index() >= (*measToTruth).size()) {
	ATH_MSG_ERROR("PRD index "<< prd->index()<<" not present in the measurement to truth vector with size "<<(*measToTruth).size());
	continue;
      }
      auto tps = (*measToTruth)[prd->index()];
      
      
      std::vector<unsigned int> tp_indices;
      std::vector<unsigned int> tp_barcodes;
      for (auto tp : tps) {
	tp_indices.push_back(tp->index());
	tp_barcodes.push_back(tp->barcode());
      }

      //TODO change how to decorate
      AUXDATA(xprd,std::vector<unsigned int>, truth_index) = tp_indices;
      AUXDATA(xprd,std::vector<unsigned int>, truth_barcode) = tp_barcodes;
      
      
    }// association map exists
  
  }// close loop on clusters

  ATH_MSG_DEBUG( " recorded PixelPrepData objects: size " << xaod->size() );

  return StatusCode::SUCCESS;
}

  StatusCode PixelClusterTruthDecorator::finalize(){
    
    return StatusCode::SUCCESS;
  }
}


