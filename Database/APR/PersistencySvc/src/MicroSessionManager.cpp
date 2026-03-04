/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MicroSessionManager.h"
#include "DatabaseRegistry.h"
#include "DatabaseHandler.h"
#include "StorageSvc/IStorageSvc.h"
#include "PersistencySvc/ITransaction.h"
#include "StorageSvc/DatabaseConnection.h"
#include "StorageSvc/DbDatabase.h"
#include "StorageSvc/DbOption.h"
#include "StorageSvc/pool.h"

#include "GaudiKernel/StatusCode.h"
#include <exception>

pool::PersistencySvc::MicroSessionManager::MicroSessionManager( pool::PersistencySvc::DatabaseRegistry& registry,
                                                                long technology ):
  m_registry( registry ),
  m_storageSvc( 0 ),
  m_inSession( false ),
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


bool
pool::PersistencySvc::MicroSessionManager::connect( ITransaction::Type transType )
{
  if( !m_inSession ) {
    long mode = (transType == ITransaction::UPDATE) ? pool::UPDATE : pool::READ;
    m_inSession = m_storageSvc->startSession(mode, m_technology).isSuccess();
  }
  return m_inSession;
}


pool::PersistencySvc::DatabaseHandler*
pool::PersistencySvc::MicroSessionManager::connect( ITransaction::Type transType,
                                                    const std::string& fid,
                                                    const std::string& pfn,
                                                    long accessMode )
{
  if( transType == ITransaction::INACTIVE ) return 0;
  if( m_databaseHandlers.empty() ) {
    long mode = (transType == ITransaction::UPDATE) ? pool::UPDATE : pool::READ;
    if( !m_inSession ) {
      if( !m_storageSvc->startSession(mode, m_technology).isSuccess() ) {
        return nullptr;
      }
      m_inSession = true;
    }
  }

  pool::PersistencySvc::DatabaseHandler* db = 0;
  try {
    db = new pool::PersistencySvc::DatabaseHandler( *m_storageSvc,
                                                    m_technology,
                                                    fid,
                                                    pfn,
                                                    accessMode );
    m_registry.registerDatabaseHandler( db );
    m_databaseHandlers.insert( db );
  }
  catch( std::runtime_error& /* error */) { // FIXME, this looks dangerous
  }

  if( m_databaseHandlers.empty() && m_inSession ) {
    m_storageSvc->endSession().ignore();
    m_inSession = false;
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
  if( m_databaseHandlers.empty() && m_inSession ) {
    m_storageSvc->endSession().ignore();
    m_inSession = false;
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

  if( m_inSession ) {
    ret = ret and m_storageSvc->endSession().isSuccess();
    m_inSession = false;
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
    if( !m_storageSvc->startSession(mode, m_technology).isSuccess() ) {
      return "";
    }
  }

  std::string fid = "";
  pool::FileDescriptor fd( pfn, pfn );
  // this is only a temporary FID so use a special pattern to make that clear
  fd.setFID( fd.FID().substr(0,24) + "0FF0FF0FF0FF" );
  if( m_storageSvc->existsConnection(fd).isSuccess() ) {
    if ( m_storageSvc->connect(pool::READ, fd).isSuccess() ) {
      pool::DatabaseConnection* connection = fd.dbc();
      DbDatabase dbH(static_cast<DbDatabaseObj*>(connection->handle()));
      if ( ! dbH.param( "FID", fid ).isSuccess() ) fid = "";
      m_storageSvc->disconnect( fd ).ignore();
    }
  }

  if( m_databaseHandlers.empty() ) {
    m_storageSvc->endSession().ignore();
    m_inSession = false;
  }

  return fid;
}

bool
pool::PersistencySvc::MicroSessionManager::attributeOfType( const std::string& attributeName,
                                                            void* data,
                                                            const std::type_info& typeInfo,
                                                            const std::string& option )
{
  if( !m_inSession ) {
      return false;
  }
  pool::DbOption domainOption( attributeName, option );
  if( !m_storageSvc->getDomainOption(domainOption).isSuccess() ) return false;
  return domainOption.i_getValue( typeInfo, data ).isSuccess();
}

bool
pool::PersistencySvc::MicroSessionManager::setAttributeOfType( const std::string& attributeName,
                                                               const void* data,
                                                               const std::type_info& typeInfo,
                                                               const std::string& option )
{
  if( !m_inSession ) {
      return false;
  }
  pool::DbOption domainOption( attributeName, option );
  if( !domainOption.i_setValue( typeInfo, const_cast<void*>( data ) ).isSuccess() ) return false;
  return m_storageSvc->setDomainOption(domainOption).isSuccess();
}
