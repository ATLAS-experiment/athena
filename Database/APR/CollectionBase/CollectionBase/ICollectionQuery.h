/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef COLLECTIONBASE_ICOLLECTIONQUERY_H
#define COLLECTIONBASE_ICOLLECTIONQUERY_H

#include <string>
#include <vector>


namespace coral {
  class AttributeList;
}

namespace pool {

  class ICollectionCursor;
  class TokenList;

  /** 
   * @class ICollectionQuery ICollectionQuery.h CollectionBase/ICollectionQuery.h
   *
   * An interface used to query a collection.
   */
  class ICollectionQuery
  {
  public:
    /// Default destructor.
    virtual ~ICollectionQuery() {}

    /// Adds all Attribute columns to the query select list.
    virtual void selectAllAttributes() = 0;

    /// Adds all Token columns to the query select list.
    virtual void selectAllTokens() = 0;

    /// Adds all Token and Attribute columns to the query select list.
    virtual void selectAll() = 0;

    /// Processes the query and returns a cursor over the query result.
    virtual pool::ICollectionCursor& execute() = 0;
  };

}

#endif


