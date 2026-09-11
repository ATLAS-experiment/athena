/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MicroSessionManager.h"
#include "DatabaseRegistry.h"
#include "DatabaseHandler.h"

#include "StorageSvc/IStorageSvc.h"
#include "StorageSvc/DbConnection.h"
#include "StorageSvc/DbDatabase.h"
#include "StorageSvc/DbOption.h"

#include "GaudiKernel/StatusCode.h"
#include <exception>

pool::MicroSessionManager::MicroSessionManager( pool::DatabaseRegistry& registry,
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


pool::MicroSessionManager::~MicroSessionManager()
{
  this->disconnectAll();
  m_storageSvc->release();
}


bool
pool::MicroSessionManager::connect( Io::IoFlag mode, int ageLimit )
{
  if( !m_inSession ) {
    m_inSession = m_storageSvc->startSession(mode, m_technology, ageLimit).isSuccess();
  }
  return m_inSession;
}


pool::DatabaseHandler*
pool::MicroSessionManager::connect( Io::IoFlag mode,
                                                     const std::string& fid,
                                                     const std::string& pfn )
{
  if( mode == Io::INVALID ) return 0;
  if( m_databaseHandlers.empty() ) {
    if( !m_inSession ) {
      if( !m_storageSvc->startSession(mode, m_technology).isSuccess() ) {
        return nullptr;
      }
      m_inSession = true;
    }
  }

  pool::DatabaseHandler* db = 0;
  try {
    db = new pool::DatabaseHandler( *m_storageSvc,
                                    m_technology,
                                    fid,
                                    pfn,
                                    mode );
    m_registry.registerDatabaseHandler( db );
    m_databaseHandlers.insert( db );
  } catch( const std::runtime_error& /* error */) {
    delete db;
    m_storageSvc->endSession().ignore();
    m_inSession = false;
    return nullptr;
  }

  if( m_databaseHandlers.empty() && m_inSession ) {
    m_storageSvc->endSession().ignore();
    m_inSession = false;
  }
  return db;
}


void
pool::MicroSessionManager::disconnect( pool::DatabaseHandler* database )
{
  std::set< pool::DatabaseHandler* >::iterator idb = m_databaseHandlers.find( database );
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
pool::MicroSessionManager::disconnectAll()
{
  bool ret = true;
  for ( std::set< pool::DatabaseHandler* >::iterator idb = m_databaseHandlers.begin();
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

std::string
pool::MicroSessionManager::fidForPfn( const std::string& pfn )
{
  if ( m_databaseHandlers.empty() ) {
    Io::IoFlag mode = Io::READ;
    if( !m_storageSvc->startSession(mode, m_technology).isSuccess() ) {
      return "";
    }
  }

  std::string fid = "";
  pool::FileDescriptor fd( pfn, pfn );
  // this is only a temporary FID so use a special pattern to make that clear
  fd.setFID( fd.FID().substr(0,24) + "0FF0FF0FF0FF" );
  if( m_storageSvc->existsConnection(fd).isSuccess() ) {
    if ( m_storageSvc->connect(Io::READ, fd).isSuccess() ) {
      DbDatabase dbH( fd.dbc()->handle() );
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
pool::MicroSessionManager::attributeOfType( const std::string& attributeName,
                                                            void* data,
                                                            const std::type_info& typeInfo,
                                                            const std::string& option )
{
  if( !m_inSession ) {
      return false;
  }
  pool::DbOption domainOption( attributeName, option );
  if( !m_storageSvc->getDomainOption(domainOption).isSuccess() ) return false;
  return domainOption.i_getValue(typeInfo, data).isSuccess();
}

bool
pool::MicroSessionManager::setAttributeOfType( const std::string& attributeName,
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
