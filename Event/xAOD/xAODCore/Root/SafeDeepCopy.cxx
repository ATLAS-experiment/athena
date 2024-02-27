/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// $Id: SafeDeepCopy.cxx 676241 2015-06-18 06:15:44Z krasznaa $

// EDM include(s):
//#define protected public
#   include "AthContainers/AuxElement.h"
//#undef protected
#include "AthContainers/exceptions.h"
#include "AthContainers/AuxTypeRegistry.h"

// Local include(s):
#include "xAODCore/tools/SafeDeepCopy.h"

namespace xAOD {

   void safeDeepCopy( const SG::AuxElement& orig, SG::AuxElement& copy ) {

      //
      // Check that the target object is healthy:
      //
      if( ! copy.container() ) {
         return;
      }
      if( ! copy.container()->hasStore() ) {
         return;
      }
      if( ! copy.container()->hasNonConstStore() ) {
         throw SG::ExcConstAuxData( "safeDeepCopy" );
      }

      //
      // Access the original's data:
      //
      const SG::AuxVectorData* ocont = orig.container();
      if( ( ! ocont ) || ( ! ocont->hasStore() ) ) {
         // In this case let the assignment operator of SG::AuxElement take
         // care of clearing out the target object.
         copy = orig;
         return;
      }

      //
      // Do the copy:
      //
      const size_t iindex = orig.index();
      const size_t oindex = copy.index();
      SG::auxid_set_t other_ids = ocont->getAuxIDs();
      SG::AuxTypeRegistry& r = SG::AuxTypeRegistry::instance();
      SG::AuxVectorData& copy_container = *copy.container();

      // Copy the variables that exist on the input object:
      for( SG::auxid_t auxid : other_ids ) {
         r.copy( auxid, copy_container, oindex, *ocont, iindex, 1 );
      }
      // Clear out the variables that only exist on the output object:
      for( SG::auxid_t auxid : copy.container()->getWritableAuxIDs() ) {
         if( !other_ids.test( auxid ) ) {
            r.clear( auxid, copy_container, oindex, 1 );
         }
      }
   }

} // namespace xAOD
