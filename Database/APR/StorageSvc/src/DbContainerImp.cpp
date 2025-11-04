/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

//====================================================================
//
//  Package    : StorageSvc (The POOL project)
//
//  @author      M.Frank
//
//====================================================================

/// Framework include files
#include "StorageSvc/DbSelect.h"
#include "StorageSvc/DbContainer.h"
#include "StorageSvc/DbContainerImp.h"
#include "StorageSvc/DbHeap.h"

using namespace std;
using namespace pool;

/// Standard Constructor
DbContainerImp::DbContainerImp(const std::string& name) :
  APRMessaging(name),
  m_size(0), m_writeSize(0), m_name("UNKNOWN"),
  m_canUpdate(false),
  m_canDestroy(false)
{
  m_stackType = NONE;
}

/// Standard Destructor
DbContainerImp::~DbContainerImp() {
  m_stack.clear();
}

/// Size of the container
uint64_t DbContainerImp::size()  {
  return m_writeSize;
}

/// Number of next record in the container (=size if no delete is allowed)
uint64_t DbContainerImp::nextRecordId()   {
  return size();
}

/// Access options
DbStatus DbContainerImp::getOption(DbOption& /* opt */) {
  return Error;  
}

/// Set options
DbStatus DbContainerImp::setOption(const DbOption& /* opt */){ 
  return Success;
}

/// Close the container and deallocate resources
DbStatus DbContainerImp::close()   {
  return clearStack();
}

/// In place allocation of raw memory for the transient object
void* DbContainerImp::allocate(unsigned long siz, DbContainer& cntH, ShapeH shape)  {
  DbObjectHandle<DbObject> objH(cntH.type());
  Token::OID_t objLink(cntH.token()->oid().first, nextRecordId());
  DbHeap::allocate(siz, &cntH, &objLink, &objH);
  if ( m_stack.size() < m_size+1 )  {
    m_stack.resize(m_size+1024);
  }
  m_stack[m_size] = DbAction( objH.ptr(), shape, objLink, pool::WRITE );
  m_stackType |= pool::WRITE;
  m_writeSize++;
  m_size++;
  return objH.ptr();
}

/// In place allocation of raw memory for the transient object
DbStatus DbContainerImp::allocate(DbContainer& cntH, const void* object, ShapeH shape, Token::OID_t& oid) {
  if ( object )  {
    oid.first  = cntH.token()->oid().first;
    oid.second = nextRecordId();
    if ( m_stack.size() < m_size+1 )  {
      m_stack.resize(m_size+1024);
    }
    m_stack[m_size] = DbAction( object, shape, oid, WRITE );
    m_stackType |= pool::WRITE;
    m_writeSize++;
    m_size++;
    return Success;
  }
  throw bad_alloc();
}

/// In place deletion of raw memory
DbStatus DbContainerImp::free(void* ptr, DbContainer& cntH) {
  return DbHeap::free(ptr, &cntH);
}

/// Reset action list
DbStatus DbContainerImp::clearStack()   {
  m_size = 0;
  m_writeSize = 0;
  m_stackType = NONE;
  return Success;
}

/// Execute object modification requests during a transaction
DbStatus DbContainerImp::commitTransaction() {
  DbStatus iret   = Success;
  DbStatus status = Success;
  ActionList::iterator i = m_stack.begin();
  for(size_t j=0; j < m_size; ++j, ++i )  {
    switch( (*i).action )  {
      case pool::WRITE:
        status = writeObject(*i);
        break;
      default:
        status = Error;
        break;
    }
    if ( !status.isSuccess() ) {
      iret = status;
      ATH_MSG_ERROR("The Transaction cannot be committed..."
                    << " Container has " << size() << " Entries in total.");
      break;
    }
  }
  return iret;
}


/// Execute Database Transaction action
DbStatus DbContainerImp::transAct(Transaction::Action action)
{
  DbStatus status = Success;
  if( action==Transaction::TRANSACT_COMMIT || action==Transaction::TRANSACT_FLUSH ) {
     status = commitTransaction();
  }
  clearStack();
  return status;
}

DbStatus 
DbContainerImp::save(DbObjectHandle<DbObject>& objH)  {
  // Can only be done if no Transaction is ongoing...
  // i.e. exactly one object was allocated
  if ( m_writeSize == 1 )   {
     if ( m_stack.begin()->object == objH.ptr() )   {
        objH.oid() = m_stack.begin()->link;
        DbStatus status = writeObject( *m_stack.begin() );
        clearStack();
        return status;
     }
  }
  return Error;
}

DbStatus
DbContainerImp::save(DbContainer& /* cntH */, const void* object, ShapeH shape, Token::OID_t& linkH)
{
  // Only possible if no open transaction, i.e. No object was allocated
  if ( m_stack.empty() )  {
     DbAction act(object, shape, linkH, WRITE);
     return writeObject( act );
  }
  return Error;
}

// Fetch next object address of the selection to set token
DbStatus DbContainerImp::fetch(DbSelect& sel) {
   Token::OID_t lnk = sel.link();
   while( (uint64_t)lnk.second < size() ) {
      if( fetch(lnk, lnk).isSuccess() )  {
         sel.link() = lnk;
         return Success;
      }
      lnk.second++;
   }
   return Error;
} 

// Fetch refined object address. Default implementation returns identity
DbStatus DbContainerImp::fetch(const Token::OID_t& linkH, Token::OID_t& stmt)  {
   stmt.second = linkH.second;
   return linkH.second >= 0 && (uint64_t)linkH.second < size() ? Success : Error;
}


// Read object (oid) from a container container (linkH)
DbStatus DbContainerImp::load( void** ptr, ShapeH shape, 
                               const Token::OID_t& linkH, Token::OID_t& oid,
                               bool any_next )
{
   DbStatus sc = Error;
   oid.second = linkH.second;
   if( any_next ) {
      while( (uint64_t)oid.second < size() ) {
         sc = fetch(linkH, oid);
         if( sc.isSuccess() )  {
            sc = loadObject(ptr, shape, oid);
            if( sc.isSuccess() )  {
               return sc;
            }
         }
         oid.second++;
      }
      if( linkH.second < 0 || (uint64_t)linkH.second <= size() ) {
         ATH_MSG_DEBUG("No objects passing selection criteria..."
                       << " Container has " << size() << " Entries in total.");
      }
   }
   else {
      sc = loadObject(ptr, shape, oid);
   }
   return sc;
}
