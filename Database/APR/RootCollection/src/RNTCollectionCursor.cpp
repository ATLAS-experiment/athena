/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "RNTCollectionCursor.h"

#include "CoralBase/Attribute.h"

#include "ROOT/REntry.hxx"
#include "ROOT/RNTupleReader.hxx"

using namespace pool::RootCollection;

RNTCollectionCursor::RNTCollectionCursor(
   const pool::ICollectionDescription& description,
   const pool::CollectionRowBuffer& collectionRowBuffer,        
   ROOT::RNTupleReader* reader )
   : m_description( description ),
     m_RNTReader( reader ),
     m_RNTEntry( reader->GetModel().CreateEntry() ),
     m_collectionRowBuffer( collectionRowBuffer ),
     m_idx(-1),
     m_dummyRef( false )
{
   for( auto& attr : m_collectionRowBuffer.attributeList() ) {
      m_RNTEntry->BindRawPtr( attr.specification().name(), attr.addressOfData() );
   }

   if( m_description.eventReferenceColumnName() == "DummyRef" )  {
      // A collection with no Tokens, just attributes
      m_dummyRef = true;
      return;  
   }
   
   for( pool::TokenList::iterator tokenI = m_collectionRowBuffer.tokenList().begin();
        tokenI != m_collectionRowBuffer.tokenList().end(); ++tokenI )
   {
      m_tokens.emplace_back( &*tokenI, std::string() );
      m_RNTEntry->BindRawPtr( tokenI.tokenName(), &m_tokens.back().second );
   }
}


RNTCollectionCursor::~RNTCollectionCursor()
{
   RNTCollectionCursor::close();
}


void RNTCollectionCursor::close()
{
}


bool RNTCollectionCursor::next()
{
   if( ++m_idx >= size() ) {
      return false;
   }
   // read the row
   m_RNTReader->LoadEntry(m_idx, *m_RNTEntry);
   // convert Token strings
   for( auto& elem : m_tokens ) {
      elem.first->fromString( elem.second );
   }

/*
  // Get iterator over current row.
  coral::AttributeList::const_iterator iData = m_cursor.currentRow().begin();
  cout << " * Cursor next(), values: " << endl;
  for( ; iData != m_cursor.currentRow().end(); ++iData ) {
      std::cout << "[";
      iData->toOutputStream( std::cout );
      std::cout << "] ";
  }
  cout << endl;  
*/
  
  return true;
}


const pool::CollectionRowBuffer& 
RNTCollectionCursor::currentRow() const
{
  return m_collectionRowBuffer;
}


bool RNTCollectionCursor::seek(long long int position)
{
   if( position >= size() ) {
      return false;
   }
   m_idx = position-1;
   return true;
}


int RNTCollectionCursor::size()
{
  return m_RNTReader->GetNEntries();
}


const Token&  RNTCollectionCursor::eventRef() const
{
   static const Token dummyToken;
   if( m_dummyRef )  return dummyToken; 
   return m_collectionRowBuffer.tokenList()[ m_description.eventReferenceColumnName() ];
}
