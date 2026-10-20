/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "Collection.h"
#include "CollectionCursor.h"

#include "CoralBase/AttributeList.h"

#include "PersistentDataModel/Token.h"
#include "StorageSvc/APRDefaults.h"
#include "StorageSvc/DbDatabase.h"
#include "StorageSvc/DbConnection.h"
#include "StorageSvc/DbContainer.h"
#include "StorageSvc/DbTypeInfo.h"

#include "CollectionColumn.h"

#include <algorithm>
#include <format>
#include <set>
#include <stdexcept>
#include <utility>
#include <vector>

namespace pool {

   Collection::~Collection() = default;

   Collection::Collection( FileDescriptor& fileDescr )
      : APRMessaging( "Collection")
      {
         if( !fileDescr.dbc() ) {
            throw std::runtime_error( "Collection: file must already be connected: " + fileDescr.PFN() );
         }

         // all errors are thrown as exceptions
         ATH_MSG_VERBOSE( "Opening collection '" << fileDescr.PFN() << "' in READ mode" );

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
                  if( attrName != CollectionColumn::tokenColumn().name() ) {
                     const auto duplicate = std::find_if(
                        m_attributeColumns.begin(), m_attributeColumns.end(),
                        [&attrName]( const CollectionColumn& column ) {
                           return column.name() == attrName;
                        } );
                     if( duplicate != m_attributeColumns.end() ) {
                        throw std::runtime_error( "Column with name `" + attrName + "' already exists." );
                     }
                     m_attributeColumns.emplace_back( attrName, typ_info->clazz().Name() );
                  }
                  m_containerMap.emplace( attrName, std::move(cnt) );
               } else {
                  ATH_MSG_WARNING("EventTag container " << contName << " could not be opened");
               }
            } else if( contName == newDHContName || contName == oldDHContName ) {
               ATH_MSG_DEBUG("  :container " << contName << " is the DataHeader container");
               DbContainer cnt( db.type() );
               if( cnt.open( db, contName, typ_info, token.technology(), db.openMode() ).isSuccess() )  {
                  m_dhCont = std::move(cnt);
               } else {
                  ATH_MSG_WARNING("EventTag container " << contName << " could not be opened");
               }
            }
         }
      }


   std::unique_ptr<CollectionCursor> Collection::cursor() {
      if( m_containerMap.empty() ) {
         m_containerMap.emplace( APRDefaults::TokenTypeName, std::move(m_dhCont) );
      }
      pool::CollectionRowBuffer collectionRowBuffer;
      coral::AttributeList attributeList;
      for( const auto& attrCol : m_attributeColumns ) {
         attributeList.extend( attrCol.name(), attrCol.type() );
      }
      collectionRowBuffer.setAttributeList( attributeList );
      return std::make_unique<CollectionCursor>( collectionRowBuffer, m_containerMap);
   }

} //namespace pool
