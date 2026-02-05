/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "RNTupleCollectionCursor.h"

#include "CoralBase/Attribute.h"

#include "ROOT/REntry.hxx"
#include "ROOT/RNTupleReader.hxx"

using namespace pool::RootCollection;

RNTupleCollectionCursor::RNTupleCollectionCursor(
   const pool::CollectionDescription& description,
   const pool::CollectionRowBuffer& collectionRowBuffer,
   ROOT::RNTupleReader* reader )
   : m_description( description ),
     m_RNTReader( reader ),
     m_RNTEntry( reader->GetModel().CreateEntry() ),
     m_collectionRowBuffer( collectionRowBuffer ),
     m_idx(-1)
{
   for( auto& attr : m_collectionRowBuffer.attributeList() ) {
      m_RNTEntry->BindRawPtr( attr.specification().name(), attr.addressOfData() );
   }

   for( pool::TokenList::iterator tokenI = m_collectionRowBuffer.tokenList().begin();
        tokenI != m_collectionRowBuffer.tokenList().end(); ++tokenI )
   {
      m_tokens.emplace_back( &*tokenI, std::string() );
      m_RNTEntry->BindRawPtr( tokenI.tokenName(), &m_tokens.back().second );
   }
}


RNTupleCollectionCursor::~RNTupleCollectionCursor()
{
   RNTupleCollectionCursor::close();
}


void RNTupleCollectionCursor::close()
{
}


bool RNTupleCollectionCursor::next()
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
   return true;
}


const pool::CollectionRowBuffer&
RNTupleCollectionCursor::currentRow() const
{
  return m_collectionRowBuffer;
}


std::size_t RNTupleCollectionCursor::size()
{
  return m_RNTReader->GetNEntries();
}


bool RNTupleCollectionCursor::seek(std::size_t position)
{
   if( position >= size() ) {
      return false;
   }
   m_idx = position-1;
   return true;
}


const Token& RNTupleCollectionCursor::eventRef() const
{
   return m_collectionRowBuffer.tokenList()[ m_description.eventReferenceColumnName() ];
}
