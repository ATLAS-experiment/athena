/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

//====================================================================
//  DbContainerObj handle implementation
//--------------------------------------------------------------------
//
//  Package    : StorageSvc (The POOL project)
//
//  Description: Generic data persistency
//
//  @author      M.Frank
//====================================================================

// Framework include files
#include "StorageSvc/DbTypeInfo.h"
#include "StorageSvc/DbContainer.h"
#include "StorageSvc/DbToken.h"
#include "StorageSvc/DbReflex.h"
#include "DbContainerObj.h"
#include "CxxUtils/checker_macros.h"

#include <memory>
#include <stdexcept>

using namespace std;
using namespace pool;
static const string s_empty = "";

/// Add reference count to object if present
int DbContainer::refCount() const  {
  return isValid() ? ptr()->refCount() : int(INVALID);
}

// Open container from handle
DbStatus DbContainer::open( DbDatabase&  dbH, 
                            const string& nam, 
                            const DbTypeInfo*  typ, 
                            const DbType&      dbtyp,
                            DbAccessMode mod)
{
  if ( dbH.isValid() )  {
    if ( !(dbH.openMode() == pool::READ && mod != pool::READ) ) {
      DbContainerObj* q = dbH.find(nam);
      switchPtr(q ? q : new DbContainerObj(dbH, nam, dbtyp, mod));
      if ( mod == pool::READ || mod == pool::UPDATE )  {
        if ( 0 == typ )  {
          typ = dbH.contShape(nam);
        }
      }
      if ( ptr()->open(typ).isSuccess() )  {
        return Success;
      }
      close();
    }
  }
  return Error;
}

// Check if we can access the container for reading with the given type
DbStatus DbContainer::checkAccess(DbDatabase&  dbH,
                                  const string& nam,
                                  const DbType& dbtyp)
{
  DbStatus result = Error;
  if ( dbH.isValid() && dbH.openMode() == pool::READ ) {
    // ASM: Double check this implementation...
    DbContainerObj* q = dbH.find(nam);
    switchPtr(q ? q : new DbContainerObj(dbH, nam, dbtyp, pool::READ));
    result = ptr()->checkAccess();
    close();
  }
  return result;
}

void DbContainer::switchPtr(DbContainerObj* obj) {
  if (   obj ) obj->addRef();
  if ( m_ptr ) {
    if (m_ptr->release() == 0) m_ptr = 0;
  }
  m_ptr = obj;
  if ( m_ptr )  {
    m_type = obj->type();
  }
}

DbDatabase& DbContainer::containedIn() {
  if (!isValid()) std::abort();
  return m_ptr->database();
}

uint64_t DbContainer::size() {
  return isValid() ? m_ptr->size() : 0;
}

/// Access to access mode member
DbAccessMode DbContainer::openMode() const {
  return isValid() ? m_ptr->mode() : DbAccessMode(pool::NOT_OPEN);
}

/// Access to db name
const string& DbContainer::name() const {
  return isValid() ? m_ptr->name() : s_empty;
}

/// Close container object if handle is valid
DbStatus DbContainer::close() {
  if ( isValid() )  {
    DbStatus res = m_ptr->close();
    switchPtr(0);
    return res;
  }
  return Error;
}

/// Check if the container was opened
bool DbContainer::isOpen() const  {
  return isValid();
}

/// Execute Database Transaction Action
DbStatus DbContainer::transAct(Transaction::Action action) {
  return isValid() ? m_ptr->transAct(action) : Error;
}

/// Pass options to the implementation
DbStatus DbContainer::setOption(const DbOption& refOpt) {
  return isValid() ? m_ptr->setOption(refOpt) : Error;
}

/// Access options
DbStatus DbContainer::getOption(DbOption& refOpt) {
  return isValid() ? m_ptr->getOption(refOpt) : Error;
}

/// Start/Commit/Rollback Database Transaction
const Token* DbContainer::token() const {
  return isValid() ? m_ptr->token() : 0;
}

/// Access implementation internals
const IDbContainer* DbContainer::info() const {
  // Be sure to use the const version of info() to avoid checker warnings.
  return isValid() ? std::as_const(*m_ptr).info() : 0;
}
IDbContainer* DbContainer::info() {
  return isValid() ? m_ptr->info() : 0;
}

/// Retrieve persistent type information
const DbTypeInfo* 
DbContainer::objectShape(const Guid& guid) {
  return isValid() ? m_ptr->objectShape(guid) : 0;
}

/// Perform selection
DbStatus DbContainer::select(DbSelect& sel) {
  return isValid() ? m_ptr->select(sel) : Error;
}

/// Fetch next object address of the selection to set token
DbStatus DbContainer::fetch(DbSelect& sel) {
  return isValid() ? m_ptr->fetch(sel) : Error;
}

/// Store object in location
DbStatus DbContainer::store(const void* object, const DbTypeInfo* typ) {
  if ( isValid() )  {
    return m_ptr->store(object, *this, typ);
  }
  throw std::runtime_error("DbContainer::store failed: invalid container");
}

/// In place allocation of object location
DbStatus DbContainer::allocate(const void* object, ShapeH shape, Token::OID_t& oid) {
  if ( !isValid() ) {
    throw std::runtime_error("DbContainer::allocate failed: invalid container");
  }
  if ( !object ) {
    throw std::runtime_error("DbContainer::allocate failed: null object pointer");
  }
  return m_ptr->allocate(*this, object, shape, oid);
}

/// Load object in the container identified by its handle
DbStatus DbContainer::load( void** ptr,
                            ShapeH shape,
                            const Token::OID_t& linkH )
{
  if ( isValid() )  {
    Token::OID_t oid;
    DbStatus sc = m_ptr->load(ptr, shape, linkH, oid, false);
    return sc;
  }
  return Error;
}
