/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
//
//   @file    RoiReader.cxx         
//   
//
//   @author M.Sutton
// 
//


/// class header
#include "RoiReader.h"

/// xAOD storage class
#include "xAODTrigger/RoiDescriptorStore.h"
#include "xAODTrigger/RoiDescriptorStoreAuxInfo.h"

/// utility to serialise the RoiDescriptorCollection
#include "TrigRoiConversion/RoiSerialise.h"


RoiReader::RoiReader( const std::string& name  ) : asg::AsgTool( name ) { } 



void RoiReader::execute( std::vector<std::string>& keys ) {

  ATH_MSG_DEBUG( "RoiReader::execute() in" );

  /// get list of storage keys ...

  //  std::vector<std::string> keys;
  //  evtStore()->keys<xAOD::RoiDescriptorStore>( keys );

  /// loop over all the keys creating actual TrigRoiDescriptor 
  /// collections
  const std::string collectionname = "RoiDescriptorStore";
  for ( unsigned ik=0 ; ik<keys.size() ; ik++ ) { 

    
    std::string keybase = keys[ik];
    keybase.erase( 0, keybase.find(collectionname)+collectionname.size() );
    
    std::string newkey      = "HLT_TrigRoiDescriptorCollection" + keybase;

    auto roicollection = std::make_unique<TrigRoiDescriptorCollection>();

    deserialiser( *roicollection, keys[ik] );

    if ( evtStore()->overwrite( std::move(roicollection), newkey, false ).isFailure() ) {
      ATH_MSG_INFO( "RoiReader::execute() could not write "  << newkey );
    }

  }

  ATH_MSG_DEBUG( "RoiReader::execute() out " );

}




void RoiReader::deserialiser( TrigRoiDescriptorCollection& collection, const std::string& key ) const {
  xAOD::RoiDescriptorStore* fetchedstore = 0;
  if ( evtStore()->retrieve( fetchedstore, key ).isSuccess() && fetchedstore!=0 ) { 
    RoiUtil::deserialise( fetchedstore->serialised(), collection );
  }
}

