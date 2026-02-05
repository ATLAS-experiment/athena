/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "CollectionSvc/CollectionService.h"
#include "CollectionSvc/CollectionDescription.h"

#include "Gaudi/PluginService.h"

#include "AthenaKernel/getMessageSvc.h"

#include <stdexcept>

using namespace std;
using namespace pool;
namespace pool { class ISession; }

pool::ICollection*
pool::CollectionService::create( const pool::CollectionDescription& description )
{
   if( description.name().empty() ) {
      std::string errorMsg = "Must specify name of collection in description input argument.";
      throw std::runtime_error( errorMsg + " (APR: \" CollectionService::create \" from \" CollectionSvc \")" );
   }
   pool::ICollection::OpenMode openMode = pool::ICollection::CREATE_AND_OVERWRITE;
   return plugin( description, openMode );
}


pool::ICollection* 
pool::CollectionService::open( const std::string& name,
                               const DbType& type,
                               const std::string& connection,
                               pool::ISession* session ) const
{
   pool::CollectionDescription description( name, type, connection );
   return plugin( description, ICollection::READ, session );
}


void
pool::CollectionService::setMessageSvcQuiet( bool quiet )
{
   Athena::getMessageSvcQuiet = quiet;
}

pool::ICollection*
pool::CollectionService::plugin( const CollectionDescription& description,
                                 ICollection::OpenMode openMode,
                                 ISession* session ) const
{
   pool::DbType type( description.type() );
   std::string typeString = "ImplicitCollection";
   if (type.majorType() == pool::ROOT_StorageType.type()) {
      typeString = "RootCollection";
   }
   ICollection *coll = Gaudi::PluginService::Factory<ICollection*( const CollectionDescription*, ICollection::OpenMode, ISession*)>::create( typeString, &description, openMode, session ).release();
   if( !coll ) {
      std::string errorMsg = "FAILED!  Plugin for " + typeString + "," + description.name() + " could not be loaded.";
      throw std::runtime_error( errorMsg + " (APR: \" CollectionService::plugin \" from \" CollectionSvc \")" );
   }
   return coll;
}
