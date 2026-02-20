/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TTreeCollectionCursor.h"
#include "CollectionSvc/CollectionColumn.h"
#include "PersistentDataModel/Token.h"

#include "CoralBase/Attribute.h"

#include "TTree.h"

#include <exception>

pool::RootCollection::TTreeCollectionCursor::
TTreeCollectionCursor(
   const pool::CollectionDescription& description,
   const pool::CollectionRowBuffer& collectionRowBuffer,
   TTree *tree
   )
      :
      m_description( description ),
      m_collectionRowBuffer( collectionRowBuffer ),
      m_idx(-1),
      m_entries( tree->GetEntries() )
{
   for( coral::AttributeList::iterator attrI = m_collectionRowBuffer.attributeList().begin();
        attrI != m_collectionRowBuffer.attributeList().end();
        ++attrI ) {
      std::string branchName = attrI->specification().name();
      TBranch* branch = tree->GetBranch( branchName.c_str() );
      if( !branch ) {
         std::string errorMsg = "Failed to retrieve TBranch " + branchName + " from the CollectionTree";
         throw std::runtime_error( errorMsg + " (APR: \" TTreeCollectionCursor() \" from \" TTreeCollection \")");
      }
      if( attrI->specification().type() == typeid(std::string) ) {
         branch->SetAddress( m_charBuffer );
         m_attrBranches.push_back( std::make_pair(branch, &attrI->data<std::string>()) );
      } else {
         branch->SetAddress( attrI->addressOfData() );
         m_attrBranches.push_back( std::make_pair( branch, (std::string*)0 ) );
      }
   }
   
   std::string branchName = description.tokenColumn().name();
   m_tokenBranch = tree->GetBranch( branchName.c_str() );
   if( !m_tokenBranch ) {
      std::string errorMsg = "Failed to retrieve TBranch " + branchName + " from the CollectionTree";
      throw std::runtime_error( errorMsg + " (APR: \" TTreeCollectionCursor() \" from \" TTreeCollection \")");
   }
   m_tokenBranch->SetAddress( m_charBuffer );
}


pool::RootCollection::TTreeCollectionCursor::~TTreeCollectionCursor()
{
   TTreeCollectionCursor::close();
}


void
pool::RootCollection::TTreeCollectionCursor::close()
{ }


bool
pool::RootCollection::TTreeCollectionCursor::next()
{
   if( ++m_idx >= size() ) {
      return false;
   }
   Long64_t entry = m_idx;

   // read attributes
   for( auto& branchEl : m_attrBranches ) {
      branchEl.first->GetEntry(entry);
      if( branchEl.second ) {
         // copy the read string from character buffer to std::string of the coral attribute
         *branchEl.second = m_charBuffer;
      }
   }
   // read token
   m_tokenBranch->GetEntry(entry);
   m_collectionRowBuffer.token().fromString( m_charBuffer );
   
   return true;
}


const pool::CollectionRowBuffer&
pool::RootCollection::TTreeCollectionCursor::currentRow() const
{
  return m_collectionRowBuffer;
}


std::size_t
pool::RootCollection::TTreeCollectionCursor::size()
{
  return m_entries;
}


bool
pool::RootCollection::TTreeCollectionCursor::seek(std::size_t position)
{
   if( position >= size() ) {
      return false;
   }
   m_idx = position-1;
   return true;
}


const Token&
pool::RootCollection::TTreeCollectionCursor::eventRef() const
{
   return m_collectionRowBuffer.token();
}
