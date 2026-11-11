/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "CollectionRowBuffer.h"
#include "CollectionColumn.h"
#include "PersistentDataModel/Token.h"


CollectionRowBuffer::CollectionRowBuffer()
    : m_token( new Token ),
      m_attributeList( new coral::AttributeList )
{}


CollectionRowBuffer::CollectionRowBuffer( const CollectionRowBuffer& rhs )
    : m_token( new Token ),
      m_attributeList( new coral::AttributeList )
{
  rhs.token().setData( m_token );
  // share the data
  m_attributeList->merge( *rhs.m_attributeList );
}


bool CollectionRowBuffer::deleteAL ATLAS_NOT_THREAD_SAFE()
{
  delete m_attributeList;  m_attributeList = nullptr;
  delete m_token; m_token = nullptr;
  return true;
}


CollectionRowBuffer::~CollectionRowBuffer()
{
  [[maybe_unused]] bool flag ATLAS_THREAD_SAFE = deleteAL();
}


void
CollectionRowBuffer::setAttributeList( const coral::AttributeList& attributeList )
{
  *m_attributeList = attributeList;
}


Token&
CollectionRowBuffer::token()
{
  return *m_token;
}

const Token&
CollectionRowBuffer::token() const
{
  return *m_token;
}


const std::string&
CollectionRowBuffer::tokenName() const
{
  return CollectionColumn::tokenColumn().name();
}


coral::AttributeList&
CollectionRowBuffer::attributeList()
{
  return *m_attributeList;
}


const coral::AttributeList&
CollectionRowBuffer::attributeList() const
{
  return *m_attributeList;
}

