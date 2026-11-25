/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "DatabaseHandler.h"
#include "Container.h"
#include "PersistentDataModel/Token.h"
#include "StorageSvc/IStorageTool.h"
#include "StorageSvc/DbDatabase.h"
#include "StorageSvc/DbOption.h"
#include "StorageSvc/DbConnection.h"

#include <exception>
#include <memory>

DatabaseHandler::DatabaseHandler( ToolHandle<IStorageTool> storageTool,
                                  long technology,
                                  const std::string& fid,
                                  const std::string& pfn,
                                  Io::IoFlag accessmode ) :
        m_storageTool( storageTool ),
        m_fileDescriptor( fid, pfn ),
        m_technology( technology ),
        m_accessMode( accessmode ) {
   if ( ! m_storageTool->connect( m_accessMode, m_fileDescriptor ).isSuccess() ) {
      throw std::runtime_error( "Could not connect to the database (APR: \" DatabaseHandler::DatabaseHandler \" from \" PersistencySvc \")" );
   }
}

DatabaseHandler::~DatabaseHandler() {
   Io::IoFlag mode = Io::INVALID;
   if( m_storageTool->openMode(m_fileDescriptor, mode).isSuccess() ) {
      m_storageTool->disconnect( m_fileDescriptor ).ignore();
   }
}

bool DatabaseHandler::commitTransaction() {
   return m_storageTool->endTransaction( m_fileDescriptor, Transaction::TRANSACT_COMMIT ).isSuccess() &&
          m_storageTool->endTransaction( m_fileDescriptor, Transaction::TRANSACT_FLUSH ).isSuccess();
}

bool DatabaseHandler::commitAndHoldTransaction() {
   return m_storageTool->endTransaction( m_fileDescriptor, Transaction::TRANSACT_COMMIT ).isSuccess();
}

bool DatabaseHandler::disconnectTransaction() {
   return ( m_storageTool->disconnect( m_fileDescriptor ).isSuccess() );
}

std::vector< std::string > DatabaseHandler::containers() {
   std::vector< std::string > result;
   std::vector<const Token*> containerTokens;
   DbDatabase dbH( m_fileDescriptor.dbc()->handle() );
   if( !dbH.containers(containerTokens, false).isSuccess() ) {
      return result;
   }
   for ( std::vector<const Token*>::const_iterator iToken = containerTokens.begin();
         iToken != containerTokens.end(); ++iToken ) {
      Token tok (*iToken);
      result.push_back( m_storageTool->getContName(m_fileDescriptor, tok) );
   }
   return result;
}

Container* DatabaseHandler::container( const std::string& containerName ) {
   // Check the existence of a given container.
   std::vector< std::string > allContainers = this->containers();
   for ( std::vector< std::string >::const_iterator iName = allContainers.begin();
         iName != allContainers.end(); ++iName ) {
      if ( *iName == containerName ) {
         return new Container( m_fileDescriptor, containerName );
      }
   }
   return 0;
}

const std::string& DatabaseHandler::pfn() const {
  return m_fileDescriptor.PFN();
}

const std::string& DatabaseHandler::fid() const {
  return m_fileDescriptor.FID();
}

long DatabaseHandler::technology() const {
  return m_technology;
}

Io::IoFlag DatabaseHandler::accessMode() const {
  return m_accessMode;
}

Token* DatabaseHandler::writeObject( const std::string& containerName,
                                     long minorTechnology,
                                     const void* object,
                                     const RootType& type ) {
  Token* token = nullptr;
  if( !m_storageTool->write( m_fileDescriptor,
                           containerName,
                           minorTechnology,
                           object,
                           type,
                           token ).isSuccess() ) {
     return nullptr;
  }
  return token;
}

void* DatabaseHandler::readObject( const Token& token, void* object ) {
  void* result( object );
  if( !m_storageTool->read( m_fileDescriptor, token, &result ).isSuccess() ) {
     return nullptr;
  }
  return result;
}

bool DatabaseHandler::attribute( const std::string& attributeName,
                                 void* data,
                                 const std::type_info& typeInfo,
                                 const std::string& option ) {
  DbOption databaseOption( attributeName, option );
  DbDatabase dbH( m_fileDescriptor.dbc()->handle() );
  if( !dbH.getOption(databaseOption).isSuccess() ) return false;
  return databaseOption.i_getValue(typeInfo, data).isSuccess();
}

bool DatabaseHandler::setAttribute( const std::string& attributeName,
                                    const void* data,
                                    const std::type_info& typeInfo,
                                    const std::string& option ) {
  DbOption databaseOption( attributeName, option );
  if( !databaseOption.i_setValue( typeInfo, const_cast<void*>( data ) ).isSuccess() ) return false;
  DbDatabase dbH( m_fileDescriptor.dbc()->handle() );
  return dbH.setOption(databaseOption).isSuccess();
}
