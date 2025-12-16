/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MicroSessionManager.h"
#include "DatabaseRegistry.h"
#include "DatabaseHandler.h"
#include "StorageSvc/IStorageSvc.h"
#include "PersistencySvc/ITransaction.h"
#include "StorageSvc/DatabaseConnection.h"
#include "StorageSvc/DbDatabase.h"
#include "StorageSvc/DbOption.h"
#include "StorageSvc/DbStatus.h"
#include "StorageSvc/pool.h"

#include <exception>

pool::PersistencySvc::MicroSessionManager::MicroSessionManager( pool::PersistencySvc::DatabaseRegistry& registry,
                                                                pool::ITransaction& transaction,
                                                                long technology ):
  m_registry( registry ),
  m_transaction( transaction ),
  m_storageSvc( 0 ),
  m_session( 0 ),
  m_technology( technology ),
  m_databaseHandlers()
{
  m_storageSvc = createStorageSvc("StorageSvc");
  if ( ! m_storageSvc ) {
    throw std::runtime_error( "Could not create a StorageSvc object (APR: \" MicroSessionManager::MicroSessionManager \" from \" PersistencySvc \"" );
  }
}


pool::PersistencySvc::MicroSessionManager::~MicroSessionManager()
{
  this->disconnectAll();
  m_storageSvc->release();
}


pool::PersistencySvc::DatabaseHandler*
pool::PersistencySvc::MicroSessionManager::connect( const std::string& fid,
                                                    const std::string& pfn,
                                                    long accessMode )
{
  if ( ! m_transaction.isActive() ) return 0;
  if ( m_databaseHandlers.empty() ) {
    long mode = pool::READ;
    if ( m_transaction.type() == ITransaction::UPDATE ) {
      mode = pool::UPDATE;
    }

    if ( ! m_session ) {
      if ( ! ( m_storageSvc->startSession( mode,
                                           m_technology,
                                           m_session ).isSuccess() ) ) {
        return 0;
      }
    }
  }

  pool::PersistencySvc::DatabaseHandler* db = 0;
  try {
    db = new pool::PersistencySvc::DatabaseHandler( *m_storageSvc,
                                                    m_session,
                                                    m_technology,
                                                    fid,
                                                    pfn,
                                                    accessMode );
    m_registry.registerDatabaseHandler( db );
    m_databaseHandlers.insert( db );
  }
  catch( std::runtime_error& /* error */) { // FIXME, this looks dangerous
  }

  if ( m_databaseHandlers.empty() && m_session ) {
    m_storageSvc->endSession( m_session );
    m_session = 0;
  }
  return db;
}


void
pool::PersistencySvc::MicroSessionManager::disconnect( pool::PersistencySvc::DatabaseHandler* database )
{
  std::set< pool::PersistencySvc::DatabaseHandler* >::iterator idb = m_databaseHandlers.find( database );
  if ( idb != m_databaseHandlers.end() ) {
    m_registry.deregisterDatabaseHandler( *idb );
    delete *idb;
    m_databaseHandlers.erase( idb );
  }
  if ( m_databaseHandlers.empty() && m_session ) {
    m_storageSvc->endSession( m_session );
    m_session = 0;
  }
}


bool
pool::PersistencySvc::MicroSessionManager::disconnectAll()
{
  bool ret = true;
  for ( std::set< pool::PersistencySvc::DatabaseHandler* >::iterator idb = m_databaseHandlers.begin();
        idb != m_databaseHandlers.end(); ++idb ) {
    m_registry.deregisterDatabaseHandler( *idb );
    ret = (*idb)->disconnectTransaction();
    delete *idb;
  }
  m_databaseHandlers.clear();

  if ( m_session ) {
    m_storageSvc->endSession( m_session );
    m_session = 0;
  }
  return ret;
}


long
pool::PersistencySvc::MicroSessionManager::technology() const
{
  return m_technology;
}


std::string
pool::PersistencySvc::MicroSessionManager::fidForPfn( const std::string& pfn )
{
  if ( m_databaseHandlers.empty() ) {
    long mode = pool::READ;
    if ( ! ( m_storageSvc->startSession( mode,
                                         m_technology,
                                         m_session ).isSuccess() ) ) {
      return "";
    }
  }

  std::string fid = "";

  pool::DbStatus sc;
  pool::FileDescriptor fd( pfn, pfn );
  // this is only a temporary FID so use a special pattern to make that clear
  fd.setFID( fd.FID().substr(0,24) + "0FF0FF0FF0FF" );
  sc = m_storageSvc->existsConnection( m_session, pool::READ, fd );
  if ( !( sc.value() == static_cast<unsigned int>( IStorageSvc::CONNECTION_NOT_EXISTING ) ||
          sc.value() == static_cast<unsigned int>( IStorageSvc::INVALID_SESSION_TOKEN ) ) ) {
    if ( m_storageSvc->connect( m_session, pool::READ, fd ).isSuccess() ) {
      pool::DatabaseConnection* connection = fd.dbc();
      DbDatabase dbH(static_cast<DbDatabaseObj*>(connection->handle()));
      if ( ! dbH.param( "FID", fid ).isSuccess() ) fid = "";
      m_storageSvc->disconnect( fd );
    }
  }

  if ( m_databaseHandlers.empty() ) {
    m_storageSvc->endSession( m_session );
    m_session = 0;
  }

  return fid;
}


bool
pool::PersistencySvc::MicroSessionManager::attributeOfType( const std::string& attributeName,
                                                            void* data,
                                                            const std::type_info& typeInfo,
                                                            const std::string& option )
{
  if ( ! m_session ) {
    long mode = pool::READ;
    if ( m_transaction.type() == ITransaction::UPDATE ) {
      mode = pool::UPDATE;
    }
    if ( ! ( m_storageSvc->startSession( mode,
                                         m_technology,
                                         m_session ).isSuccess() ) ) {
      return false;
    }
  }

  pool::DbOption domainOption( attributeName, option );
  pool::DbStatus sc = m_storageSvc->getDomainOption( m_session, domainOption );
  if ( !sc.isSuccess() ) return false;
  if ( domainOption.i_getValue( typeInfo, data ).isSuccess() ) {
    return true;
  }
  else {
    return false;
  }
}

bool
pool::PersistencySvc::MicroSessionManager::setAttributeOfType( const std::string& attributeName,
                                                               const void* data,
                                                               const std::type_info& typeInfo,
                                                               const std::string& option )
{
  if ( ! m_session ) {
    long mode = pool::READ;
    if ( m_transaction.type() == ITransaction::UPDATE ) {
      mode = pool::UPDATE;
    }
    if ( ! ( m_storageSvc->startSession( mode,
                                         m_technology,
                                         m_session ).isSuccess() ) ) {
      return false;
    }
  }

  pool::DbOption domainOption( attributeName, option );
  pool::DbStatus sc = domainOption.i_setValue( typeInfo, const_cast<void*>( data ) );
  if ( !sc.isSuccess() ) return false;
  sc = m_storageSvc->setDomainOption( m_session, domainOption );
  if ( !sc.isSuccess() ) {
    return false;
  }
  else {
    return true;
  }
}
