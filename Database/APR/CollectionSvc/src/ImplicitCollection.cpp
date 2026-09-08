/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "ImplicitCollection.h"
#include "ImplicitCollectionIterator.h"

#include "PoolSvc/ISession.h"
#include "PoolSvc/IContainer.h"

#include "StorageSvc/DbType.h"
#include "StorageSvc/APRDefaults.h"

#include <sstream>
#include <memory>
#include <format>


namespace pool {

   ImplicitCollection::ImplicitCollection( const CollectionDescription* description,
                       Io::IoFlag mode,
                       ISession* session )
         : APRMessaging("ImplicitCollection"),
         m_container( 0 ),
         m_description( *description )
   {
      open( mode, session );
   }


   void
   ImplicitCollection::open( Io::IoFlag/* mode*/, ISession* session )
   {
      if( !session ) {
         throw std::runtime_error( "session object not set (APR: \" ImplicitCollection::ImplicitCollection \" from \" ImplicitCollection \")" );
      }
      // parse the connection string
      const std::string& connection = m_description.connection();
      auto database = session->databaseHandle( connection.substr(4), DatabaseSpecification::PFN );
      if( !database ) {
         throw std::runtime_error( "Could not retrieve a database handle (APR: \" ImplicitCollection::ImplicitCollection \" from \" ImplicitCollection \")" );
      }

      if ( database->openMode() == Io::INVALID ) {
         // The following fix was added to allow the reading of an implicit 
         // collection in the absence of a POOL file catalog. For now, it assumes 
         // a ROOT persistency storage type if no other type can be found. 
         try {
            database->connectForRead();
         }
         catch ( std::runtime_error& /* exception */ ) {
	    // use provided tech name or assume ROOT
	    // setting tech will make connectForRead work without a catalog
            database->setTechnology( ROOT_StorageType.type() );
            database->connectForRead();
         }
      }

      const std::string& newName = std::format("{}(DataHeader)", APRDefaults::ReadConfig::getDataHeaderName( database->fid() ));
      const std::string& oldName = std::format("{}_DataHeader", APRDefaults::ReadConfig::getDataHeaderName( database->fid() ));
      std::vector< std::string > containers = database->containers();
      for( std::vector< std::string >::const_iterator iContainer = containers.begin();
           iContainer != containers.end(); ++iContainer ) {
         if( newName == *iContainer ) {
            m_container = database->containerHandle( newName );
            break;
         } else if( oldName == *iContainer ) {
            m_container = database->containerHandle( oldName );
            break;
         }
      }

      if( !m_container ) {
         throw std::runtime_error( "Could not open the container (APR: \" ImplicitCollection::ImplicitCollection \" from \" ImplicitCollection \")" );
      }
      ATH_MSG_INFO( "Opened the implicit collection with connection string '" << connection );
   }


   ImplicitCollection::~ImplicitCollection()
   {
      delete m_container;
   }


   void
   ImplicitCollection::insertRow(const pool::CollectionRowBuffer& /*inputRowBuffer*/)
   {
      throw std::runtime_error( "Cannot modify the data of a implicit collection. (APR: \" ImplicitCollection::insertRow \" from \" ImplicitCollection \")" );
   }


   const CollectionDescription& ImplicitCollection::description() const
   {
      return m_description;
   }

   std::unique_ptr<ICollectionCursor> ImplicitCollection::cursor()
   {
      return std::make_unique<ImplicitCollectionIterator>( *m_container );
   }
}
