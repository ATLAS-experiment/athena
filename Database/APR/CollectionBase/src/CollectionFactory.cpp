/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#include "CollectionBase/CollectionFactory.h"
#include "CollectionBase/CollectionDescription.h"
#include "CollectionBase/ICollectionCursor.h"
#include "CollectionBase/CollectionBaseNames.h"
#include "CollectionBase/boost_tokenizer_headers.h"

#include "FileCatalog/IFileCatalog.h"

#include "Gaudi/PluginService.h"

#include "CoralBase/MessageStream.h"

#include "PersistentDataModel/Guid.h"
#include <cstring>


using namespace std;
using namespace pool;

pool::CollectionFactory pool::CollectionFactory::s_instance;
static const string	thisModule( "POOLCollFactory" );

const std::string pool::CollectionFactory::c_fileType = "PoolCollection";


pool::CollectionFactory::CollectionFactory()
      : m_myOwnCatalog( false ),
        m_catalog( 0 ) 
{ }


pool::CollectionFactory::~CollectionFactory()
{
   if( m_myOwnCatalog ) delete m_catalog; 
}


pool::CollectionFactory* pool::CollectionFactory::get()
{
  return &s_instance;
}



pool::ICollection*
pool::CollectionFactory::create_callPlugin( const pool::ICollectionDescription& description,
					    pool::ICollection::OpenMode openMode,
					    pool::ISession* session ) const
{
   std::string type( description.type() );
   ICollection *coll = Gaudi::PluginService::Factory<ICollection*( const ICollectionDescription*, ICollection::OpenMode, ISession*)>::create( type, &description, openMode, session ).release();
   if( !coll ) {
      std::string errorMsg = "APR::CollectionFactory::create(" + type + "," + description.name() + ") FAILED!  Plugin for that collection technology could not be loaded.";
      if( type == "MemoryCollection" ) {
         errorMsg = "Collection type <MemoryCollection> is not supported";
      }
      throw std::runtime_error( errorMsg + " (APR: \" CollectionFactory::create \" from \" CollectionBase \")" );
   }
   return coll;
}
   


// resolve PHYSICAL_NAME, LOGICAL_NAME and GUID notation (using catalog)
void
CollectionFactory::resolveDescription( CollectionDescription& description, bool readOnly ) const
{
   string type = description.type();
   string name = description.name();
   if( type == "LOGICAL_NAME" ) {
      description = descFromLogicalName( name, 0, readOnly);
   } else if( type == "GUID" ) {
      description = descFromGuid( name, 0, readOnly );
   } else if( type == "PHYSICAL_NAME" ) {
      CollectionDescription	tmpdesc( descFromPhysicalName( name, 0, readOnly ) );
      // the original description content is needed for CREATE 
      description.setName( tmpdesc.name() );
      description.setType( tmpdesc.type() ); 
      description.setConnection( tmpdesc.connection() ); 
   }
}

         

pool::ICollection*
pool::CollectionFactory::create( const ICollectionDescription& _description,
                                 ICollection::OpenMode openMode,
                                 ISession* session ) const
{
   // local copy of the description to allow modifications
   CollectionDescription	description( _description );
   // resolve PHYSICAL_NAME, LOGICAL_NAME and GUID notation (using catalog)
   resolveDescription( description, openMode==ICollection::READ );

   ICollection *coll = create_callPlugin( description, openMode, session );

   if( coll && (openMode == ICollection::CREATE ||
                openMode == ICollection::CREATE_AND_OVERWRITE ) ) {      
      // generate and add collection ID
      coral::MessageStream log( thisModule ); 
      Guid	coll_id;
      Guid::create( coll_id );
      string	guid_str( coll_id.toString() );
      log << coral::Debug << "Generated new ID for collection " <<  description.name()
	  << " GUID=" << guid_str << endl
	  << coral::MessageStream::endmsg;
      coll->commit( true );
   }
   return coll;
}





pool::ICollection*
pool::CollectionFactory::createAndRegister( const pool::ICollectionDescription& _description,
                                            pool::IFileCatalog* collectionCatalog,
                                            bool overwrite,
                                            std::string logicalName,
                                            pool::ISession* session ) const
{
  if( !_description.hasEventReferenceColumn() )  {
    std::string errorMsg = "Cannot register a collection fragment in a collection catalog. Must add an event reference columnto collection fragment `" + _description.name() + "' to make it a collection.";
    throw std::runtime_error( errorMsg + " (APR: \" CollectionFactory::createAndRegister \" from \" CollectionBase \")" );
  }

  std::string physicalName;
  CollectionDescription	description( _description );
  if( description.type() == "PHYSICAL_NAME" ) {
     physicalName = description.name(); // should be already a type|connection|name
     std::string type, connection, name;     
     extract( physicalName, type, connection, name );
     description.setName(name);
     description.setType(type); 
     description.setConnection(connection); 
  } else {
     physicalName = description.type() + "|" + description.connection() + "|" + description.name();
  }
  
  if( !collectionCatalog) collectionCatalog = getDefaultCatalog();
  std::string fileType, guid;
  collectionCatalog->start();
  collectionCatalog->lookupFileByPFN( physicalName, guid, fileType );
  collectionCatalog->commit();
  
  if( guid.length() )  {
     if( overwrite )    {
	if( !isUnique( guid, *collectionCatalog ) ) {
	   std::string errorMsg = "Cannot overwrite collection `" + description.name() + "' because it has been replicated.";
           throw std::runtime_error( errorMsg + " (APR: \" CollectionFactory::createAndRegister \" from \" CollectionBase \")" );
	}
     } 
     else {
	std::string errorMsg = "User did not authorize overwrite of registered collection `" + description.name() + "'.";
        throw std::runtime_error( errorMsg + " (APR: \" CollectionFactory::createAndRegister \" from \" CollectionBase \")" );
     }
  }  

  ICollection::OpenMode openMode = overwrite ? ICollection::CREATE_AND_OVERWRITE : ICollection::CREATE;

  pool::ICollection* collection = create_callPlugin( description, openMode, session );
  
  collectionCatalog->start();
  if( !guid.length() ) {
     collectionCatalog->registerPFN( physicalName, c_fileType, guid );
  }
  if( logicalName.length() ) {
     collectionCatalog->registerLFN( guid, logicalName );
  }
  // add collection ID
  collection->commit( true ); 
  collectionCatalog->commit();

  return collection;
}


bool 
pool::CollectionFactory::registerExisting( const pool::ICollectionDescription& description,
                                           pool::IFileCatalog* collectionCatalog,
                                           std::string logicalName,
                                           pool::ISession* session ) const
{
   if( !description.hasEventReferenceColumn() )  {
      std::string errorMsg = "Cannot register a collection fragment in a collection catalog.";
      throw std::runtime_error( errorMsg + " (APR: \" CollectionFactory::registerExisting \" from \" CollectionBase \")" );
   }

   if( !collectionCatalog) collectionCatalog = getDefaultCatalog();
   std::string physicalName = 
      description.type() + "|" + description.connection() + "|" + description.name();

   coral::MessageStream log( thisModule ); 
   log << coral::Debug << "Registering collection PFN=" << physicalName
       << ", LFN=" << logicalName  << coral::MessageStream::endmsg;

   pool::ICollection* collection = openWithPhysicalName( physicalName,
                                                         collectionCatalog,
                                                         pool::ICollection::READ,
                                                         session );

   bool overwrite( true );
   bool rc = registerExisting( collection, overwrite, collectionCatalog, logicalName, session );
   delete collection;   collection = 0;
   return rc;
}


bool 
pool::CollectionFactory::registerExisting( pool::ICollection* collection,
					   bool overwrite,
                                           pool::IFileCatalog* collectionCatalog,
                                           std::string logicalName,
                                           pool::ISession* ) const
{
   if( !collectionCatalog) collectionCatalog = getDefaultCatalog();
   std::string physicalName = collection->description().type() + "|" + collection->description().connection() + "|" + collection->description().name();
  
   coral::MessageStream log( thisModule ); 
   log << coral::Debug << "Registering existing collection PFN=" << physicalName
       << ", LFN=" << logicalName  << coral::MessageStream::endmsg;

   collectionCatalog->start();
   std::string fileType, guid;
   collectionCatalog->lookupFileByPFN( physicalName, guid, fileType );
   collectionCatalog->commit();

   if( !collection->isOpen() )
      collection->open();

   log << coral::Debug << " ---  found catalog guid=" << guid << coral::MessageStream::endmsg;
   
   if( guid.length() )  {

      if( !overwrite ) {
	 std::string errorMsg = "Collection with physical name `" + physicalName + "' is already registered in collection catalog.";
         throw std::runtime_error( errorMsg + " (APR: \" CollectionFactory::registerExisting \" from \" CollectionBase \")" );
      }
      // attempt to remove the existing GUID from the catalog (no GUID reuse)
      log << coral::Info  << " -- removing " << guid << " from the catalog " <<  coral::MessageStream::endmsg;
      collectionCatalog->start();
      collectionCatalog->deleteFID( guid );
      collectionCatalog->commit();
   }

   collectionCatalog->start();
   collectionCatalog->registerPFN( physicalName, c_fileType, guid );
   if( logicalName.length() )  {
      collectionCatalog->registerLFN( guid, logicalName );
   }
   collectionCatalog->commit();
   if( collection->openMode() != ICollection::READ ) try {
      // try to update the collection ID with the one generated by the catalog
      collection->commit();
   } catch( std::runtime_error& e) {
      log << coral::Warning << "Failed to update collection ID for collection " << collection->description().name() <<  ".  The error was: " << e.what() <<  coral::MessageStream::endmsg;
      // return false; // FIXME: This is dangerous
   }
   
  return true;
}


// --   Open a collection using catalog ---

// PHYSICAL_NAME does not require a catalog entry because all info is provided

pool::ICollection* 
pool::CollectionFactory::openWithPhysicalName( const std::string& physicalName,
                                               pool::IFileCatalog* collectionCatalog,
                                               pool::ICollection::OpenMode openMode,
                                               pool::ISession* session ) const
{
   CollectionDescription description( descFromPhysicalName( physicalName, collectionCatalog, openMode==ICollection::READ) );
   return create_callPlugin( description, openMode, session );
}


pool::ICollection* 
pool::CollectionFactory::openWithLogicalName( const std::string& logicalName,
                                              pool::IFileCatalog* collectionCatalog,
                                              bool readOnly,
                                              pool::ISession* session ) const
{
   CollectionDescription description( descFromLogicalName( logicalName, collectionCatalog, readOnly ) );
   ICollection::OpenMode openMode = readOnly? ICollection::READ : ICollection::UPDATE;
   return create_callPlugin( description, openMode, session );
}



pool::ICollection* 
pool::CollectionFactory::openWithGuid( const pool::FileCatalog::FileID& guid,
                                       pool::IFileCatalog* collectionCatalog,
                                       bool readOnly,
                                       pool::ISession* session ) const
{
  CollectionDescription description( descFromGuid(guid, collectionCatalog, readOnly) ); 
  ICollection::OpenMode openMode = readOnly? ICollection::READ : ICollection::UPDATE;
  return create_callPlugin( description, openMode, session );
}



CollectionDescription
CollectionFactory::descFromPhysicalName( const std::string& physicalName,
                                         pool::IFileCatalog* collectionCatalog,
                                         bool readOnly ) const
{
   if( !collectionCatalog ) collectionCatalog = getDefaultCatalog();
   string       fileType, guid;
   collectionCatalog->start();
   collectionCatalog->lookupFileByPFN( physicalName, guid, fileType );
   collectionCatalog->commit();

   if( guid.length() )  {
      return descFromGuid( guid, collectionCatalog, readOnly );
   }

   std::string type, connection, name;
   extract( physicalName, type, connection, name );  
   return CollectionDescription(name, type, connection);
}  

      
CollectionDescription
CollectionFactory::descFromLogicalName( const std::string& logicalName,
                                        pool::IFileCatalog* collectionCatalog,
                                        bool readOnly ) const
{
  if( !collectionCatalog ) collectionCatalog = getDefaultCatalog();
  collectionCatalog->start();
  std::string guid = collectionCatalog->lookupLFN( logicalName );
  collectionCatalog->commit();
  if( !guid.length() )  {
     std::string errorMsg = "No collection registered with logical name `" + logicalName + "' in  collection catalog.";
     throw std::runtime_error( errorMsg + " (APR: \" CollectionFactory::openWithLogicalName \" from \" CollectionBase \")" );
  }
  return descFromGuid( guid, collectionCatalog, readOnly );
}
                                        


CollectionDescription
CollectionFactory::descFromGuid( const pool::FileCatalog::FileID& guid,
                                 pool::IFileCatalog* collectionCatalog,
                                 bool readOnly ) const
{
  if( !collectionCatalog ) collectionCatalog = getDefaultCatalog();

  if( !readOnly && !isUnique( guid, *collectionCatalog ) ) {
     throw std::runtime_error( "Cannot open collection with GUID `" + guid + "' for updates because it has been replicated. (APR: \" CollectionFactory::openWithLogicalName \" from \" CollectionBase \")" );
  }

  std::string physicalName;
  std::string fileType;
  collectionCatalog->start();
  collectionCatalog->getFirstPFN( guid, physicalName, fileType );
  collectionCatalog->commit();
  if( physicalName.empty() )  {
     throw std::runtime_error( "No collection registered with GUID `" + guid + "' in collection catalog. (APR: \" CollectionFactory::openWithLogicalName \" from \" CollectionBase \")" );
  } 
  if( fileType != c_fileType )  {
     std::string errorMsg = "Entries in collection catalog must be of type `" + c_fileType + "'.";
      throw std::runtime_error( errorMsg + " (APR: \" CollectionFactory::openWithGuid \" from \" CollectionBase \")" );
  }
  
  std::string name, type, connection;
  extract( physicalName, type, connection, name );  
  return CollectionDescription( name, type, connection ); 
}


bool 
pool::CollectionFactory::isUnique( const pool::FileCatalog::FileID& guid,
                                   pool::IFileCatalog& collectionCatalog ) const
{
   IFileCatalog::Files  pfns;
   collectionCatalog.start();
   collectionCatalog.getPFNs( guid, pfns );
   collectionCatalog.commit();
   if( pfns.empty() ) {
      std::string errorMsg = "A Collection with GUID `" + guid + "' is not registered in the collection catalog.";
      throw std::runtime_error( errorMsg + " (APR: \" CollectionFactory::isUnique \" from \" CollectionBase \")" );
   }
   return pfns.size()==1;
}


void 
pool::CollectionFactory::extract( const std::string& physicalName,
                                  std::string& type,
                                  std::string& connection,
                                  std::string& name ) const
{
  typedef boost::tokenizer<boost::char_separator<char> > Tizer;
  boost::char_separator<char> sep( "|", "", boost::keep_empty_tokens );
  Tizer tizer( physicalName, sep);
  Tizer::iterator token=tizer.begin(); 
  type = *token;
  connection = *(++token);
  name = *(++token);
}




void 
pool::CollectionFactory::setDefaultCatalog( pool::IFileCatalog* catalog)
{
   if( m_catalog && m_myOwnCatalog ) {
      m_catalog->disconnect();
      delete m_catalog;
   }
   m_catalog = catalog; 
   m_myOwnCatalog=false;  
}


void 
pool::CollectionFactory::setWriteCatalog( const std::string &cat )
{
   if( !m_catalog ) {
      m_catalog = new IFileCatalog();
      m_myOwnCatalog = true;
   }
   m_catalog->setWriteCatalog( cat );
}
  

pool::IFileCatalog* 
pool::CollectionFactory::getDefaultCatalog() const
{  
   if( m_catalog )
      return m_catalog;
  
   m_catalog = new IFileCatalog();
   m_myOwnCatalog = true;
   
   bool ok = false;  
   const char* chr = getenv("POOL_COLLECTION_WRITE_CATALOG");
   if(chr && strlen(chr)){
      coral::MessageStream log( thisModule );
      log << coral::Debug << "setting write collection catalog: " << chr
	  << coral::MessageStream::endmsg;
      m_catalog->setWriteCatalog( chr );
      ok = true;
   }
   chr = getenv("POOL_COLLECTION_READ_CATALOGS");
   if(chr && strlen(chr)){
      coral::MessageStream log( thisModule );
      std::string str = chr;
      typedef boost::tokenizer<boost::char_separator<char> > Tizer;
      boost::char_separator<char> sep(" ,");
      Tizer tizer( str, sep);
      for( Tizer::iterator token=tizer.begin();
	   token!=tizer.end();
	   ++token){
	 log << coral::Debug << "adding read collection catalog: " << *token
	     << coral::MessageStream::endmsg;
	 m_catalog->addReadCatalog( *token );
      }
      ok = true;
   }
   if(!ok)
      m_catalog->setWriteCatalog("xmlcatalog_file:CollectionCatalog.xml");
  
   m_catalog->connect();

   return m_catalog;
}


