/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "CollectionSvc/ICollection.h"
#include "CollectionSvc/CollectionDescription.h"
#include "CollectionSvc/CollectionColumn.h"
#include "CollectionSvc/CollectionRowBuffer.h"
#include "CoralBase/Attribute.h"

/// Initialize a new RowBuffer by adding all Attributes and Tokens of this collection to it
void pool::ICollection::initNewRow( pool::CollectionRowBuffer& rowBuffer ) const
{
   coral::AttributeList          attributeList;
   
   for( int j = 0; j < description().numberOfAttributeColumns(); j++ ) {
      const auto& attrCol = description().attributeColumn( j );
      attributeList.extend( attrCol.name(), attrCol.type() );
   }
   rowBuffer.setAttributeList( attributeList );   
}
