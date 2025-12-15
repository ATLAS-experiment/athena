/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "RootCollection.h"
#include "RootCollection/AttributeListLayout.h"

#include "CollectionSvc/CollectionDescription.h"
#include "CollectionSvc/CollectionColumn.h"

#include "POOLCore/DbPrint.h"
#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeList.h"

ClassImp(AttributeListLayout)
using namespace pool;
using namespace std;
   
AttributeListLayout::AttributeListLayout()
{}

AttributeListLayout::~AttributeListLayout()
{}

AttributeListLayout::AttributeListLayout( const pool::ICollectionDescription& desc )
{
   for( int col_id = 0; col_id < desc.numberOfTokenColumns(); col_id++ ) {
         const ICollectionColumn&        column = desc.tokenColumn(col_id);
         m_layout.push_back( make_pair( column.name(), column.type() ) );
   }

   for( int col_id = 0; col_id < desc.numberOfAttributeColumns(); col_id++ ) {
         const ICollectionColumn& column = desc.attributeColumn(col_id);
         m_layout.push_back( make_pair( column.name(), column.type() ) );
   }
   m_eventRefColumnName = desc.eventReferenceColumnName();
}


void AttributeListLayout::fillDescription( pool::CollectionDescription& desc )
{
   if( m_eventRefColumnName.empty() ) {
      m_eventRefColumnName = RootCollection::RootCollection::c_tokenBranchName;
   }
   desc.setEventReferenceColumnName( m_eventRefColumnName );
      
   for( size_t i = 0; i < m_layout.size(); i++ ) {
      std::string column_name = m_layout[i].first;
      if( !i && column_name == "EVENT_REFERENCE" ) {
         // apparently some older collections stored "EVENT_REFERENCE" in the description
         column_name = m_eventRefColumnName;
      }
      desc.insertColumn( column_name, m_layout[i].second );
   }
}


void AttributeListLayout::print() const {
  DbPrint log( "AttributeListLayout" );
  for(auto iter=m_layout.begin(); iter!=m_layout.end(); ++iter) {
     log << MSG::INFO << iter->first << " \t" << iter->second << endmsg;
  }
}
