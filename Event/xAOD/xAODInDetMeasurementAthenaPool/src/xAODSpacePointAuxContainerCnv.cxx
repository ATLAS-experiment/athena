/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// Local include(s):
#include "xAODSpacePointAuxContainerCnv.h"

// EDM include(s):
#include "xAODInDetMeasurement/SpacePointContainer.h"
#include "xAODMeasurementBase/UncalibratedMeasurementContainer.h"

// Other includes
#include "AthLinks/ElementLink.h"

xAODSpacePointAuxContainerCnv::xAODSpacePointAuxContainerCnv( ISvcLocator* svcLoc )
  : xAODSpacePointAuxContainerCnvBase( svcLoc )
{}

StatusCode xAODSpacePointAuxContainerCnv::initialize()
{
  ATH_MSG_DEBUG("Initializing xAODSpacePointAuxContainerCnv ...");
  return xAODSpacePointAuxContainerCnvBase::initialize();
}

xAOD::SpacePointAuxContainer* xAODSpacePointAuxContainerCnv::createPersistentWithKey( xAOD::SpacePointAuxContainer* trans,
										      const std::string& key )
{
  ATH_MSG_DEBUG("Calling xAODSpacePointAuxContainerCnv::createPersistentWithKey for our xAOD space points");
  
  // Load the necessary ROOT class(es):
  static char const* const NAME =
    "std::vector<std::vector<ElementLink<xAOD::UncalibratedMeasurementContainer> > >";
  static TClass const* const cls = TClass::GetClass( NAME );
  if( ! cls ) {
    ATH_MSG_ERROR( "Couldn't load dictionary for type: " << NAME );
  }
  
  // This makes a copy of the container, with any thinning applied.
  std::unique_ptr< xAOD::SpacePointAuxContainer > result 
    ( xAODSpacePointAuxContainerCnvBase::createPersistentWithKey (trans, key) );
  
  // see if we can get the variable from trans
  using measurement_collection_t = std::vector< const xAOD::UncalibratedMeasurement* >;
  static const SG::ConstAccessor< measurement_collection_t > uncalibMeasurementAcc("measurements");
  static const SG::auxid_t measurementAuxId = uncalibMeasurementAcc.auxid();

  // Create a helper object for the Element Links
  xAOD::SpacePointContainer helper;
  for (std::size_t i(0); i<result->size(); ++i) {
    helper.push_back( std::make_unique< xAOD::SpacePoint >() );
  }
  helper.setStore( result.get() );

  // Convert the bare pointer(s) to Element Link(s)
  static const SG::AuxElement::Accessor< std::vector<ElementLink<xAOD::UncalibratedMeasurementContainer>> > accesor("measurementLink"); 

  for (xAOD::SpacePoint *sp : helper) {
    accesor(*sp).clear();

    // get values from the trans aux container directly
    const void* ptrToSomething = trans->getData (measurementAuxId);
    const measurement_collection_t& measurements = static_cast<const measurement_collection_t*>(ptrToSomething)[sp->index()];
    accesor(*sp).reserve( measurements.size() );
    for (std::size_t idx(0); idx<measurements.size(); ++idx) {
      accesor(*sp).push_back( ElementLink< xAOD::UncalibratedMeasurementContainer >(*dynamic_cast<const xAOD::UncalibratedMeasurementContainer*>(measurements[idx]->container()), measurements[idx]->index()) );
    }    
  }
  
  return result.release();
}
