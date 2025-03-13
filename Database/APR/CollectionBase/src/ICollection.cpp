/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "CollectionBase/ICollection.h"
#include "CollectionBase/ICollectionDescription.h"
#include "CollectionBase/TokenList.h"
#include "CollectionBase/ICollectionColumn.h"
#include "CollectionBase/CollectionRowBuffer.h"
#include "CoralBase/Attribute.h"

/// Initialize a new RowBuffer by adding all Attributes and Tokens of this collection to it
void pool::ICollection::initNewRow( pool::CollectionRowBuffer& rowBuffer ) const
{
   pool::TokenList                      tokenList;
   coral::AttributeList                 attributeList;
   const ICollectionDescription&        descr = description();
   
   for( int j = 0; j < descr.numberOfTokenColumns(); j++ ) {
      tokenList.extend( descr.tokenColumn( j ).name() );
   }
   for( int j = 0; j < descr.numberOfAttributeColumns(); j++ ) {
      const auto& attrCol = descr.attributeColumn( j );
      attributeList.extend( attrCol.name(), attrCol.type() );
   }
   rowBuffer.setTokenList( tokenList );
   rowBuffer.setAttributeList( attributeList );   
}
