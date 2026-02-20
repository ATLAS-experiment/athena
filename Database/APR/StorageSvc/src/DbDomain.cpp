/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

//====================================================================
//  DbDomainObj handle implementation
//--------------------------------------------------------------------
//
//  Package    : StorageSvc  (The POOL project)
//  @author      M.Frank
//====================================================================

// Framework include files
#include "StorageSvc/DbDomain.h"
#include "StorageSvc/DbSession.h"
#include "DbDatabaseObj.h"
#include "DbDomainObj.h"
#include "CxxUtils/checker_macros.h"

using namespace std;
using namespace pool;

static const string s_empty = "";

/// Object constructor
DbDomain::DbDomain(DbDomainObj* dom)   {
  switchPtr( dom );
  if ( dom ) setType(dom->type());
}

StatusCode DbDomain::open(DbSession& sesH,const DbType& typ,DbAccessMode mod)  {
  if ( !isValid() )   {
    if ( sesH.isValid() )    {
      DbType db_typ(typ.majorType());
      DbDomainObj* dom = sesH.find(db_typ);
      if ( !dom )   {
        dom = new DbDomainObj(sesH, db_typ, mod);
      }
      switchPtr(dom);
      return dom->open(mod);
    }
    close().ignore();
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}

StatusCode DbDomain::close() {
  StatusCode rc = StatusCode::SUCCESS;
  if ( isValid() )    {
    rc = ptr()->close();
    switchPtr(0);
  }
  return rc;
}

/// Assign transient object properly (including reference counting)
void DbDomain::switchPtr(DbDomainObj* obj) {
   if( obj ) obj->addRef();
   if( isValid() ) {
      if (ptr()->release() == 0) {
         setPtr(0);
         setType(DbType(0));
      }
   }
   if( obj )  {
      setPtr(obj);
      setType(obj->type());
   } else {
      setPtr(0);
      setType(DbType(0));
   }
}

/// Add domain to session
StatusCode DbDomain::add(const string& nam, DbDatabaseObj* dbH) {
  StatusCode sc = StatusCode::FAILURE;
  if ( isValid() && dbH )    {
    sc = ptr()->add(nam, dbH);
    if ( sc.isSuccess() )  {
      ptr()->addRef();
    }
  }
  return sc;
}

/// Find domain in session
StatusCode DbDomain::remove(DbDatabaseObj* dbH) {
  StatusCode sc = StatusCode::FAILURE;
  if ( isValid() && dbH )    {
    sc = ptr()->remove(dbH);
    if ( isValid() && sc.isSuccess() )  {
      if (ptr()->release() == 0) {
        setPtr(0);
        setType(DbType(0));
      }
    }
  }
  return sc;
}

/// Check for the existence of a domain within a session
bool DbDomain::exist(DbSession& sH, int typ) const
{  return sH.find(typ) != 0;                                            }

DbSession DbDomain::containedIn() const 
{  return isValid() ? ptr()->session() : DbSession();                   }

/// Access to access mode
DbAccessMode DbDomain::openMode() const
{  return isValid() ? ptr()->mode() : DbAccessMode(pool::NOT_OPEN);     }

/// Access to domain name
const string& DbDomain::name() const
{  return isValid() ? ptr()->name() : s_empty;                          }

/// Add reference count to object if present
int DbDomain::refCount() const
{  return isValid() ? ptr()->refCount() : int(INVALID);                 }

/// Find domain in session
const DbDatabaseObj* DbDomain::find(const string& db_name) const 
{  return isValid() ? ptr()->find(db_name) : 0;                         }

DbDatabaseObj* DbDomain::find(const string& db_name) 
{  return isValid() ? ptr()->find(db_name) : 0;                         }

/// Check if Database exists within the domain
bool DbDomain::existsDbase(const string& nam)
{  return isValid() ? ptr()->existsDbase(nam) : false;                  }

/// Access implementation internals
IDbDomain* DbDomain::info()
{  return isValid() ? ptr()->info() : 0;                                }

const IDbDomain* DbDomain::info()   const
{  return isValid() ? ptr()->info() : 0;                                }

/// Allow access to the Database implementation
IOODatabase* DbDomain::db()
{  return isValid() ? ptr()->db() : 0;                                  }

const IOODatabase* DbDomain::db()  const
{  return isValid() ? ptr()->db() : 0;                                  }

/// Increase the age of all open databases
StatusCode DbDomain::ageOpenDbs()
{  return isValid() ? ptr()->ageOpenDbs() : StatusCode::FAILURE;                    }

/// Check if databases are present, which aged a lot and need to be closed
StatusCode DbDomain::closeAgedDbs()
{  return isValid() ? ptr()->closeAgedDbs() : StatusCode::FAILURE;                  }

/// Set the maximal allowed age limit for files in this domain
void DbDomain::setAgeLimit(int value)
{  if ( isValid() ) ptr()->setAgeLimit(value);                          }

/// Access the maximal age limit
int DbDomain::ageLimit()  const
{  return isValid() ? ptr()->ageLimit() : -1;                           }

/// Set domain specific options
StatusCode DbDomain::setOption(const DbOption& refOpt)
{  return isValid() ? ptr()->setOption(refOpt) : StatusCode::FAILURE;               }

/// Access domain specific options
StatusCode DbDomain::getOption(DbOption& refOpt) const
{  return isValid() ? ptr()->getOption(refOpt) : StatusCode::FAILURE;               }
