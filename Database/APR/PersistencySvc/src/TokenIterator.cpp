/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "TokenIterator.h"
#include "StorageSvc/DbDatabase.h"
#include "StorageSvc/DbContainer.h"
#include "StorageSvc/DbConnection.h"
#include "StorageSvc/FileDescriptor.h"

#include "GaudiKernel/StatusCode.h"

#include <exception>

pool::PersistencySvc::TokenIterator::TokenIterator( FileDescriptor& fileDescriptor,
                                                    const std::string& containerName) :
  m_container( nullptr ), m_refToken ( nullptr )
{
   pool::DatabaseConnection* connection = fileDescriptor.dbc();
   DbDatabase dbH(static_cast<DbDatabaseObj*>(connection->handle()));
   if ( dbH.isValid() )  {
      m_refToken = new Token(dbH.cntToken(containerName));
      m_container = new DbContainer(m_refToken->technology());
   }
   if( !dbH.isValid() || !m_container->open(dbH, m_refToken->contID(), 0, m_refToken->technology(), pool::READ).isSuccess() ) {
      throw std::runtime_error( "Selection from " + fileDescriptor.PFN() + "(" + containerName + ") failed (APR: \" TokenIterator::TokenIterator() \" from \" PersistencySvc \")" );
   }
}

pool::PersistencySvc::TokenIterator::~TokenIterator()
{
  delete m_container;
  delete m_refToken;
}


Token*
pool::PersistencySvc::TokenIterator::next()
{
  Token::OID_t linkH(m_refToken->oid());
  if ( ! m_container->next(linkH).isSuccess() ) return 0;
  m_refToken->oid() = linkH;
  return new Token(m_refToken); // FIXME, PvG: Think about a const version keeping ownership
}


std::size_t
pool::PersistencySvc::TokenIterator::size()
{
  return m_container->size();
}


bool
pool::PersistencySvc::TokenIterator::seek(std::size_t position)
{
  m_refToken->oid().second = int(position);
  return true;
}
