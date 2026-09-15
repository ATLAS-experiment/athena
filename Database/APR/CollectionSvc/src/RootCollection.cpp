/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "RootCollection.h"
#include "CollectionCursor.h"
#include "ImplicitCollectionIterator.h"

#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeList.h"

#include "PersistentDataModel/Token.h"
#include "PoolSvc/ISession.h"
#include "StorageSvc/APRDefaults.h"
#include "StorageSvc/DbReflex.h"
#include "StorageSvc/DbDatabase.h"
#include "StorageSvc/DbConnection.h"
#include "StorageSvc/DbContainer.h"
#include "StorageSvc/DbTypeInfo.h"
#include "StorageSvc/IStorageSvc.h"

#include "CollectionSvc/CollectionColumn.h"

#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IFileMgr.h"
#include "GaudiKernel/IService.h"

#include <exception>
#include <map>

#include <iostream>
using namespace std;

namespace pool {

   RootCollection::RootCollection( const pool::CollectionDescription& description,
                                   Io::IoFlag mode,
                                   ISession* session )
      : APRMessaging( "RootCollection"),
        m_description( description ),
        m_name( description.name() ),
        m_fileName( description.connection() ),
        m_mode( mode ),
        m_open( false ),
        m_session( session )
      {
         RootCollection::open();
      }


   RootCollection::~RootCollection() {
      if( m_open ) RootCollection::close();
   }


   void RootCollection::insertRow( const pool::CollectionRowBuffer& inputRowBuffer )
   {
      if( m_mode == Io::READ ) {
         throw std::runtime_error( "Cannot modify the data of a collection in READ open mode. (APR: \" RootCollection::insertRow \" from \" RootCollection \")" );
      }
  
      std::string strToken = inputRowBuffer.token().toString();
      writeColumn( m_description.tokenColumn().name(), &strToken, typeid(std::string) );

      coral::AttributeList attribs_nc = inputRowBuffer.attributeList();
      for( coral::Attribute& att : attribs_nc ) {
         writeColumn( att.specification().name(), att.addressOfData(), att.specification().type() );
      }
      if( !m_storageSvc->endTransaction( m_fileDescr, pool::Transaction::TRANSACT_COMMIT ).isSuccess() ) {
         throw std::runtime_error( "RootCollection::insertRow: commit failed" );
      }
   }


   void RootCollection::writeColumn( const std::string& columnName, const void* data, const std::type_info& typeInfo )
   {
      const Guid guid = DbReflex::guid(typeInfo);
      const Shape* shape = nullptr;
      if( !m_storageSvc->getShape( m_fileDescr, guid, shape ).isSuccess() ) {
         shape = m_storageSvc->createShape( guid );
      }
      std::string containerName = std::format("{}({})", m_containerPrefix, columnName );
      Token *tp = nullptr;
      if( m_storageSvc->allocate( m_fileDescr, containerName, m_description.type().type(), data, shape, tp ).isSuccess() ) {
         delete tp; tp = nullptr;
      } else {
         throw std::runtime_error( "RootCollection: Failed to write collection column " + columnName );
      }
   }


   void RootCollection::commit( bool )
   {
      ATH_MSG_DEBUG( "RootCollection::commit: " + m_fileName );
      if( m_open ) {
         if( !m_storageSvc->endTransaction( m_fileDescr, Transaction::TRANSACT_COMMIT ).isSuccess()
             or !m_storageSvc->endTransaction( m_fileDescr, Transaction::TRANSACT_FLUSH ).isSuccess() ) {
            throw std::runtime_error( "RootCollection::commit: commit failed" );
         }
      }
   }


   void RootCollection::close()
   {
      ATH_MSG_INFO( "Closing " << (m_open? "open":"not open") << " collection '" << m_fileName << "'" );
      if(m_open) {
         m_open = false;
         if( !m_storageSvc->disconnect( m_fileDescr ).isSuccess() ) {
            throw std::runtime_error( "RootCollection '" + m_fileName + "' could not be properly closed" );
         }
         m_storageSvc->endSession().ignore();
      }
      if( m_ownStorageSvc ) {
         delete m_storageSvc;
         m_storageSvc = nullptr;
      }
   }


    // throw all errors as exceptions, because this method is called from the constructor
   void RootCollection::open()
   {
      if( m_fileName.empty() ) {
         ATH_MSG_ERROR( "No database name given" );
         throw std::runtime_error( "No database name (APR: RootCollection::open() )" );
      }
      if( m_fileName.starts_with("PFN:") ) {
         m_fileName = m_fileName.substr(4);
         // TODO: handle other prefixes too
      }
      if( !m_session ) {
         // not creating a new session to avoid playing with the filecatalog
         // working directly with the StorageSvc
         m_storageSvc = pool::createStorageSvc("StorageSvc");
         m_ownStorageSvc = true;
         if( !m_storageSvc->startSession( m_mode, m_description.type().type()) .isSuccess() ) {
            throw std::runtime_error( "RootCollection failed to start a session." );
         }
      } else {
         m_storageSvc = &m_session->getStorageSvc( m_description.type().type() );
         m_ownStorageSvc = false;
      }
      m_fileDescr.initFromFilename( m_fileName );
      if( !m_storageSvc->connect( m_mode, m_fileDescr ).isSuccess() ) {
         throw std::runtime_error( "RootCollection failed to open: " + m_fileName + " for " + poolOptToRootOpt[m_mode] );
      }

      if( m_mode == Io::READ ) {
         CollectionDescription desc( m_description.name(), m_description.type(), m_description.connection() );
         // clear the description
         m_description = std::move(desc);

         std::vector<const Token*> containerTokens;
         DbDatabase db( m_fileDescr.dbc()->handle() );
         if( !db.containers(containerTokens, false).isSuccess() ) {
            throw std::runtime_error( "RootCollection: error reading " + m_fileName );
         }
         m_containerPrefix = APRDefaults::ReadConfig::getEventTagName( m_fileDescr.FID() );
         const std::string& newDHContName = std::format("{}(DataHeader)", APRDefaults::ReadConfig::getDataHeaderName( m_fileDescr.FID() ));
         const std::string& oldDHContName = std::format("{}_DataHeader",  APRDefaults::ReadConfig::getDataHeaderName( m_fileDescr.FID() ));
         ATH_MSG_DEBUG("Opening RootCollection '" << m_fileName << "' using container prefix: " << m_containerPrefix );
         std::string tagContName = m_containerPrefix + "(";
         for( const Token *t : containerTokens ) {
            Token token(t);      // need a non-const Token
            const std::string& contName = db.cntName(token);
            if( contName.starts_with( tagContName ) ) {
               const std::string& attrName = contName.substr( tagContName.size(), contName.size() - tagContName.size() - 1 );
               const DbTypeInfo* typ_info = db.objectShape( token.classID() );
               ATH_MSG_DEBUG("  :container " << contName << " with attribute " << attrName << " of type: " << typ_info->clazz().Name());
               DbContainer cnt( db.type() );
               if( cnt.open( db, contName, typ_info, token.technology(), db.openMode() ).isSuccess() )  {
                  if( attrName != m_description.tokenColumn().name() ) {
                     m_description.insertColumn( attrName, typ_info->clazz().Name() );
                  }
                  m_containerMap.emplace( attrName, std::move(cnt) );
               } else {
                  ATH_MSG_WARNING("EventTag container " << contName << " could not be opened");
               }
            } else if( contName == newDHContName or contName == oldDHContName ) {
               ATH_MSG_DEBUG("  :container " << contName << " is the DataHeader container");
               m_dhContName = contName;
            }
         }
         if( m_containerMap.empty() and m_dhContName.empty() ) {
            db.close().ignore();
            throw std::runtime_error( "No Event Collections found in " + m_fileName );
         }
      }
      if( m_mode == Io::WRITE || m_mode == Io::APPEND) {
         ATH_MSG_DEBUG( "Creating collection in overwrite mode..." );
         m_containerPrefix = APRDefaults::WriteConfig::getEventTagName();
      }
      m_open = true;
   }

    
   const CollectionDescription& RootCollection::description() const {
      return m_description;
   }

   std::unique_ptr<ICollectionCursor> RootCollection::cursor() {
      if( !m_open ) {
         throw std::runtime_error( "Attempt to get cursor for a closed collection. (APR: \" RootCollection::cursor \" from \" RootCollection \")" );
      }
      if( !m_containerMap.empty() ) {
         pool::CollectionRowBuffer collectionRowBuffer;
         initNewRow(collectionRowBuffer);
         return std::make_unique<CollectionCursor>( m_description, collectionRowBuffer, m_containerMap);
      } else {

         auto database = m_session->databaseHandle( m_fileName, DatabaseSpecification::PFN );
         if( !database ) {
            throw std::runtime_error( "Could not retrieve a database handle (APR: RootCollection::cursor)" );
         }
         if( database->openMode() == Io::INVALID ) {
            cout << "MN: RootCollection::cursor: database is not open, opening " << m_fileName << " for read" << endl;
            database->setTechnology( m_description.type().type() );
            database->connectForRead();
         }
         IContainer *dhCont = database->containerHandle( m_dhContName );
         if( !dhCont ) {
            throw std::runtime_error( "Could not retrieve a handle to the DataHeader container (APR: RootCollection::cursor)" );
         }
         return std::make_unique<ImplicitCollectionIterator>( *dhCont );
      }
   }


   /// Initialize a new RowBuffer by adding all Attributes and Tokens of this collection to it
   void RootCollection::initNewRow( CollectionRowBuffer& rowBuffer ) const
   {
      coral::AttributeList          attributeList;

      for( int j = 0; j < description().numberOfAttributeColumns(); j++ ) {
         const auto& attrCol = description().attributeColumn( j );
         attributeList.extend( attrCol.name(), attrCol.type() );
      }
      rowBuffer.setAttributeList( attributeList );   
   }

} //namespace pool
