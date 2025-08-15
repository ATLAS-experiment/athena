/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "RootCollectionQuery.h"
#include "RootCollectionCursor.h"

#include "CollectionBase/ICollectionDescription.h"
#include "CollectionBase/TokenList.h"
#include "CollectionBase/CollectionRowBuffer.h"
#include "CollectionBase/ICollectionCursor.h"
#include "CollectionBase/ICollectionColumn.h"
#include "CollectionBase/CollectionBaseNames.h"
#include "CollectionBase/boost_tokenizer_headers.h"

#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeList.h"
#include "CoralBase/MessageStream.h"

#include "TEventList.h"

pool::RootCollection::RootCollectionQuery::
RootCollectionQuery(
   const pool::ICollectionDescription& description,
   TTree *tree
   ) :
      m_description( description ),
      m_tree( tree ),
      m_cursor( 0 ),
      m_skipEventRef( false )
{
}


pool::RootCollection::RootCollectionQuery::~RootCollectionQuery()
{
   delete m_cursor;   m_cursor = 0;
}


void
pool::RootCollection::RootCollectionQuery::selectAllAttributes()
{
   for( int j = 0; j < m_description.numberOfAttributeColumns(); j++ )    {
      addToAttributeOutputList( m_description.attributeColumn( j ).name() );
   }
}


void
pool::RootCollection::RootCollectionQuery::selectAllTokens()
{
   for( int j = 0; j < m_description.numberOfTokenColumns(); j++ )    {
      addToTokenOutputList( m_description.tokenColumn( j ).name() );
   }
}


void
pool::RootCollection::RootCollectionQuery::selectAll()
{
   selectAllAttributes();
   selectAllTokens();
}


pool::ICollectionCursor& 
pool::RootCollection::RootCollectionQuery::execute()
{
   if( !m_skipEventRef && m_description.hasEventReferenceColumn() )  {
      addToTokenOutputList( m_description.eventReferenceColumnName() );
   }
  
  TEventList* eventList = 0;

  // Create collection row buffer to contain query output.
  pool::CollectionRowBuffer collectionRowBuffer( m_outputTokenList, m_outputAttributeList );

  // Execute query and create a cursor for iterating over the result.
 //  m_query->defineOutput( *m_outputDataBuffer );

  m_cursor = new RootCollectionCursor( m_description, collectionRowBuffer, m_tree, eventList );

  return *m_cursor;
}



void
pool::RootCollection::RootCollectionQuery::addToTokenOutputList( const std::string& columnName )
{
   // Add to select list, if not already present
   if( m_selectedColumnNames.find( columnName ) == m_selectedColumnNames.end() ) {
      try {
         m_description.tokenColumn( columnName ); 
      } catch( std::runtime_error& /* e */ ) {
         std::string errorMsg( "Token column with name `" + columnName + "' does not exist." );
         throw std::runtime_error( errorMsg + " (APR: \" RootCollectionQuery::addToTokenOutputList \" from \" RootCollection \")");
      }
      m_outputTokenList.extend( columnName );
      m_selectedColumnNames.insert( columnName );
   }   
}



void
pool::RootCollection::RootCollectionQuery::addToAttributeOutputList( const std::string& columnName )
{  
   // Add to select list, if not already present
   if( m_selectedColumnNames.find( columnName ) == m_selectedColumnNames.end() ) {
      try {
         m_outputAttributeList.extend( columnName, m_description.attributeColumn( columnName ).type() );
      } catch( std::runtime_error& /* e */ ) {
         std::string errorMsg( "Attribute column with name `" + columnName + "' does not exist." );
         throw std::runtime_error( errorMsg + " (APR: \" RootCollectionQuery::addToAttributeOutputList \" from \" RootCollection \")");
      }         
      m_selectedColumnNames.insert( columnName );
   }
}
