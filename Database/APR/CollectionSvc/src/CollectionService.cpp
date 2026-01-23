/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
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
pool::CollectionService::create( const pool::ICollectionDescription& description )
{
   if( description.name().empty() ) {
      std::string errorMsg = "Must specify name of collection in description input argument.";
      throw std::runtime_error( errorMsg + " (APR: \" CollectionService::create \" from \" CollectionSvc \")" );
   }

   if( description.type().empty() ) {
      std::string errorMsg = "Must specify type of collection in description input argument.";
      throw std::runtime_error( errorMsg + " (APR: \" CollectionService::create \" from \" CollectionSvc \")" );
   }

   if ( description.type() == "ImplicitCollection" )  {
      std::string errorMsg = 
         "Can only open a collection of type 'ImplicitCollection' for read transtions.";
      throw std::runtime_error( errorMsg + " (APR: \" CollectionService::create \" from \" CollectionSvc \")" );
   }

   pool::ICollection::OpenMode openMode = pool::ICollection::CREATE_AND_OVERWRITE;
   return plugin( description, openMode );
}


pool::ICollection* 
pool::CollectionService::handle( const std::string& name,
                                 const std::string& type,
                                 const std::string & connection,
                                 bool readOnly,
                                 pool::ISession* session ) const
{
   if( ( type == "ImplicitCollection ") && (! readOnly ) )  {
      std::string errorMsg = "Cannot open a collection of type 'ImplicitCollection' for updates.";
      throw std::runtime_error( errorMsg + " (APR: \" CollectionService::handle \" from \" CollectionSvc \")" );
   }
   pool::CollectionDescription description( name, type, connection );
   return plugin( description, ICollection::READ, session );
}


void
pool::CollectionService::setMessageSvcQuiet( bool quiet )
{
   Athena::getMessageSvcQuiet = quiet;
}

pool::ICollection*
pool::CollectionService::plugin( const ICollectionDescription& description,
                                 ICollection::OpenMode openMode,
                                 ISession* session ) const
{
   std::string type( description.type() );
   ICollection *coll = Gaudi::PluginService::Factory<ICollection*( const ICollectionDescription*, ICollection::OpenMode, ISession*)>::create( type, &description, openMode, session ).release();
   if( !coll ) {
      std::string errorMsg = "FAILED!  Plugin for " + type + "," + description.name() + " could not be loaded.";
      throw std::runtime_error( errorMsg + " (APR: \" CollectionService::plugin \" from \" CollectionSvc \")" );
   }
   return coll;
}
