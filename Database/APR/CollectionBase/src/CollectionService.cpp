/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "CollectionBase/CollectionService.h"
#include "CollectionBase/CollectionFactory.h"
#include "CollectionBase/CollectionDescription.h"
#include "CollectionBase/ICollectionCursor.h"

#include "FileCatalog/IFileCatalog.h"

#include <exception>
#include <cstring>

using namespace std;
using namespace pool;
namespace pool { class ISession; }


static const std::string	moduleName("CollectionService");


pool::CollectionService::CollectionService( ) 
{
}


pool::CollectionService::~CollectionService()
{
}


pool::ICollection*
pool::CollectionService::create( const pool::ICollectionDescription& description, bool overwrite )
{
   if( description.name().empty() ) {
      std::string errorMsg = "Must specify name of collection in description input argument.";
      throw std::runtime_error( errorMsg + " (APR: \" CollectionService::create \" from \" CollectionService \")" );
   }

   if( description.type().empty() ) {
      std::string errorMsg = "Must specify type of collection in description input argument.";
      throw std::runtime_error( errorMsg + " (APR: \" CollectionService::create \" from \" CollectionService \")" );
   }

   if ( description.type() == "ImplicitCollection" )  {
      std::string errorMsg = 
         "Can only open a collection of type 'ImplicitCollection' for read transtions.";
      throw std::runtime_error( errorMsg + " (APR: \" CollectionService::create \" from \" CollectionService \")" );
   }

   pool::ICollection::OpenMode openMode = overwrite? pool::ICollection::CREATE_AND_OVERWRITE : pool::ICollection::CREATE;
   return  pool::CollectionFactory::get()->create( description, openMode );
}


pool::ICollection*
pool::CollectionService::createAndRegister( const pool::ICollectionDescription& description,
                                            bool overwrite,
                                            const std::string & logicalName )
{
   if( description.name().empty() )  {
      std::string errorMsg = "Must specify name of collection in description input argument.";
      throw std::runtime_error( errorMsg + " (APR: \" CollectionService::createAndRegister \" from \" CollectionService \")" );
   }

   if( description.type().empty() )  {
      std::string errorMsg = "Must specify type of collection in description input argument.";
      throw std::runtime_error( errorMsg + " (APR: \" CollectionService::createAndRegister \" from \" CollectionService \")" );
   }

   if ( description.type() == "ImplicitCollection" )  {
      std::string errorMsg = "Cannot register a collection of type 'ImplicitCollection' in a collection catalog.";
      throw std::runtime_error( errorMsg + " (APR: \" CollectionService::createAndRegister \" from \" CollectionService \")" );
   }

   return CollectionFactory::get()->createAndRegister( description,
						       0,
						       overwrite,
						       logicalName );
}


bool
pool::CollectionService::registerExisting( const std::string & name,
                                           const std::string & type,
                                           const std::string & connection,
                                           const std::string & logicalName )
{
   if( name.empty() )  {
    std::string errorMsg = "Must specify name of collection as input.";
    throw std::runtime_error( errorMsg + " (APR: \" CollectionService::registerExisting \" from \" CollectionService \")" );
  }

   if( type.empty() ) {
    std::string errorMsg = "Must specify type of collection as input.";
    throw std::runtime_error( errorMsg + " (APR: \" CollectionService::registerExisting \" from \" CollectionService \")" );
  }

  if( type == "ImplicitCollection" )  {
    std::string errorMsg = "Cannot register a collection of type 'ImplicitCollection' in a collection catalog.";
    throw std::runtime_error( errorMsg + " (APR: \" CollectionService::registerExisting \" from \" CollectionService \")" );
  }

  CollectionDescription description( name, type, connection );

  return CollectionFactory::get()->registerExisting( description,
						     0,
						     logicalName );
}



bool
pool::CollectionService::registerExisting( ICollection* collection,
					   bool overwrite,
                                           const std::string & logicalName )
{
  return CollectionFactory::get()->registerExisting( collection,
						     overwrite,
						     0,
						     logicalName );
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
      throw std::runtime_error( errorMsg + " (APR: \" CollectionService::handle \" from \" CollectionService \")" );
   }
   pool::CollectionDescription description( name, type, connection );
   pool::ICollection::OpenMode openMode = readOnly? ICollection::READ : ICollection::UPDATE;
   return CollectionFactory::get()->create( description, openMode, session );
}


pool::ICollection* 
pool::CollectionService::openWithPhysicalName( const std::string& physicalName, ICollection::OpenMode openMode )
{
  return pool::CollectionFactory::get()->openWithPhysicalName( physicalName, 0, openMode );
}


pool::ICollection* 
pool::CollectionService::openWithLogicalName( const std::string& logicalName, bool readOnly )
{
  return pool::CollectionFactory::get()->openWithLogicalName( logicalName, 0, readOnly );
}


pool::ICollection* 
pool::CollectionService::openWithGuid( const std::string& guid, bool readOnly )
{
   return pool::CollectionFactory::get()->openWithGuid( guid, 0, readOnly );
}


bool
pool::CollectionService::setDefaultCatalog()
{
   CollectionFactory::get()->setDefaultCatalog(0);  // reset so get() gets the default
   return (CollectionFactory::get()->getDefaultCatalog() != 0);  // reset so get() gets the default
}



bool 
pool::CollectionService::setWriteCatalog( const std::string& name )
{
   CollectionFactory::get()->setWriteCatalog( name ); 
   return true;
}


bool
pool::CollectionService::addReadCatalog( const std::string& name )
{
   IFileCatalog *fc = CollectionFactory::get()->getDefaultCatalog();
   fc->disconnect();
   fc->addReadCatalog( name );
   fc->connect();
   return true;
}


pool::IFileCatalog*
pool::CollectionService::getCatalog( )
{
   return CollectionFactory::get()->getDefaultCatalog();
}


void
pool::CollectionService::setCatalog( IFileCatalog* collCat )
{
   CollectionFactory::get()->setDefaultCatalog( collCat );   
}

