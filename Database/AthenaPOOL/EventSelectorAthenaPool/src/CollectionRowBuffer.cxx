/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "CollectionRowBuffer.h"
#include "CollectionColumn.h"
#include "PersistentDataModel/Token.h"


pool::CollectionRowBuffer::CollectionRowBuffer()
    : m_token( new Token ),
      m_attributeList( new coral::AttributeList )
{}


pool::CollectionRowBuffer::CollectionRowBuffer( const pool::CollectionRowBuffer& rhs )
    : m_token( new Token ),
      m_attributeList( new coral::AttributeList )
{
  rhs.token().setData( m_token );
  // share the data
  m_attributeList->merge( *rhs.m_attributeList );
}


bool pool::CollectionRowBuffer::deleteAL ATLAS_NOT_THREAD_SAFE()
{
  delete m_attributeList;  m_attributeList = nullptr;
  delete m_token; m_token = nullptr;
  return true;
}


pool::CollectionRowBuffer::~CollectionRowBuffer()
{
  [[maybe_unused]] bool flag ATLAS_THREAD_SAFE = deleteAL();
}


void
pool::CollectionRowBuffer::setAttributeList( const coral::AttributeList& attributeList )
{
  *m_attributeList = attributeList;
}


Token&
pool::CollectionRowBuffer::token()
{
  return *m_token;
}

const Token&
pool::CollectionRowBuffer::token() const
{
  return *m_token;
}


const std::string&
pool::CollectionRowBuffer::tokenName() const
{
  return CollectionColumn::tokenColumn().name();
}


coral::AttributeList&
pool::CollectionRowBuffer::attributeList()
{
  return *m_attributeList;
}


const coral::AttributeList&
pool::CollectionRowBuffer::attributeList() const
{
  return *m_attributeList;
}

