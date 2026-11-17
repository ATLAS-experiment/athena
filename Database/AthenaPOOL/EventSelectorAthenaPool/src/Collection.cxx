/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "Collection.h"

#include "CoralBase/Attribute.h"

#include "PersistentDataModel/Token.h"
#include "StorageSvc/APRDefaults.h"
#include "StorageSvc/DbDatabase.h"
#include "StorageSvc/DbConnection.h"
#include "StorageSvc/DbContainer.h"
#include "StorageSvc/DbTypeInfo.h"

#include <cstdint>
#include <format>
#include <set>
#include <stdexcept>
#include <utility>
#include <vector>

Collection::Collection( FileDescriptor& fileDescr )
      : APRMessaging( "Collection"), m_idx(-1)
{
         if( !fileDescr.dbc() ) {
            throw std::runtime_error( "Collection: file must already be connected: " + fileDescr.PFN() );
         }

         // all errors are thrown as exceptions
         ATH_MSG_VERBOSE( "Opening collection '" << fileDescr.PFN() << "' in READ mode" );

         DbContainer dhCont;
         std::vector<const Token*> containerTokens;
         DbDatabase db( fileDescr.dbc()->handle() );
         if( !db.containers(containerTokens, false).isSuccess() ) {
            throw std::runtime_error( "Collection: error reading " + fileDescr.PFN() );
         }
         const std::string containerPrefix = APRDefaults::ReadConfig::getEventTagName( fileDescr.FID() );
         const std::string dhName = APRDefaults::ReadConfig::getDataHeaderName( fileDescr.FID() );
         const std::string newDHContName = std::format("{}({})", dhName, APRDefaults::DataHeaderTypeName);
         const std::string oldDHContName = std::format("{}_{}", dhName, APRDefaults::DataHeaderTypeName);
         ATH_MSG_DEBUG("Opening Collection '" << fileDescr.PFN() << "' using container prefix: " << containerPrefix );
         std::string tagContName = containerPrefix + "(";
         // it seems merged files report multiple instances of the same container - avoid trying to process duplicates
         std::set<std::string> seenContainers;
         for( const Token *t : containerTokens ) {
            Token token(t);      // need a non-const Token
            const std::string& contName = db.cntName(token);
            const DbTypeInfo* typ_info = db.objectShape( token.classID() );
            if( contName.starts_with( tagContName ) and !seenContainers.contains( contName ) ) {
               seenContainers.insert( contName );
               const std::string& attrName = contName.substr( tagContName.size(), contName.size() - tagContName.size() - 1 );
               ATH_MSG_DEBUG("  :container " << contName << " with attribute " << attrName << " of type: " << typ_info->clazz().Name());
               DbContainer cnt( db.type() );
               if( cnt.open( db, contName, typ_info, token.technology(), db.openMode() ).isSuccess() )  {
                  if( attrName != APRDefaults::TokenTypeName ) {
                     if( m_containerMap.contains(attrName) ) {
                        throw std::runtime_error( "Column with name `" + attrName + "' already exists." );
                     }
                     m_attributeList.extend( attrName, typ_info->clazz().Name() );
                  }
                  m_containerMap.emplace( attrName, std::move(cnt) );
               } else {
                  ATH_MSG_WARNING("EventTag container " << contName << " could not be opened");
               }
            } else if( contName == newDHContName || contName == oldDHContName ) {
               ATH_MSG_DEBUG("  :container " << contName << " is the DataHeader container");
               DbContainer cnt( db.type() );
               if( cnt.open( db, contName, typ_info, token.technology(), db.openMode() ).isSuccess() )  {
                  dhCont = std::move(cnt);
               } else {
                  ATH_MSG_WARNING("EventTag container " << contName << " could not be opened");
               }
            }
         }
         if( m_containerMap.empty() ) {
            m_containerMap.emplace( APRDefaults::TokenTypeName, std::move(dhCont) );
         }
         m_tokenContainer = m_containerMap[APRDefaults::TokenTypeName];
}


bool Collection::next()
{
   if( ++m_idx >= size() ) {
      return false;
   }
   // read the row
   Token cnt_token(m_tokenContainer.token());
   Token::OID_t oid { cnt_token.oid().first, (std::int64_t)m_idx };
   for( auto& attr : m_attributeList ) {
      const std::string& attr_name = attr.specification().name();
      DbContainer &container = m_containerMap.at( attr_name );
      void *obj = attr.addressOfData();
      if( !container.load( &obj, nullptr, oid ).isSuccess() ) {
         return false;
      }
   }

   const std::string& newDHContName = std::format("{}(DataHeader)", APRDefaults::ReadConfig::getDataHeaderName( cnt_token.dbID().toString() ));
   const std::string& oldDHContName = std::format("{}_DataHeader",  APRDefaults::ReadConfig::getDataHeaderName( cnt_token.dbID().toString() ));
   std::string tokenStr;
   if( cnt_token.contID() == newDHContName || cnt_token.contID() == oldDHContName ) {
      cnt_token.setOid(oid);
      tokenStr = cnt_token.toString();
   } else {
      // read the Token
      void *token_addr = &tokenStr;
      if( !m_tokenContainer.load( &token_addr, nullptr, oid ).isSuccess() ) {
         return false;
      }
   }
   m_token.fromString( tokenStr );
   return true;
}


std::size_t Collection::size()
{
   return m_tokenContainer.size();
}


bool Collection::seek(std::size_t position)
{
   if( position >= size() ) {
      return false;
   }
   m_idx = position-1;
   return true;
}

