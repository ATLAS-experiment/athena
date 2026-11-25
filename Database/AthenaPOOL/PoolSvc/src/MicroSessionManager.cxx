/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MicroSessionManager.h"
#include "SessionTool.h"
#include "DatabaseHandler.h"

#include "StorageSvc/IStorageTool.h"
#include "StorageSvc/DbConnection.h"
#include "StorageSvc/DbDatabase.h"
#include "StorageSvc/DbOption.h"

#include "GaudiKernel/StatusCode.h"

#include <exception>

MicroSessionManager::MicroSessionManager(SessionTool& sessionTool, long technology, int ageLimit) :
        m_sessionTool( sessionTool ),
        m_storageTool( "DbStorageTool/StorageTool", &sessionTool ),
        m_technology( technology )
{
  if( !m_storageTool.retrieve().isSuccess()
      || !m_storageTool->startSession(m_sessionTool.openMode(), m_technology, ageLimit).isSuccess() ) {
    throw std::runtime_error( "PoolSvc could not initialize StorageTool " + m_storageTool.name() );
  }
}

MicroSessionManager::~MicroSessionManager() {
  this->disconnectAll();
  m_storageTool->endSession().ignore();
}

DatabaseHandler* MicroSessionManager::connect( Io::IoFlag mode,
                                               const std::string& fid,
                                               const std::string& pfn ) {
  if( mode == Io::INVALID ) return 0;
  DatabaseHandler* db = 0;
  try {
    db = new DatabaseHandler( m_storageTool, m_technology, fid, pfn, mode );
    m_sessionTool.registerDatabaseHandler( db );
    m_databaseHandlers.insert( db );
  } catch( const std::runtime_error& /* error */) {
    delete db;
    return nullptr;
  }
  return db;
}

void MicroSessionManager::disconnect( DatabaseHandler* database ) {
  std::set< DatabaseHandler* >::iterator idb = m_databaseHandlers.find( database );
  if ( idb != m_databaseHandlers.end() ) {
    m_sessionTool.deregisterDatabaseHandler( *idb );
    delete *idb;
    m_databaseHandlers.erase( idb );
  }
}

bool MicroSessionManager::disconnectAll() {
  bool ret = true;
  for ( std::set< DatabaseHandler* >::iterator idb = m_databaseHandlers.begin();
        idb != m_databaseHandlers.end(); ++idb ) {
    m_sessionTool.deregisterDatabaseHandler( *idb );
    ret = (*idb)->disconnectTransaction();
    delete *idb;
  }
  m_databaseHandlers.clear();
  return ret;
}

std::string MicroSessionManager::fidForPfn( const std::string& pfn ) {
  std::string fid = "";
  FileDescriptor fd( pfn, pfn );
  // this is only a temporary FID so use a special pattern to make that clear
  fd.setFID( fd.FID().substr(0,24) + "0FF0FF0FF0FF" );
  if ( m_storageTool->connect(Io::READ, fd).isSuccess() ) {
    DbDatabase dbH( fd.dbc()->handle() );
    if ( ! dbH.param( "FID", fid ).isSuccess() ) fid = "";
    m_storageTool->disconnect( fd ).ignore();
  }
  return fid;
}

bool MicroSessionManager::attributeOfType( const std::string& attributeName,
                                           void* data,
                                           const std::type_info& typeInfo,
                                           const std::string& option ) {
  DbOption domainOption( attributeName, option );
  if( !m_storageTool->getDomainOption(domainOption).isSuccess() ) return false;
  return domainOption.i_getValue(typeInfo, data).isSuccess();
}

bool MicroSessionManager::setAttributeOfType( const std::string& attributeName,
                                              const void* data,
                                              const std::type_info& typeInfo,
                                              const std::string& option ) {
  DbOption domainOption( attributeName, option );
  if( !domainOption.i_setValue( typeInfo, const_cast<void*>( data ) ).isSuccess() ) return false;
  return m_storageTool->setDomainOption(domainOption).isSuccess();
}
