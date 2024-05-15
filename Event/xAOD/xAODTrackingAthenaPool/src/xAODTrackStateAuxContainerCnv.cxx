/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Local include(s):
#include "xAODTrackStateAuxContainerCnv.h"

// EDM include(s):
#include "xAODTracking/TrackStateContainer.h"
#include "xAODTracking/TrackState.h"
#include "xAODMeasurementBase/UncalibratedMeasurementContainer.h"

// Other includes
#include "AthLinks/ElementLink.h"

xAODTrackStateAuxContainerCnv::xAODTrackStateAuxContainerCnv( ISvcLocator* svcLoc )
  : xAODTrackStateAuxContainerCnvBase( svcLoc )
{}

StatusCode xAODTrackStateAuxContainerCnv::initialize()
{
   ATH_MSG_DEBUG("Initializing xAODTrackStateAuxContainerCnv ...");
   return xAODTrackStateAuxContainerCnvBase::initialize();
}

xAOD::TrackStateAuxContainer* xAODTrackStateAuxContainerCnv::createPersistentWithKey( xAOD::TrackStateAuxContainer* trans,
                                                                                      const std::string& key )
{
   ATH_MSG_DEBUG("Calling xAODTrackStateAuxContainerCnv::createPersistentWithKey for our xAOD space points");

   // Load the necessary ROOT class(es):
   static char const* const NAME =
      "std::vector<ElementLink<xAOD::UncalibratedMeasurementContainer> >";
   static TClass const* const cls = TClass::GetClass( NAME );
   if( ! cls ) {
      ATH_MSG_ERROR( "Couldn't load dictionary for type: " << NAME );
   }

   // This makes a copy of the container, with any thinning applied.
   std::unique_ptr< xAOD::TrackStateAuxContainer > result
      ( xAODTrackStateAuxContainerCnvBase::createPersistentWithKey (trans, key) );

   // see if we can get the variable from trans
   using uncalib_measurement_ptr_t = const xAOD::UncalibratedMeasurement*;
   static const SG::auxid_t uncalibMeasurementAuxId = SG::ConstAccessor<uncalib_measurement_ptr_t>("uncalibratedMeasurement").auxid();

   // Create a helper object for the Element Links
   xAOD::TrackStateContainer helper;
   for (std::size_t i(0); i<result->size(); ++i) {
      helper.push_back( std::make_unique< xAOD::TrackState >() );
   }
   helper.setStore( result.get() );

   // Convert the bare pointer(s) to Element Link(s)
   static const SG::AuxElement::Accessor< ElementLink<xAOD::UncalibratedMeasurementContainer> > accesor("uncalibratedMeasurementLink");
   // result is a newly created container which we owne here
   // so we can manipulate the data
   void* ptrToSomething ATLAS_THREAD_SAFE = const_cast<void *>(result->getData (uncalibMeasurementAuxId));
   assert (trans->getData (uncalibMeasurementAuxId) != ptrToSomething);
   if (trans->getData (uncalibMeasurementAuxId) == ptrToSomething) {
      throw std::runtime_error("Modifiying source data.");
   }
   for (xAOD::TrackState *state : helper) {
      const xAOD::UncalibratedMeasurement *measurement = static_cast<const uncalib_measurement_ptr_t *>(ptrToSomething)[state->index()];
      if (measurement) {
         // set the raw pointer to zero for persistification otherwise if the data is written the tree check will seg-fault
         static_cast<uncalib_measurement_ptr_t *>(ptrToSomething)[state->index()]=nullptr;
         accesor(*state)
            = ElementLink< xAOD::UncalibratedMeasurementContainer >(*dynamic_cast<const xAOD::UncalibratedMeasurementContainer*>(measurement->container()),
                                                                    measurement->index());
      }
      else {
         accesor(*state)
            = ElementLink< xAOD::UncalibratedMeasurementContainer >();
      }
   }

   return result.release();
}
