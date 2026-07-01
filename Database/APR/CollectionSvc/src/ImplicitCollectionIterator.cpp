/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "ImplicitCollectionIterator.h"
#include "CollectionSvc/CollectionDescription.h"

#include "PersistencySvc/IContainer.h"
#include "PersistencySvc/ITokenIterator.h"

#include "PersistentDataModel/Token.h"

pool::ImplicitCollectionIterator::
ImplicitCollectionIterator( pool::IContainer& container )
      : m_container( container ),
        m_tokenIterator( m_container.tokens() ),
        m_token( 0 )
{ }


pool::ImplicitCollectionIterator::~ImplicitCollectionIterator()
{
  if ( m_token ) m_token->release();
}


bool
pool::ImplicitCollectionIterator::next()
{
  if( m_token ) {
     m_token->release();
     m_token = 0;
  }
  if( m_tokenIterator )
     m_token = m_tokenIterator->next();
  return m_token != nullptr;
}


Token*
pool::ImplicitCollectionIterator::token() const
{
  return m_token;
}

const pool::CollectionRowBuffer&
pool::ImplicitCollectionIterator::currentRow() const
{
   if (m_token)[[likely]]{
     m_token->setData( &m_rowBuffer.token() );
   }
   return m_rowBuffer;
}


bool
pool::ImplicitCollectionIterator::seek(std::size_t position)
{
  if( m_token ) {
    m_token->release();
    m_token = nullptr;
  }
  return m_tokenIterator->seek(position);
}


std::size_t
pool::ImplicitCollectionIterator::size()
{
  return m_tokenIterator->size();
}
