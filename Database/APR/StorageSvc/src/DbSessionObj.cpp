/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

//====================================================================
//  DbSessionObj object implementation
//--------------------------------------------------------------------
//
//  Package    : System (The POOL project)
//
//  Description: Generic data persistency
//
//  @author      M.Frank
//====================================================================
// Framework include files
#include "DbSessionObj.h"
#include "DbDomainObj.h"
#include "POOLCore/DbPrint.h"
#include "StorageSvc/IOODatabase.h"

#include "Gaudi/PluginService.h"

using namespace pool;

// Standard Constructor
DbSessionObj::DbSessionObj()
: Base("DbSession", pool::READ, POOL_StorageType),
  AthMessaging("DbSession")
{
}

// Standard Destructor
DbSessionObj::~DbSessionObj()  {
  clearEntries();
  for( auto& i : m_dbTypes )  releasePtr( i.second );
}

// Open session
DbStatus DbSessionObj::open()   {
  ATH_MSG_INFO( "    Open     DbSession" );
  return Success;
}

// close session
DbStatus DbSessionObj::close()   {
  ATH_MSG_INFO( "    Closed   DbSession" );
  return Success;
}

// Access different implementations
IOODatabase* DbSessionObj::db(const DbType& typ) {
  if( m_dbTypes[typ] == 0 ) {
    const std::string nam = typ.storageName();
    IOODatabase* imp = Gaudi::PluginService::Factory<IOODatabase*()>::create(nam).release();
    if( imp )  {
      m_dbTypes[typ] = imp;
    } else {
      ATH_MSG_FATAL( "Failed to load plugin for " << nam << " storage type" );
    }
  }
  return m_dbTypes[typ] ;
}
