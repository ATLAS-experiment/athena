/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "CollectionSvc/CollectionService.h"
#include "CollectionSvc/CollectionDescription.h"
#include "ImplicitCollection.h"
#include "RootCollection.h"

#include "AthenaKernel/getMessageSvc.h"

#include <stdexcept>

using namespace pool;


std::unique_ptr<pool::ICollection>
pool::CollectionService::create( const CollectionDescription& description )
{
   if( description.name().empty() ) {
      std::string errorMsg = "Must specify name of collection in description input argument.";
      throw std::runtime_error( errorMsg + " (APR: \" CollectionService::create \" from \" CollectionSvc \")" );
   }
   ICollection::OpenMode openMode = ICollection::CREATE_AND_OVERWRITE;
   return plugin( description, openMode );
}


std::unique_ptr<pool::ICollection> 
pool::CollectionService::open( const std::string& name,
                               const DbType& type,
                               const std::string& connection,
                               pool::ISession* session )
{
   pool::CollectionDescription description( name, type, connection );
   return plugin( description, ICollection::READ, session );
}


void
pool::CollectionService::setMessageSvcQuiet( bool quiet )
{
   Athena::getMessageSvcQuiet = quiet;
}


std::unique_ptr<pool::ICollection>
pool::CollectionService::plugin( const CollectionDescription& description,
                                 ICollection::OpenMode openMode,
                                 ISession* session )
{
   if( description.type().majorType() == pool::ROOT_StorageType.type() ) {
      return std::make_unique<RootCollection>( &description, openMode );
   } else {
      return std::make_unique<ImplicitCollection>( &description, openMode, session );
   }
}
