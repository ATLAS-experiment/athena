/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "RootCollection.h"
#include "CollectionCursor.h"

#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeList.h"

#include "PersistentDataModel/Token.h"
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

namespace pool {

   RootCollection::RootCollection( const pool::CollectionDescription* description,
                                   Io::IoFlag mode,
                                   ISession* session )
      : APRMessaging( "RootCollection"),
        m_description( *description ),
        m_name( description->name() ),
        m_fileName( description->connection() ),
        m_open( false )
      {
         RootCollection::open( mode, session );
      }


   RootCollection::~RootCollection() {
      if( m_open ) RootCollection::close();
   }


   void RootCollection::insertRow( const pool::CollectionRowBuffer& inputRowBuffer )
   {
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
   }


    // throw all errors as exceptions, because this method is called from the constructor
   void RootCollection::open( Io::IoFlag mode, ISession* session )
   {
      if( m_fileName.starts_with ( "PFN:") ) {
        m_fileName = m_fileName.substr(4);
      }

      if( mode == Io::READ ) {
         CollectionDescription desc( m_description.name(), m_description.type(), m_description.connection() );
         // clear the description
         m_description = std::move(desc);

         std::vector<const Token*> containerTokens;
         m_storageSvc.reset( pool::createStorageSvc("StorageSvc") );
         // PvG: TODO: On read use m_session
         if( !m_storageSvc->startSession( mode, m_description.type().type()).isSuccess() ) {
            throw std::runtime_error( "RootCollection failed to start a session." );
         }
         m_fileDescr.initFromFilename( m_fileName );
         if( !m_storageSvc->connect( mode, m_fileDescr ).isSuccess() ) {
            throw std::runtime_error( "RootCollection failed to open: " + m_fileName + " for " + poolOptToRootOpt[mode] );
         }
         DbDatabase db( m_fileDescr.dbc()->handle() );
         if( !db.containers(containerTokens, false).isSuccess() ) {
            throw std::runtime_error( "RootCollection: error reading " + m_fileName );
         }
         m_containerPrefix = APRDefaults::ReadConfig::getEventTagName( m_fileDescr.FID() );
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
               }
            }
         }
         if( m_containerMap.empty() ) {
            db.close().ignore();
            throw std::runtime_error( "No RootCollection found in " + m_fileName );
         }
      }
      if( mode == Io::WRITE || mode == Io::APPEND) {
         ATH_MSG_DEBUG( "Creating collection in overwrite mode..." );
         m_storageSvc.reset( pool::createStorageSvc("StorageSvc") );
         if( !m_storageSvc->startSession( mode, m_description.type().type()).isSuccess() ) {
            throw std::runtime_error( "RootCollection failed to start a session." );
         }
         m_fileDescr.initFromFilename( m_fileName );
         if( !m_storageSvc->connect( mode, m_fileDescr ).isSuccess() ) {
            throw std::runtime_error( "RootCollection failed to open: " + m_fileName + " for " + poolOptToRootOpt[mode] );
         }
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
      pool::CollectionRowBuffer collectionRowBuffer;
      initNewRow(collectionRowBuffer);
      return std::make_unique<CollectionCursor>( m_description, collectionRowBuffer, m_containerMap);
   }
}
