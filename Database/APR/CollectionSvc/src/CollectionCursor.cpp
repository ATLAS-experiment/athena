/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "CoralBase/Attribute.h"

#include "CollectionCursor.h"
#include "CollectionSvc/CollectionColumn.h"

#include "StorageSvc/APRDefaults.h"
#include "StorageSvc/DbContainer.h"

using namespace pool;


CollectionCursor::CollectionCursor(
   const CollectionDescription& description,
   const CollectionRowBuffer& collectionRowBuffer,
   ContainerMap& containers
   )
   : m_description( description ),
     m_collectionRowBuffer( collectionRowBuffer ),
     m_attrContainers( containers ),
     m_tokenContainer( containers[description.tokenColumn().name()] ),
     m_idx( -1 )
{
}


bool CollectionCursor::next()
{
   if( ++m_idx >= size() ) {
      return false;
   }
   // read the row
   Token cnt_token(m_tokenContainer.token());
   Token::OID_t oid { cnt_token.oid().first, (std::int64_t)m_idx };
   for( auto& attr : m_collectionRowBuffer.attributeList() ) {
      const std::string& attr_name = attr.specification().name();
      DbContainer &container = m_attrContainers.at( attr_name );
      void *obj = attr.addressOfData();
      if( !container.load( &obj, nullptr, oid ).isSuccess() ) {
         return false;
      }
   }

   const std::string& newDHContName = std::format("{}(DataHeader)", APRDefaults::ReadConfig::getDataHeaderName( cnt_token.dbID().toString() ));
   const std::string& oldDHContName = std::format("{}_DataHeader",  APRDefaults::ReadConfig::getDataHeaderName( cnt_token.dbID().toString() ));
   if( cnt_token.contID() == newDHContName || cnt_token.contID() == oldDHContName ) {
      cnt_token.setOid(oid);
      m_tokenStr = cnt_token.toString();
   } else {
      // read the Token
      void *token_addr = &m_tokenStr;
      if( !m_tokenContainer.load( &token_addr, nullptr, oid ).isSuccess() ) {
         return false;
      }
   }
   m_collectionRowBuffer.token().fromString( m_tokenStr );
   return true;
}


const pool::CollectionRowBuffer&
CollectionCursor::currentRow() const
{
  return m_collectionRowBuffer;
}


std::size_t CollectionCursor::size()
{
  return m_tokenContainer.size();
}


bool CollectionCursor::seek(std::size_t position)
{
   if( position >= size() ) {
      return false;
   }
   m_idx = position-1;
   return true;
}


const Token& CollectionCursor::eventRef() const
{
   return m_collectionRowBuffer.token();
}
