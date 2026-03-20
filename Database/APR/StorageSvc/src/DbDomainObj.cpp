/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

//====================================================================
//  DbDomainObj handle implementation
//--------------------------------------------------------------------
//
//  Package    : StorageSvc (The POOL project)
//
//  Description: Generic data persistency
//
//  @author      M.Frank
//====================================================================

#include "DbDatabaseObj.h"
#include "DbDomainObj.h"

// Framework include files
#include "StorageSvc/pool.h"
#include "StorageSvc/IDbDomain.h"
#include "StorageSvc/IOODatabase.h"

#include "GaudiKernel/StatusCode.h"
#include "AthenaKernel/errorcheck.h"

// C++ include files
#include <vector>
#include <ranges>

using namespace std;
using namespace pool;

/// Constructor
DbDomainObj::DbDomainObj(IOODatabase* imp, const DbType& typ, DbAccessMode mode)
: Base("Domain["+typ.storageName()+"]", mode, typ, imp),
  APRMessaging(name()),
  m_maxAge(2),
  m_info(0)
{
  if ( 0 == db() )    {
    ATH_MSG_ERROR( ">   Access   DbDomain     " << accessMode(mode)
        << " " << name() << " (UNKNOWN) impossible." << " [" << typ.storageName() << "]" );
    throw std::runtime_error("POOL::DbDomain: Unknown storage type requested: " + typ.storageName());
    return;
  }
  m_info = db()->createDomain();
  ATH_MSG_INFO( ">   Access   DbDomain     "
        << accessMode(mode) << " [" << type().storageName() << "]" );
}

/// Destructor
DbDomainObj::~DbDomainObj()  {
  clearEntries();
  deletePtr(m_info);
  ATH_MSG_INFO( ">   Deaccess DbDomain     "
      << accessMode(mode()) 
      << " [" << type().storageName() << "]" );
}

bool DbDomainObj::existsDbase( const string& name)
{  return (m_info) ? m_info->existsDbase( name ) : false;               }

StatusCode DbDomainObj::open(DbAccessMode mod) {
  setMode(mod);
  //  return m_info ? m_info->open(session(),name(),mode()) : FAILURE;
  return m_info ? StatusCode::SUCCESS : StatusCode::FAILURE;
}

StatusCode DbDomainObj::open()
{  return open( mode() );                                               }


StatusCode DbDomainObj::close()
{
  // temporary vector to avoid iterator invalidation by remove()
  vector<DbDatabaseObj*> dbs { views::values(*this).begin(), views::values(*this).end() };
  for( DbDatabaseObj* db : dbs )  {
    CHECK( db->close() );
    CHECK( remove(db) );
  }
  clearEntries();
  return StatusCode::SUCCESS;
}

/// Increase the age of all open databases
StatusCode DbDomainObj::ageOpenDbs() {
  for (iterator i = begin(); i != end(); ++i ) {
    DbDatabaseObj* pDB = (*i).second;
    DbAccessMode m  = pDB->mode();
    if( 0==(m&pool::CREATE) && 0==(m&pool::UPDATE) )  {
      pDB->setAge(1);
    }
  }
  return StatusCode::SUCCESS;
}

/// Check if databases are present, which aged a lot and need to be closed
StatusCode DbDomainObj::closeAgedDbs()  {
  vector<DbDatabaseObj*> aged_dbs;
  for (const_iterator i = begin(); i != end(); ++i ) {
    DbDatabaseObj* pDB = (*i).second;
    if ( pDB->age() > m_maxAge )   {
      DbAccessMode m  = pDB->mode();
      if( 0 == (m&pool::CREATE) && 0 == (m&pool::UPDATE) )  {
        aged_dbs.push_back(pDB);
      }
    }
  }
  vector<DbDatabaseObj*>::const_iterator j;
  for (j=aged_dbs.begin(); j != aged_dbs.end(); ++j)
  CHECK( (*j)->retire() );
  return StatusCode::SUCCESS;
}

/// Set domain specific options
StatusCode DbDomainObj::setOption(const DbOption& refOpt)  
{  return m_info ? m_info->setOption(refOpt) : StatusCode::FAILURE;                   }

/// Access domain specific options
StatusCode DbDomainObj::getOption(DbOption& refOpt) const
{  return m_info ? m_info->getOption(refOpt) : StatusCode::FAILURE;                   }
