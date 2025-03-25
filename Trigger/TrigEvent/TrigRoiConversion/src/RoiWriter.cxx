/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
//
//   @file    RoiWriter.cxx
//
//   @author M.Sutton
//   @author F. Winklmeier
// 

// EDM include(s):
#include "xAODTrigger/RoiDescriptorStore.h"
#include "xAODTrigger/RoiDescriptorStoreAuxInfo.h"

// Athena include(s):
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"

// Local include(s):
#include "RoiWriter.h"
#include "TrigRoiConversion/RoiSerialise.h"

namespace {
  // Prefix of the AOD container:
  static const std::string prefix = "HLT_TrigRoiDescriptorCollection";
  static const std::string newPrefix = "HLT_xAOD__RoiDescriptorStore";
}

RoiWriter::RoiWriter( const std::string& name, ISvcLocator* pSvcLocator )
   : AthReentrantAlgorithm( name, pSvcLocator ) {
}

StatusCode RoiWriter::execute(const EventContext& ctx) const {

   ATH_MSG_DEBUG( "In execute()..." );

   // Get the keys of the AOD container(s):
   std::vector< std::string > keys;
   evtStore()->keys< TrigRoiDescriptorCollection >( keys );

   // Buffer for serialisation:
   std::vector< std::vector< uint32_t > > roiserial;

   bool just_dandy = true;

   // Create handle keys for use in the loop:
   SG::ReadHandleKey< TrigRoiDescriptorCollection > rhk("temp");
   SG::WriteHandleKey< xAOD::RoiDescriptorStore >   whk("temp");
   ATH_CHECK( rhk.initialize() && whk.initialize() );

   // Loop over these container(s):
   for( const std::string& key : keys ) {
      // Update key to read from:
      rhk = key;

      // Construct the key of the new container:
      whk = ( ( key.find( prefix ) == 0 ) ?
              ( newPrefix + key.substr( prefix.size() ) ) :
              key );

      auto rh = SG::makeHandle(rhk, ctx);
      auto wh = SG::makeHandle(whk, ctx);

      /// check object actually points to something - make sure that we do all
      /// collections that we can, and not barf on the first one that might fail  
      if ( rh.isValid() ) {

         // Create the "new payload" from it:
         roiserial.clear();
         RoiUtil::serialise( *rh, roiserial );

         // Create the xAOD objects:
         auto store = std::make_unique< xAOD::RoiDescriptorStore >();
         auto aux   = std::make_unique< xAOD::RoiDescriptorStoreAuxInfo >();
         store->setStore( aux.get() );
         store->setSerialised( roiserial );

         // Record the new container:
         ATH_CHECK( wh.record( std::move( store ), std::move( aux ) ) );
      }
      else just_dandy = false;
   }

   // Return gracefully:
   return just_dandy ? StatusCode::SUCCESS : StatusCode::FAILURE;
}

