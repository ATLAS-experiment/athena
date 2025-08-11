/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "RNTCollectionQuery.h"
#include "RNTCollectionCursor.h"

#include "CollectionBase/ICollectionDescription.h"
#include "CollectionBase/TokenList.h"
#include "CollectionBase/CollectionRowBuffer.h"
#include "CollectionBase/ICollectionCursor.h"
#include "CollectionBase/ICollectionColumn.h"
#include "CollectionBase/CollectionBaseNames.h"
#include "CollectionBase/boost_tokenizer_headers.h"

#include "CoralBase/AttributeList.h"

#include <ROOT/RNTuple.hxx>
#include <ROOT/RNTupleModel.hxx>

using namespace pool::RootCollection;


RNTCollectionQuery::RNTCollectionQuery( const pool::ICollectionDescription& description, 
                                        ROOT::RNTupleReader *reader ) :
   AthMessaging(std::string("RNTCollectionQuery[") + description.name() + "]"),
   m_description( description ),
   m_reader( reader ),
   m_cursor( 0 ),
   m_skipEventRef( false )
{
}


RNTCollectionQuery::~RNTCollectionQuery()
{
   delete m_cursor;   m_cursor = 0;
}


void RNTCollectionQuery::selectAllAttributes()
{
   for( int j = 0; j < m_description.numberOfAttributeColumns(); j++ )    {
      addToAttributeOutputList( m_description.attributeColumn( j ).name() );
   }
}


void RNTCollectionQuery::selectAllTokens()
{
   for( int j = 0; j < m_description.numberOfTokenColumns(); j++ )    {
      addToTokenOutputList( m_description.tokenColumn( j ).name() );
   }
}


void RNTCollectionQuery::selectAll()
{
   selectAllAttributes();
   selectAllTokens();
}


pool::ICollectionCursor&  RNTCollectionQuery::execute()
{
   if( !m_skipEventRef && m_description.hasEventReferenceColumn() )  {
      addToTokenOutputList( m_description.eventReferenceColumnName() );
   } 
  // Create collection row buffer to contain query output.
  pool::CollectionRowBuffer collectionRowBuffer( m_outputTokenList, m_outputAttributeList );

  m_cursor = new RNTCollectionCursor( m_description, collectionRowBuffer, m_reader );
  return *m_cursor;
}


void RNTCollectionQuery::addToTokenOutputList( const std::string& columnName )
{
   // Add to select list, if not already present
   if( m_selectedColumnNames.find( columnName ) == m_selectedColumnNames.end() ) {
      m_description.tokenColumn( columnName ); 
      m_outputTokenList.extend( columnName );
      m_selectedColumnNames.insert( columnName );
   }
}


void RNTCollectionQuery::addToAttributeOutputList( const std::string& columnName )
{  
   // Add to select list, if not already present
   if( m_selectedColumnNames.find( columnName ) == m_selectedColumnNames.end() ) {
      m_outputAttributeList.extend( columnName, m_description.attributeColumn( columnName ).type() );
      m_selectedColumnNames.insert( columnName );
   }
}
