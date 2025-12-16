/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

//====================================================================
//  DbContainerObj class implementation
//--------------------------------------------------------------------
//
//  Package    : StorageSvc (The POOL project)
//
//  Description: Generic data persistency
//
//  @author      M.Frank
//====================================================================

// Framework include files
#include "DbContainerObj.h"
#include "StorageSvc/IDbContainer.h"
#include "StorageSvc/DbDomain.h"
#include "StorageSvc/DbTypeInfo.h"
#include "StorageSvc/DbContainer.h"
#include <memory>
#include <stdexcept>
#include <atomic>
using namespace std;
using namespace pool;

int DbObjectHolder::release() {  return 1;    }

// Enable this to force regular retirement
void retireDatabase(DbContainerObj* c)  {
  static std::atomic<int> i=0;
  if ( (++i%2)==0 )  {
    c->database().setAge(20);
    c->database().containedIn().closeAgedDbs();
  }
}

/// Constructor
DbContainerObj::DbContainerObj( DbDatabase&       dbH,
                                const string&     nam, 
                                const DbType&     dbtyp,
                                DbAccessMode      mod)   
: Base(nam, mod, dbtyp, dbH.db()),
  APRMessaging( dbH.logon() ),
  m_info(0), m_tokH(0)
{
  m_isOpen    = false;

  if ( 0 != db() && dbtyp == dbH.type() )   {
    if ( dbH.isValid() )  {
      if ( dbH.add( name(), this).isSuccess() )    {
        m_dbH  = dbH;
        setMode(mod);
        if ( mod & pool::UPDATE ) {
          setMode(mod |= pool::CREATE);
        }
        ATH_MSG_DEBUG("--> Access   DbContainer  " 
            << accessMode(mode())
            << " [" << type().storageName() << "] " 
            << name() );
        return;
      }
    }
  }
  ATH_MSG_ERROR("--> Access   DbContainer  "
      << " Mode:" << accessMode(mode()) 
      << "  " << name()
      << " impossible."
      << " [" << type().storageName() << "] " );
  type().missingDriver(msg());
}

// Destructor
DbContainerObj::~DbContainerObj()     {
   string id = m_dbH.isValid() ? m_dbH.logon() : name();
   clearEntries();
   releasePtr(m_info);
   m_dbH.remove(this);
   ATH_MSG_DEBUG("--> Deaccess DbContainer  " 
      << accessMode(mode()) 
      << " [" << type().storageName() << "] " 
      << name());
}

// Check database access
bool DbContainerObj::hasAccess()    {
  if ( 0 == m_info )  {
    const DbTypeInfo* typ = m_dbH.contShape(name());
    if ( typ )  {
      if ( open(typ).isSuccess() )  {
        return true;
      }
      releasePtr(m_info);
    }
  }
  return m_info != 0;
}

// Retrieve container size
uint64_t DbContainerObj::size()   {
  if ( !hasAccess() )    {
    string id = database().isValid() ? database().logon() : name();
    ATH_MSG_ERROR("--> Access   DbContainer::size()" 
        << "  " << name()
        << " impossible - invalid object!");
     return -1;
   }
   database().setAge(0);
   return m_info->size();
}

/// Open Database container
DbStatus DbContainerObj::open(const DbTypeInfo* typ)   {
  if ( !m_isOpen )    {
    if ( 0 == m_info )  {
      m_info = db()->createContainer(name(), type());
    }
    if ( 0 != m_info && 0 != typ && database().isValid() )  {
      DbStatus sc = info()->open(database(), name(), typ, mode());
      if ( sc.isSuccess())  {
        if ( mode() != pool::READ )  {
          Token tok;
          tok.setDb(database().name());
          tok.setCont(name());
          tok.setTechnology(type().type());
          tok.setClassID(typ->shapeID());
          sc = database().makeLink(&tok, tok.oid());
          if ( !sc.isSuccess() )   {
            m_info->close();
            return sc;
          }
          sc = database().addShape(typ);
          if ( !sc.isSuccess() )  {
            m_info->close();
            return sc;
          }
        }
        m_tokH = database().cntToken(name());
        if ( m_tokH )    {
          m_isOpen = true;
          return Success;
        }
      }
      return sc;
    }
  }
  else if ( typ ) {
    return m_dbH.addShape(typ);
  }
  return Error;
}

/// Check if we can access the container
DbStatus DbContainerObj::checkAccess() {
  DbStatus result = Error;
  auto container = db()->createContainer(name(), type());
  if( database().isValid() && container ) {
    result = container->checkAccess(database(), name());
  }
  releasePtr(container);
  return result;
}

/// Close Database container
DbStatus DbContainerObj::close()   {
  if ( retire().isSuccess() )    {
    m_dbH.remove(this);
    return Success;
  }
  return Error;
}
/// Close Database container
DbStatus DbContainerObj::retire()   {
  if ( m_isOpen )    {
    if ( 0 != m_info )    {
      if ( m_info->close() )   {
        m_isOpen = false;
        releasePtr(m_info);
        return Success;
      }
      releasePtr(m_info);
    }
    m_isOpen = false;
    return Error;
  }
  return Success;
}

/// Execute Transaction Action
DbStatus DbContainerObj::transAct(Transaction::Action action) {
   return m_info?  m_info->transAct(action) : Success;
}

/// Pass options to the implementation
DbStatus DbContainerObj::setOption(const DbOption& refOpt) {
  return hasAccess() ? m_info->setOption(refOpt) : Error;
}

/// Access options
DbStatus DbContainerObj::getOption(DbOption& refOpt) {
  return hasAccess() ? m_info->getOption(refOpt) : Error;
}

/// Store object in location
DbStatus DbContainerObj::store(const void* object,
                               DbContainer& cntH,
                               ShapeH shape)
{
  if ( !isReadOnly() && hasAccess() )  {
    m_dbH.setAge(0);
    return m_info->store(object, cntH, shape);
  }
  return Error;
}

/// In place allocation of raw memory
DbStatus DbContainerObj::allocate(DbContainer& cntH,
                                  const void* object,
                                  ShapeH shape,
                                  Token::OID_t& oid)
{
  if ( isReadOnly() ) {
    throw std::runtime_error("DbContainerObj::allocate failed: container is read-only");
  }
  if ( !hasAccess() ) {
    throw std::runtime_error("DbContainerObj::allocate failed: no access to container");
  }
  if ( !object ) {
    throw std::runtime_error("DbContainerObj::allocate failed: null object pointer");
  }
  m_dbH.setAge(0);
  return m_info->allocate(cntH, object, shape, oid);
}

/// Retrieve persistent type information
const DbTypeInfo* DbContainerObj::objectShape(const Guid& guid) {
  return m_dbH.objectShape(guid);
}

/// Select object in the container identified by its handle
DbStatus DbContainerObj::load( void** ptr, ShapeH shape,
                               const Token::OID_t& linkH, 
                               Token::OID_t& oid,
                               bool any_next)
{
  if ( hasAccess() && m_isOpen )  {
    m_dbH.setAge(0);
    oid.first  = token()->oid().first;
    // Specific implementation may overwrite OID
    return m_info->load(ptr, shape, linkH, oid, any_next);
  }
  return Error;
}

/// Fetch next object address to set token
DbStatus DbContainerObj::next(Token::OID_t& linkH) {
  if ( hasAccess() )   {
    m_dbH.setAge(0);
    return m_info->next(linkH);
  }
  return Error;
}
