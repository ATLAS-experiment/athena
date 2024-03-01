/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "RNTCollectionCursor.h"

#include "CoralBase/Attribute.h"
#include "POOLCore/Exception.h"

#if ROOT_VERSION_CODE >= ROOT_VERSION( 6, 31, 0 )
#include "ROOT/RNTupleReader.hxx"
#else
#include "ROOT/RNTuple.hxx"
#endif
#include "ROOT/RField.hxx"


using namespace pool::RootCollection;

RNTCollectionCursor::RNTCollectionCursor(
   const pool::ICollectionDescription& description,
   const pool::CollectionRowBuffer& collectionRowBuffer,        
   RNTupleReader* reader )
   : m_description( description ),
     m_reader( reader ),
     m_collectionRowBuffer( collectionRowBuffer ),
     m_idx(-1),
     m_entries( reader->GetNEntries() ),
     m_dummyRef( false )
{
#if ROOT_VERSION_CODE >= ROOT_VERSION( 6, 31, 0 )
   for( auto& attr : m_collectionRowBuffer.attributeList() ) {
      std::string    attrName = attr.specification().name();
      const auto& rntuple = m_reader->GetDescriptor();
      auto field_id = rntuple.FindFieldId( attrName );
      auto& field_desc = rntuple.GetFieldDescriptor(field_id);
      std::unique_ptr<RFieldBase> field = field_desc.CreateField(rntuple);
      // MN: the move constructor is broken now, can't use this field to read
      m_attrFields.emplace_back( std::move( field ), attr.addressOfData() );
   }
#endif

   if( m_description.eventReferenceColumnName() == "DummyRef" )  {
      // Athena "fake" collection with no Tokens
      m_dummyRef = true;
      return;  
   }
   
   for( pool::TokenList::iterator tokenI = m_collectionRowBuffer.tokenList().begin();
        tokenI != m_collectionRowBuffer.tokenList().end(); ++tokenI )
   {
      // MN: the move constructor is broken now, can't use this field to read
      m_tokenFields.emplace_back(std::move(m_reader->GetView<std::string>( tokenI.tokenName())), &*tokenI);
   }
}


RNTCollectionCursor::~RNTCollectionCursor()
{
   close();
}


void RNTCollectionCursor::close()
{
}


bool RNTCollectionCursor::next()
{
   if( ++m_idx >= m_entries ) {
      return false;
   }

   // read attributes
#if ROOT_VERSION_CODE >= ROOT_VERSION( 6, 31, 0 )
   auto entry = m_reader->GetModel().CreateEntry();
   for( auto& elem : m_attrFields ) {
      //cout << "  field " << elem.first->GetFieldName() << " is " << 
      //   ( (elem.first->GetState() != RFieldBase::EState::kConnectedToSource)? "not" : "")
      //     << " connected   " << (int)elem.first->GetState() <<endl;
      //RFieldBase::RValue val = elem.first->BindValue(std::shared_ptr<void>(elem.second, [](void *) {}));
      //val.Read(m_idx);
      entry->BindValue( elem.first->GetFieldName(), std::shared_ptr<void>(elem.second, [](void *) {}) );
   }
   // read Tokens
   for( auto& elem : m_tokenFields ) {
      // std::string tokenstr =  elem.first(m_idx);
      std::string tokenstr = m_reader->GetView<std::string>( elem.first.GetField().GetFieldName() )( m_idx );
      elem.second->fromString( tokenstr );
   }
   m_reader->LoadEntry(m_idx, *entry);
#endif
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
  iData = m_cursor.currentRow().begin();
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
   if( position >= m_entries ) {
      return false;
   }
   m_idx = position-1;
   return true;
}


int RNTCollectionCursor::size()
{
  return m_entries;
}


const Token&  RNTCollectionCursor::eventRef() const
{
   static const Token dummyToken;
   if( m_dummyRef )  return dummyToken; 
   return m_collectionRowBuffer.tokenList()[ m_description.eventReferenceColumnName() ];
}
