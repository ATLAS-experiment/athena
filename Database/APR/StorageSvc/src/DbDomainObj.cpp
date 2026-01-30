/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
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
#include "StorageSvc/DbSession.h"
#include "StorageSvc/IDbDomain.h"
#include "StorageSvc/IOODatabase.h"
#include "POOLCore/DbPrint.h"

#include "GaudiKernel/StatusCode.h"
#include "AthenaKernel/errorcheck.h"

// C++ include files
#include <vector>
#include <ranges>

using namespace std;
using namespace pool;

/// Constructor
DbDomainObj::DbDomainObj(DbSession& sessionH, 
                               const DbType& typ,
                               DbAccessMode mode)
: Base("Domain["+typ.storageName()+"]", mode, typ, sessionH.db(typ)),
  APRMessaging(name()),
  m_session(sessionH),
  m_maxAge(2),
  m_info(0)
{
  if ( 0 == db() )    {
    ATH_MSG_ERROR( ">   Access   DbDomain     " << accessMode(mode)
        << " " << name() << " (UNKNOWN) impossible." << " [" << typ.storageName() << "]" );
    type().missingDriver(msg());
    return;
  }
  m_info = db()->createDomain();
  if ( !m_session.add( this ).isSuccess() )    {
    ATH_MSG_ERROR( ">   Access   DbDomain     " 
        << accessMode(mode) << " " << name() << " (" << db()->name() << ")"
        << " impossible. Error inserting domain!" );
    return;
  }
  ATH_MSG_INFO( ">   Access   DbDomain     "
        << accessMode(mode) << " [" << type().storageName() << "]" );
}

/// Destructor
DbDomainObj::~DbDomainObj()  {
  clearEntries();
  if ( m_session.isValid() )    {
    m_session.remove(this).ignore();
  }
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

StatusCode DbDomainObj::close()   {
  if ( m_session.isValid() ) {
    // temporary vector to avoid iterator invalidation by remove()
    vector<DbDatabaseObj*> dbs { views::values(*this).begin(), views::values(*this).end() };
    for( DbDatabaseObj* db : dbs )  {
      CHECK( db->close() );
      CHECK( remove(db) );
    }
    clearEntries();
    CHECK( m_session.remove(this) );
    m_session = DbSession(0);
    return StatusCode::SUCCESS;
  }
  return StatusCode::FAILURE;
}

/// Increase the age of all open databases
StatusCode DbDomainObj::ageOpenDbs() {
  if ( m_session.isValid() )    {
    for (iterator i = begin(); i != end(); ++i ) {
      DbDatabaseObj* pDB = (*i).second;
      DbAccessMode m  = pDB->mode();
      if( 0==(m&pool::CREATE) && 0==(m&pool::UPDATE) )  {
        pDB->setAge(1);
      }
    }
    return StatusCode::SUCCESS;
  }
  return StatusCode::FAILURE;
}

/// Check if databases are present, which aged a lot and need to be closed
StatusCode DbDomainObj::closeAgedDbs()  {
  if ( m_session.isValid() )    {
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
  return StatusCode::FAILURE;
}

/// Set domain specific options
StatusCode DbDomainObj::setOption(const DbOption& refOpt)  
{  return m_info ? m_info->setOption(refOpt) : StatusCode::FAILURE;                   }

/// Access domain specific options
StatusCode DbDomainObj::getOption(DbOption& refOpt) const
{  return m_info ? m_info->getOption(refOpt) : StatusCode::FAILURE;                   }
