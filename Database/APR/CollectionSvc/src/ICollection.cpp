/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "CollectionSvc/ICollection.h"
#include "CollectionSvc/CollectionDescription.h"
#include "CollectionSvc/CollectionColumn.h"
#include "CollectionSvc/CollectionRowBuffer.h"
#include "CollectionSvc/TokenList.h"
#include "CoralBase/Attribute.h"

/// Initialize a new RowBuffer by adding all Attributes and Tokens of this collection to it
void pool::ICollection::initNewRow( pool::CollectionRowBuffer& rowBuffer ) const
{
   pool::TokenList                      tokenList;
   coral::AttributeList                 attributeList;
   const CollectionDescription&        descr = description();
   
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
