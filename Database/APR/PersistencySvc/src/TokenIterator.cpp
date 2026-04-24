/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TokenIterator.h"
#include "StorageSvc/DbDatabase.h"
#include "StorageSvc/DbContainer.h"
#include "StorageSvc/DbConnection.h"
#include "StorageSvc/FileDescriptor.h"

#include "GaudiKernel/StatusCode.h"

#include <exception>

pool::TokenIterator::TokenIterator( FileDescriptor& fileDescriptor,
                                                    const std::string& containerName) :
  m_container( nullptr ), m_refToken ( nullptr )
{
   DbDatabase dbH( fileDescriptor.dbc()->handle() );
   if ( dbH.isValid() )  {
      m_refToken = new Token(dbH.cntToken(containerName));
      m_container = new DbContainer(m_refToken->technology());
   }
   if( !dbH.isValid() || !m_container->open(dbH, m_refToken->contID(), 0, m_refToken->technology(), pool::READ).isSuccess() ) {
      throw std::runtime_error( "Selection from " + fileDescriptor.PFN() + "(" + containerName + ") failed (APR: \" TokenIterator::TokenIterator() \" from \" PersistencySvc \")" );
   }
}

pool::TokenIterator::~TokenIterator()
{
  delete m_container; m_container = nullptr;
  m_refToken->release(); m_refToken = nullptr;
}


Token*
pool::TokenIterator::next()
{
  Token::OID_t linkH(m_refToken->oid());
  if( !m_container->next(linkH).isSuccess() ) return nullptr;
  m_refToken->oid() = linkH;
  m_refToken->addRef();
  return m_refToken;
}


std::size_t
pool::TokenIterator::size()
{
  return m_container->size();
}


bool
pool::TokenIterator::seek(std::size_t position)
{
  if( position >= size() ) return false;
  // go to position-1, so that the next call to next() will return the Token at <position>
  m_refToken->oid().second = int(position-1);
  return true;
}
