/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "ImplicitCollectionIterator.h"
#include "CollectionSvc/CollectionDescription.h"
#include "CollectionSvc/TokenList.h"

#include "PersistencySvc/IContainer.h"
#include "PersistencySvc/ITokenIterator.h"

#include "PersistentDataModel/Token.h"

pool::ImplicitCollectionIterator::
ImplicitCollectionIterator( pool::IContainer& container,
                            const pool::CollectionDescription& description )
      :
      m_container( container ),
      m_tokenIterator( m_container.tokens() ),
      m_token( 0 )
{
   TokenList        tokenList;
   tokenList.extend( description.eventReferenceColumnName() );
   m_rowBuffer.setTokenList( tokenList );
}


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
   return (m_token!=0);
}

Token*
pool::ImplicitCollectionIterator::token() const
{
  return m_token;
}

const pool::CollectionRowBuffer&
pool::ImplicitCollectionIterator::currentRow() const
{
   m_token->setData( &*m_rowBuffer.tokenList().begin() );
   return m_rowBuffer;
}


bool
pool::ImplicitCollectionIterator::seek(std::size_t position)
{
  // We'll have to do a next() to read the event.
  // So subtract one here to compensate for that.
  return m_tokenIterator->seek(position - 1);
}


std::size_t
pool::ImplicitCollectionIterator::size()
{
  return m_tokenIterator->size();
}
