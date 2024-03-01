/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
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

#include "POOLCore/Exception.h"

#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeList.h"
#include "CoralBase/MessageStream.h"

#include <ROOT/RNTuple.hxx>
#include <ROOT/RNTupleModel.hxx>

//#include <iostream>
//using namespace std;
using namespace pool::RootCollection;
// Import classes from experimental namespace for the time being
using RNTupleModel = ROOT::Experimental::RNTupleModel;


RNTCollectionQuery::RNTCollectionQuery( const pool::ICollectionDescription& description, 
                                          RNTupleReader *reader ) :
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


void RNTCollectionQuery::addToOutputList( const std::string& columnNames )
{
   typedef boost::tokenizer<boost::char_separator<char> > Tizer;
   boost::char_separator<char> sep(" ,");
   Tizer tizer( columnNames, sep );

   for( Tizer::iterator token = tizer.begin(); token != tizer.end(); ++token ) { 
      if( *token == "*" ) {
         selectAll();
         return;
      }
      // Check if is a Token column.
      if( m_description.column( *token ).type() == CollectionBaseNames::tokenTypeName ) {
         addToTokenOutputList( *token );
      } else {
         addToAttributeOutputList( *token );
      }
   }
}


void RNTCollectionQuery::addToOutputList( const std::vector<std::string>& columnNames )
{
  for ( std::vector< std::string >::const_iterator
           iName = columnNames.begin(); iName != columnNames.end(); ++iName )
  {
     addToOutputList( *iName );
  }
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


void RNTCollectionQuery::setCondition( const std::string& whereClause,
              coral::AttributeList* /* attributeBindData */,
              pool::TokenList* /*tokenBindData */ )
{
   m_whereClause += whereClause;
}


const std::string& RNTCollectionQuery::whereClause() const
{
  return m_whereClause;
}


void RNTCollectionQuery::setRowCacheSize( int )
{
}


pool::ICollectionCursor&  RNTCollectionQuery::execute()
{
   if( !m_skipEventRef && m_description.hasEventReferenceColumn() )  {
      addToTokenOutputList( m_description.eventReferenceColumnName() );
   }
  
   if( m_whereClause.size() ) {
        std::string errorMsg = "Queries not supported yet";
        throw pool::Exception( errorMsg,
                               "RNTCollectionQuery::execute", 
                               "RootCollection");
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
      try {
         m_description.tokenColumn( columnName ); 
      } catch( pool::Exception& /* e */ ) {
         std::string errorMsg( "Token column with name `" + columnName + "' does not exist." );
         throw pool::Exception( errorMsg,
                             "RNTCollectionQuery::addToTokenOutputList",
                             "RootCollection" );
      }
      m_outputTokenList.extend( columnName );
      m_selectedColumnNames.insert( columnName );
   }   
}


void RNTCollectionQuery::addToAttributeOutputList( const std::string& columnName )
{  
   // Add to select list, if not already present
   if( m_selectedColumnNames.find( columnName ) == m_selectedColumnNames.end() ) {
      try {
         m_outputAttributeList.extend( columnName, m_description.attributeColumn( columnName ).type() );
      } catch( pool::Exception& /* e */) {
         std::string errorMsg( "Attribute column with name `" + columnName + "' does not exist." );
         throw pool::Exception( errorMsg,
                             "RNTCollectionQuery::addToAttributeOutputList",
                             "RootCollection" );
      }         
      m_selectedColumnNames.insert( columnName );
   }
}


void RNTCollectionQuery::skipEventReference( bool skip )
{
   m_skipEventRef = skip;
}
