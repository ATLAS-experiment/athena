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
#include <optional>
#include <set>
#include <stdexcept>
#include <string_view>
#include <vector>

// Scans the containers of the file and opens the ones that make up the collection:
//  - the EventTag columns: "Token" (addresses of the DataHeaders) and optional attributes
//  - the DataHeader container, which is used as the token source if there are no EventTag columns
Collection::Collection( FileDescriptor& fileDescr, Content content )
      : APRMessaging( "Collection")
{
   // the file must have been connected by the caller
   if( !fileDescr.dbc() ) {
      throw std::runtime_error( "Collection: file must already be connected: " + fileDescr.PFN() );
   }

   // all errors are thrown as exceptions
   ATH_MSG_VERBOSE( "Opening collection '" << fileDescr.PFN() << "' in READ mode" );

   // list all (non-internal) containers in the file
   std::vector<const Token*> containerTokens;
   DbDatabase db( fileDescr.dbc()->handle() );
   if( !db.containers(containerTokens, false).isSuccess() ) {
      throw std::runtime_error( "Collection: error reading " + fileDescr.PFN() );
   }

   // container names are configurable per file; EventTag columns are named "<prefix>(<column>)"
   // and the DataHeader container "<name>(DataHeader)" or, in older files, "<name>_DataHeader"
   const std::string containerPrefix = APRDefaults::ReadConfig::getEventTagName( fileDescr.FID() );
   const std::string dhName = APRDefaults::ReadConfig::getDataHeaderName( fileDescr.FID() );
   const std::string newDHContName = std::format("{}({})", dhName, APRDefaults::DataHeaderTypeName);
   const std::string oldDHContName = std::format("{}_{}", dhName, APRDefaults::DataHeaderTypeName);
   const std::string tagContName = containerPrefix + "(";
   ATH_MSG_DEBUG("Opening Collection '" << fileDescr.PFN() << "' using container prefix: " << containerPrefix );

   // opens a container of the file; warns and returns nothing if that fails
   auto openContainer = [&]( const std::string& contName, const Token& token, const DbTypeInfo* typ_info,
                             std::string_view kind ) -> std::optional<DbContainer> {
      DbContainer cnt( db.type() );
      if( cnt.open( db, contName, typ_info, token.technology(), db.openMode() ).isSuccess() ) {
         return cnt;
      }
      ATH_MSG_WARNING( kind << " container " << contName << " could not be opened" );
      return std::nullopt;
   };

   DbContainer tokenColumn;   // the EventTag Token column
   DbContainer dhCont;        // the DataHeader container
   // true if the file has EventTag columns, which then replace the DataHeader container as the token source
   bool hasTagColumns = false;
   // it seems merged files report multiple instances of the same container - avoid trying to process duplicates
   std::set<std::string> seenContainers;
   for( const Token *t : containerTokens ) {
      Token token(t);      // need a non-const Token
      const std::string contName = db.cntName(token);
      if( contName.starts_with( tagContName ) && !seenContainers.contains( contName ) ) {
         // EventTag column: the column name is the part inside the parentheses
         seenContainers.insert( contName );
         const std::string attrName = contName.substr( tagContName.size(), contName.size() - tagContName.size() - 1 );
         const bool isToken = attrName == APRDefaults::TokenTypeName;
         // attributes are not needed when only the tokens are requested
         if( content == Content::TokenOnly && !isToken ) {
            hasTagColumns = true;
            continue;
         }
         const DbTypeInfo* typ_info = db.objectShape( token.classID() );
         ATH_MSG_DEBUG("  :container " << contName << " with attribute " << attrName << " of type: " << typ_info->clazz().Name());
         auto cnt = openContainer( contName, token, typ_info, "EventTag" );
         if( !cnt ) {
            continue;
         }
         hasTagColumns = true;
         if( isToken ) {
            tokenColumn = *cnt;
         } else {
            // every other column is an attribute, stored in m_attributeList
            if( m_attributeContainers.contains(attrName) ) {
               throw std::runtime_error( "Column with name `" + attrName + "' already exists." );
            }
            m_attributeList.extend( attrName, typ_info->clazz().Name() );
            m_attributeContainers.emplace( attrName, *cnt );
         }
      } else if( contName == newDHContName || contName == oldDHContName ) {
         // DataHeader container, the token source if there are no EventTag columns
         ATH_MSG_DEBUG("  :container " << contName << " is the DataHeader container");
         if( auto cnt = openContainer( contName, token, db.objectShape( token.classID() ), "DataHeader" ) ) {
            dhCont = *cnt;
         }
      }
   }
   // without EventTag columns the DataHeader container itself provides the tokens
   m_tokenContainer = hasTagColumns ? tokenColumn : dhCont;
}


// Reads the next row: its attributes (if requested) and the token of its DataHeader.
// Returns false at the end of the collection or if reading fails.
bool Collection::next()
{
   if( m_nextRow >= size() ) {
      return false;
   }
   const std::size_t row = m_nextRow++;
   // the row is addressed by the container link of the token container and the row index
   Token cnt_token(m_tokenContainer.token());
   const Token::OID_t oid { cnt_token.oid().first, static_cast<std::int64_t>(row) };
   // read the attribute values of this row; the list is empty with Content::TokenOnly, so nothing is read then
   for( auto& attr : m_attributeList ) {
      DbContainer& container = m_attributeContainers.at( attr.specification().name() );
      void *obj = attr.addressOfData();
      if( !container.load( &obj, nullptr, oid ).isSuccess() ) {
         return false;
      }
   }

   const std::string newDHContName = std::format("{}(DataHeader)", APRDefaults::ReadConfig::getDataHeaderName( cnt_token.dbID().toString() ));
   const std::string oldDHContName = std::format("{}_DataHeader",  APRDefaults::ReadConfig::getDataHeaderName( cnt_token.dbID().toString() ));
   std::string tokenStr;
   if( cnt_token.contID() == newDHContName || cnt_token.contID() == oldDHContName ) {
      // the token container is the DataHeader container: this row's DataHeader is the entry at the row index
      cnt_token.setOid(oid);
      tokenStr = cnt_token.toString();
   } else {
      // the token container is the EventTag Token column: read the stored token string,
      // which may point to any DataHeader, also in another file
      void *token_addr = &tokenStr;
      if( !m_tokenContainer.load( &token_addr, nullptr, oid ).isSuccess() ) {
         return false;
      }
   }
   m_token.fromString( tokenStr );
   return true;
}


// The number of rows is the number of entries in the token container
std::size_t Collection::size()
{
   return m_tokenContainer.size();
}


// The next call to next() reads the requested row
bool Collection::seek(std::size_t position)
{
   if( position >= size() ) {
      return false;
   }
   m_nextRow = position;
   return true;
}

