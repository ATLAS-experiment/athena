/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "ImplicitCollection.h"
#include "ImplicitCollectionIterator.h"

#include "PersistencySvc/ISession.h"
#include "PersistencySvc/IDatabase.h"
#include "PersistencySvc/IContainer.h"

#include "CoralBase/MessageStream.h"

#include "StorageSvc/DbType.h"

#include <sstream>
#include <memory>


namespace pool {

   ImplicitCollection::
   ImplicitCollection( ISession* session,
                       const std::string& connection,
                       const std::string& name,
                       ICollection::OpenMode mode )
         :
         m_container( 0 ),
         m_description( name,"ImplicitCollection", connection )
   {
      open( mode, session );
   }


   ImplicitCollection::
   ImplicitCollection( const ICollectionDescription* description,
                       ICollection::OpenMode mode,
                       ISession* session )
         :
         m_container( 0 ),
         m_description( *description )
   {
      open( mode, session );
   }


   void
   ImplicitCollection::
   open( ICollection::OpenMode mode,
         ISession* session )
   {
      coral::MessageStream log( "ImplicitCollection");

      if ( mode != ICollection::READ ) {
         log << coral::Error << "An implicit collection can be opened only in READ mode" << coral::MessageStream::endmsg;
         throw std::runtime_error( "An implicit collection can be opened only in READ mode (APR: \" ImplicitCollection::ImplicitCollection \" from \" ImplicitCollection \")" );
      }

      DatabaseSpecification::NameType dbNameType = DatabaseSpecification::UNDEFINED;

      // parse the connection string
      const std::string& connection = m_description.connection();
      std::string::size_type pos = connection.find( ":" );
      if ( pos == std::string::npos ) {
         log << coral::Error << "Badly formed connection string : \"" << connection << "\"" << coral::MessageStream::endmsg;
         throw std::runtime_error( "Badly formed connection string (APR: \" ImplicitCollection::ImplicitCollection \" from \" ImplicitCollection \")" );
      }

      const std::string dbType = connection.substr( 0, pos );

      if ( dbType == "PFN" ) dbNameType = DatabaseSpecification::PFN;
      else if ( dbType == "LFN" ) dbNameType = DatabaseSpecification::LFN;
      else if ( dbType == "FID" ) dbNameType = DatabaseSpecification::FID;
      else {
         log << coral::Error << "Unrecognizable database name type : \"" << dbType << "\"" << coral::MessageStream::endmsg;
         throw std::runtime_error( "Unrecognizable database name type : " + dbType + " (APR: \"ImplicitCollection::ImplicitCollection (APR: \" ImplicitCollection \")" );
      }

      std::string dbName = "";
      std::string technologyName = "";
      std::istringstream is( connection.substr( pos + 1 ).c_str() );
      is >> dbName >> technologyName;

      if ( dbName.empty() ) {
         log << coral::Error << "Invalid database name " << coral::MessageStream::endmsg;
         throw std::runtime_error( "Invalid database name (APR: \" ImplicitCollection::ImplicitCollection \" from \" ImplicitCollection \")" );
      }
  
      if( !session ) {
         throw std::runtime_error( "session object not set (APR: \" ImplicitCollection::ImplicitCollection \" from \" ImplicitCollection \")" );
      }
  
      auto database = session->databaseHandle( dbName, dbNameType );
      if( !database ) {
         throw std::runtime_error( "Could not retrieve a database handle (APR: \" ImplicitCollection::ImplicitCollection \" from \" ImplicitCollection \")" );
      }

      if ( database->openMode() == IDatabase::CLOSED ) {
         // The following fix was added to allow the reading of an implicit 
         // collection in the absence of a POOL file catalog. For now, it assumes 
         // a ROOT persistency storage type if no other type can be found. 
         try {
            database->connectForRead();
         }
         catch ( std::runtime_error& /* exception */ ) {
	    // use provided tech name or assume ROOT
            DbType theDbType = (technologyName != "") ? DbType::getType( technologyName ) : ROOT_StorageType;
	    // setting tech will make connectForRead work without a catalog
            database->setTechnology( theDbType.type() );
            database->connectForRead();
         }
      }

      const std::string& name = m_description.name();
      std::vector< std::string > containers = database->containers();
      for( std::vector< std::string >::const_iterator iContainer = containers.begin();
           iContainer != containers.end(); ++iContainer ) {
         if( name == *iContainer ) {
            m_container = database->containerHandle( name );
            break;
         }
      }

      if( !m_container ) {
         throw std::runtime_error( "Could not open the container " + name + " (APR: \" ImplicitCollection::ImplicitCollection \" from \" ImplicitCollection \")" );
      }
      log << coral::Info << "Opened the implicit collection with connection string \""
          << connection << "\"" << coral::MessageStream::endmsg
          << "and a name \"" << name << "\"" << coral::MessageStream::endmsg;
   }


   
   ImplicitCollection::~ImplicitCollection()
   {
      delete m_container;
   }

   

   ICollection::OpenMode 
   ImplicitCollection::openMode() const{
      return ICollection::READ;
   }


   // old method implemented for backward compatibility
   // and maybe also for the ease of use?
   ImplicitCollectionIterator*
   ImplicitCollection::select( const std::string &/* primaryQuery*/,
                               std::string,
                               std::string )
   {
      // iterator object supporting the collection query interface
      std::unique_ptr<ImplicitCollectionIterator>
	 iterquery( new ImplicitCollectionIterator( *m_container, m_description ) );
      iterquery->execute();
      return iterquery.release();
   }



   void
   ImplicitCollection::insertRow(const pool::CollectionRowBuffer& /*inputRowBuffer*/)
   {
      throw std::runtime_error( "Cannot modify the data of a implicit collection. (APR: \" ImplicitCollection::insertRow \" from \" ImplicitCollection \")" );
   }



   void
   ImplicitCollection::commit(bool)
   {
   }


   void
   ImplicitCollection::close()
   {
      // can't be closed
   }


   void
   ImplicitCollection::open()
   {
      // hmm, no-op at the moment  //MN
   }

   bool
   ImplicitCollection::isOpen() const
   {
      return true;
   }

   const ICollectionDescription& ImplicitCollection::description() const
   {
      return m_description;
   }

      
   ICollectionQuery* ImplicitCollection::newQuery()
   {
      return new ImplicitCollectionIterator( *m_container, m_description ); 
   }
}
