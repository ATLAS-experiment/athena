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
#include "StorageSvc/DbContainer.h"
#include "StorageSvc/DbContainerImp.h"
#include "GaudiKernel/StatusCode.h"
#include "AthenaKernel/errorcheck.h"

#include <stdexcept>

using namespace std;
using namespace pool;

/// Standard Constructor
DbContainerImp::DbContainerImp(const std::string& name) :
  APRMessaging(name),
  m_size(0), m_name("UNKNOWN")
{
}

/// Standard Destructor
DbContainerImp::~DbContainerImp() {
}

/// Size of the container
uint64_t DbContainerImp::size()  {
  return m_size;
}

/// Number of next record in the container (=size if no delete is allowed)
uint64_t DbContainerImp::nextRecordId()   {
  return size();
}

/// Access options
StatusCode DbContainerImp::getOption(DbOption& /* opt */) {
  return StatusCode::FAILURE;
}

/// Set options
StatusCode DbContainerImp::setOption(const DbOption& /* opt */) {
  return StatusCode::SUCCESS;
}

/// Close the container and deallocate resources
StatusCode DbContainerImp::close()   {
  m_size = 0;
  return StatusCode::SUCCESS;
}

/// In place allocation of raw memory for the transient object
StatusCode DbContainerImp::store(const void* object, DbContainer& cntH, ShapeH shape)  {
  Token::OID_t objLink(cntH.token()->oid().first, nextRecordId());
  DbAction action( object, shape, objLink );
  return writeObject( action );
}

/// In place allocation of raw memory for the transient object
StatusCode DbContainerImp::allocate(DbContainer& cntH, const void* object, ShapeH shape, Token::OID_t& oid) {
  if ( object )  {
    oid.first  = cntH.token()->oid().first;
    oid.second = nextRecordId();
    if ( m_writeStack.size() < m_size+1 )  {
      m_writeStack.resize(m_size+1024);
    }
    m_writeStack[m_size] = DbAction( object, shape, oid );
    m_size++;
    return StatusCode::SUCCESS;
  }
  throw std::runtime_error("DbContainerImp::allocate failed: null object pointer");
}

/// Execute object modification requests during a transaction
StatusCode DbContainerImp::commitTransaction() {
  StatusCode iret   = StatusCode::SUCCESS;
  StatusCode status = StatusCode::SUCCESS;
  ActionList::iterator i = m_writeStack.begin();
  for(size_t j=0; j < m_size; ++j, ++i )  {
    status = writeObject(*i);
    if ( !status.isSuccess() ) {
      iret = status;
      ATH_MSG_ERROR("The Transaction cannot be committed..."
                    << " Container has " << size() << " Entries in total.");
    }
  }
  return iret;
}


/// Execute Database Transaction action
StatusCode DbContainerImp::transAct(Transaction::Action action)
{
  if( action==Transaction::TRANSACT_COMMIT || action==Transaction::TRANSACT_FLUSH ) {
    CHECK( commitTransaction() );
  }
  m_size = 0;
  return StatusCode::SUCCESS;
}

// Fetch next object address to set token
StatusCode DbContainerImp::next(Token::OID_t& linkH) {
   linkH.second++;
   if( linkH.second >= 0 && (uint64_t)linkH.second  < size() )  {
      return StatusCode::SUCCESS;
   }
   return StatusCode::FAILURE;
} 

// Read object (oid) from a container container (linkH)
StatusCode DbContainerImp::load( void** ptr, ShapeH shape,
                               const Token::OID_t& linkH, Token::OID_t& oid,
                               bool any_next )
{
   StatusCode sc = StatusCode::FAILURE;
   oid.second = linkH.second;
   if( any_next ) {
      while( (uint64_t)oid.second < size() ) {
	 oid.second = linkH.second;
         if( linkH.second >= 0 && (uint64_t)linkH.second < size() )  {
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
