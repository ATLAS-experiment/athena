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

namespace {

   template <typename T>
   using const_span = std::span<T const>;

   template <typename T, typename T_AuxContainer>
   const_span<T> getElementVector(const T_AuxContainer &aux_container, const SG::ConstAccessor<T> &accessor) {
      const T *data = static_cast<const T *>(aux_container.getData (accessor.auxid()));
      const_span<T> ret( data, aux_container.size() );
      return ret;
   }
   template <typename T, typename T_AuxContainer>
   std::span<T> createDecoration(T_AuxContainer &aux_container, const SG::Decorator<T> &decor) {
      std::size_t sz=aux_container.size();
      T *data = static_cast<T *>(aux_container.getDecoration(decor.auxid(), sz, sz));
      return std::span<T>( data, sz );
   }
}

xAOD::TrackStateAuxContainer* xAODTrackStateAuxContainerCnv::createPersistentWithKey( xAOD::TrackStateAuxContainer* trans,
                                                                                      const std::string& key )
{
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
   static const SG::ConstAccessor<const xAOD::UncalibratedMeasurement*> uncalibMeasurement_acc("uncalibratedMeasurement");
   if (trans->getAuxIDs().test(uncalibMeasurement_acc.auxid())){

      const_span<const xAOD::UncalibratedMeasurement*>
         uncalibratedMeasurements = getElementVector(*trans, uncalibMeasurement_acc);

      static const SG::Decorator< ElementLink<xAOD::UncalibratedMeasurementContainer> > link_decor("uncalibratedMeasurementLink");
      std::span<decltype(link_decor)::element_type> links = createDecoration(*result, link_decor);
      // @TODD thinning is not supported for Acts tracks but this would break if thinning was somehow implemented
      assert( links.size() == uncalibratedMeasurements.size() );
      // Convert the bare pointer(s) to Element Link(s)
      for (unsigned int index =0; index < uncalibratedMeasurements.size(); ++index) {
         const xAOD::UncalibratedMeasurement *measurement = uncalibratedMeasurements[index];
         if (measurement) {
            links[index]
                = ElementLink< xAOD::UncalibratedMeasurementContainer >(*dynamic_cast<const xAOD::UncalibratedMeasurementContainer*>(measurement->container()),
                                                                        measurement->index());
         }
      }
   }
   else {
      ATH_MSG_WARNING("No uncalibratedMeasurement aux data.");
   }

   return result.release();
}
