/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "CollectionSvc/CollectionRowBuffer.h"
#include "CollectionSvc/CollectionDescription.h"
#include "CollectionSvc/CollectionColumn.h"
#include "PersistentDataModel/Token.h"


pool::CollectionRowBuffer::CollectionRowBuffer()
    : m_token( new Token ),
      m_attributeList( new coral::AttributeList )
{}


pool::CollectionRowBuffer::CollectionRowBuffer( coral::AttributeList& attributeList )
    : m_token( new Token ),
      m_attributeList( new coral::AttributeList )
{
   // share data
   m_attributeList->merge( attributeList );
}


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


pool::CollectionRowBuffer&
pool::CollectionRowBuffer::operator=( const pool::CollectionRowBuffer& rhs )
{
  rhs.token().setData( m_token );
  *m_attributeList = *rhs.m_attributeList;

  return *this;
}


bool
pool::CollectionRowBuffer::operator==( const pool::CollectionRowBuffer& rhs ) const
{
  return *m_attributeList == *rhs.m_attributeList;
}


bool
pool::CollectionRowBuffer::operator!=( const pool::CollectionRowBuffer& rhs ) const
{
  return ( ! ( *this == rhs ) );
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
  return CollectionDescription::tokenColumn().name();
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

