/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#include "DatabaseHandler.h"
#include "Container.h"
#include "PersistentDataModel/Token.h"
#include "StorageSvc/IStorageSvc.h"
#include "StorageSvc/Transaction.h"
#include "StorageSvc/Shape.h"
#include "StorageSvc/DbDatabase.h"
#include "StorageSvc/DbOption.h"
#include "StorageSvc/DbReflex.h"
#include "StorageSvc/DbTypeInfo.h"
#include "StorageSvc/DbConnection.h"

#include <exception>
#include <memory>

pool::PersistencySvc::DatabaseHandler::DatabaseHandler( pool::IStorageSvc& storageSvc,
                                                        pool::Session* session,
                                                        long technology,
                                                        const std::string& fid,
                                                        const std::string& pfn,
                                                        long accessmode ):
  m_storageSvc( storageSvc ),
  m_session( session ),
  m_fileDescriptor( fid, pfn ),
  m_technology( technology ),
  m_accessMode( accessmode )
{
  if ( ! m_storageSvc.connect( m_session,
                               m_accessMode,
                               m_fileDescriptor ).isSuccess() ) {
    throw std::runtime_error( "Could not connect to the database (APR: \" DatabaseHandler::DatabaseHandler \" from \" PersistencySvc \")" );
  }
}

pool::PersistencySvc::DatabaseHandler::~DatabaseHandler()
{
   int mode = 0;
   pool::DbStatus sc = m_storageSvc.openMode(m_fileDescriptor, mode);
   if ( sc.isSuccess() ) {
     rollBackTransaction();
     m_storageSvc.disconnect( m_fileDescriptor );
   }
}


bool
pool::PersistencySvc::DatabaseHandler::commitTransaction()
{
   return ( m_storageSvc.endTransaction( m_fileDescriptor.dbc(),
                                         Transaction::TRANSACT_COMMIT ).isSuccess() &&
            m_storageSvc.endTransaction( m_fileDescriptor.dbc(),
                                         Transaction::TRANSACT_FLUSH ).isSuccess() );
}


bool
pool::PersistencySvc::DatabaseHandler::commitAndHoldTransaction()
{
   return ( m_storageSvc.endTransaction( m_fileDescriptor.dbc(),
                                         Transaction::TRANSACT_COMMIT ).isSuccess() );
}


void
pool::PersistencySvc::DatabaseHandler::rollBackTransaction()
{
   m_storageSvc.endTransaction( m_fileDescriptor.dbc(), Transaction::TRANSACT_ROLLBACK );
}



bool
pool::PersistencySvc::DatabaseHandler::disconnectTransaction()
{
   rollBackTransaction();
   return ( m_storageSvc.disconnect( m_fileDescriptor ).isSuccess() );
}

std::vector< std::string >
pool::PersistencySvc::DatabaseHandler::containers()
{
   std::vector< std::string > result;
   std::vector<const Token*> containerTokens;
   pool::DatabaseConnection* connection = m_fileDescriptor.dbc();
   DbDatabase dbH(static_cast<DbDatabaseObj*>(connection->handle()));
   dbH.containers(containerTokens, false);
   for ( std::vector<const Token*>::const_iterator iToken = containerTokens.begin();
         iToken != containerTokens.end(); ++iToken ) {
      Token tok (*iToken);
      result.push_back( m_storageSvc.getContName(m_fileDescriptor, tok) );
   }
   return result;
}

pool::IContainer*
pool::PersistencySvc::DatabaseHandler::container( const std::string& containerName )
{
   // Check the existence of a given container.
   std::vector< std::string > allContainers = this->containers();
   for ( std::vector< std::string >::const_iterator iName = allContainers.begin();
         iName != allContainers.end(); ++iName ) {
      if ( *iName == containerName ) {
         return new pool::PersistencySvc::Container( m_fileDescriptor,
                                                     m_technology,
                                                     containerName );
      }
   }
   return 0;
}


const std::string&
pool::PersistencySvc::DatabaseHandler::pfn() const
{
  return m_fileDescriptor.PFN();
}


const std::string&
pool::PersistencySvc::DatabaseHandler::fid() const
{
  return m_fileDescriptor.FID();
}


long
pool::PersistencySvc::DatabaseHandler::technology() const
{
  return m_technology;
}


long
pool::PersistencySvc::DatabaseHandler::accessMode() const
{
  return m_accessMode;
}


Token*
pool::PersistencySvc::DatabaseHandler::writeObject( const std::string& containerName,
                                                    long minorTechnology,
                                                    const void* object,
                                                    const RootType& type )
{
  Token* token = 0;
  if ( ! object ) return token;

  // Get the persistent shape.
  const pool::Shape* shape = 0;
  Guid guid = DbReflex::guid(type);
  if ( m_storageSvc.getShape( m_fileDescriptor, guid, shape ) == pool::IStorageSvc::SHAPE_NOT_AVAILIBLE ) {
    m_storageSvc.createShape( m_fileDescriptor, containerName, guid, shape );
  }

  if ( shape )  {
    if ( m_storageSvc.allocate( m_fileDescriptor,
                                containerName,
                                minorTechnology,
                                object,
                                shape, 
                                token ).isSuccess() ) {
      token->setClassID( shape->shapeID() );
    }
  }
  return token;
}


void*
pool::PersistencySvc::DatabaseHandler::readObject( const Token& token, void* object )
{
  void* result( object );

  // Get the persistent shape
  const pool::Shape* shape = 0;
  if ( m_storageSvc.getShape( m_fileDescriptor, token.classID(), shape ) != pool::IStorageSvc::IS_PERSISTENT_SHAPE )
    return result;

  if (! m_storageSvc.read( m_fileDescriptor, token, shape, &result ).isSuccess() ) {
    result = 0;
  }

  return result;
}


bool
pool::PersistencySvc::DatabaseHandler::attribute( const std::string& attributeName,
                                                  void* data,
                                                  const std::type_info& typeInfo,
                                                  const std::string& option )
{
  pool::DbOption databaseOption( attributeName, option );
  pool::DatabaseConnection* connection = m_fileDescriptor.dbc();
  DbDatabase dbH(static_cast<DbDatabaseObj*>(connection->handle()));
  pool::DbStatus sc = dbH.getOption(databaseOption);
  if ( !sc.isSuccess() ) return false;
  if ( databaseOption.i_getValue( typeInfo, data ).isSuccess() ) {
    return true;
  }
  else {
    return false;
  }
}


bool
pool::PersistencySvc::DatabaseHandler::setAttribute( const std::string& attributeName,
                                                     const void* data,
                                                     const std::type_info& typeInfo,
                                                     const std::string& option )
{
  pool::DbOption databaseOption( attributeName, option );
  pool::DbStatus sc = databaseOption.i_setValue( typeInfo, const_cast<void*>( data ) );
  if ( !sc.isSuccess() ) return false;
  pool::DatabaseConnection* connection = m_fileDescriptor.dbc();
  DbDatabase dbH(static_cast<DbDatabaseObj*>(connection->handle()));
  sc = dbH.setOption(databaseOption);
  if ( !sc.isSuccess() ) {
    return false;
  }
  else {
    return true;
  }
}
