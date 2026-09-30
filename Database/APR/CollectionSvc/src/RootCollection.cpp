/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "RootCollection.h"
#include "CollectionCursor.h"

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
         if( m_ownStorageSvc ) {
            // only interact with the StorageSvc if we created it ourselves
            StatusCode sc = m_storageSvc->disconnect( m_fileDescr );
            if( sc.isSuccess() ) sc = m_storageSvc->endSession();
            if( !sc.isSuccess() ) {
               // just warn and continue, no much to be done here
               ATH_MSG_WARNING("StorageSvc connection to '" + m_fileName + "' could not be properly closed" );
            }
            delete m_storageSvc; m_storageSvc = nullptr;
            m_ownStorageSvc = false;
         } else {
            // release the database handle
            if( m_database ) {
               m_database.reset();
            }
         }
      }
   }


    // throw all errors as exceptions, because this method is called from the constructor
   void RootCollection::open()
   {
      ATH_MSG_VERBOSE( "Opening collection '" << m_fileName << "' in mode " << poolOptToRootOpt[m_mode] );
      DatabaseSpecification::NameType dbNameType = DatabaseSpecification::UNDEFINED;
      if( m_fileName.starts_with("PFN:") ) {
         dbNameType = DatabaseSpecification::PFN;
      } else if( m_fileName.starts_with("LFN:") ) {
         dbNameType = DatabaseSpecification::LFN;
      } else if( m_fileName.starts_with("FID:") ) {
         dbNameType = DatabaseSpecification::FID;
      }
      if( dbNameType == DatabaseSpecification::UNDEFINED ) {
         // if no qualifier is specified, assume it's a PFN
         dbNameType = DatabaseSpecification::PFN;
      } else {
         // remove the identified prefix
         m_fileName = m_fileName.substr(4);
      }

      if( !m_session ) {
         // not creating a new session to avoid playing with the filecatalog
         // working directly with the StorageSvc
         m_storageSvc = pool::createStorageSvc("StorageSvc");
         m_ownStorageSvc = true;
         if( !m_storageSvc->startSession( m_mode, m_description.type().type()) .isSuccess() ) {
            throw std::runtime_error( "RootCollection failed to start a session." );
         }
         m_fileDescr.initFromFilename( m_fileName );
         if( !m_storageSvc->connect( m_mode, m_fileDescr ).isSuccess() ) {
           throw std::runtime_error( "RootCollection failed to open: " + m_fileName + " for " + poolOptToRootOpt[m_mode] );
         }
      } else {
         m_storageSvc = &m_session->getStorageSvc( m_description.type().type() );
         m_ownStorageSvc = false;
         m_database = m_session->databaseHandle( m_fileName, dbNameType );
         if( !m_database ) {
            throw std::runtime_error( "Could not retrieve a database handle to '" + m_fileName + "' (APR: RootCollection)" );
         }
         if( m_database->openMode() == Io::INVALID ) {
            m_database->setTechnology( m_description.type().type() );
            m_database->connectForRead();
         }
         FileDescriptor* fd = m_database->fileDescriptor();
         if( !fd ) {
            throw std::runtime_error( "Could not retrieve connection info from DB '" + m_fileName + "' (APR: RootCollection)" );
         }
         m_fileDescr = *fd;
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
         // it seems merged files report multiple instances of the same container - avoid trying to process duplicates
         std::set<std::string> seenContainers;
         for( const Token *t : containerTokens ) {
            Token token(t);      // need a non-const Token
            const std::string& contName = db.cntName(token);
            const DbTypeInfo* typ_info = db.objectShape( token.classID() );
            if( contName.starts_with( tagContName ) and !seenContainers.contains( contName ) ) {
               seenContainers.insert( contName );
               const std::string& attrName = contName.substr( tagContName.size(), contName.size() - tagContName.size() - 1 );
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
            } else if( contName == newDHContName || contName == oldDHContName ) {
               ATH_MSG_DEBUG("  :container " << contName << " is the DataHeader container");
               DbContainer cnt( db.type() );
               if( cnt.open( db, contName, typ_info, token.technology(), db.openMode() ).isSuccess() )  {
                  m_dhCont = std::move(cnt);
               } else {
                  ATH_MSG_WARNING("EventTag container " << contName << " could not be opened");
               }
            }
         }
      } else {
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
      if( m_containerMap.empty() ) {
         m_containerMap.emplace( "Token", std::move(m_dhCont) );
      }
      pool::CollectionRowBuffer collectionRowBuffer;
      initNewRow(collectionRowBuffer);
      return std::make_unique<CollectionCursor>( m_description, collectionRowBuffer, m_containerMap);
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
