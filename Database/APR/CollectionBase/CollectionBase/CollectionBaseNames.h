/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef COLLECTIONBASE_COLLECTIONBASENAMES_H
#define COLLECTIONBASE_COLLECTIONBASENAMES_H

namespace pool {

  /**
   * @namespace CollectionBaseNames CollectionBaseNames.h CollectionBase/CollectionBaseNames.h
   *
   * string literals for the CollectionBase package.
   */
  namespace CollectionBaseNames 
  {
    /// The type name for objects of type pool::Token.
    static constexpr const char* tokenTypeName {"Token"};


    /// The default name assigned to the event reference Token column of a collection.
    static constexpr const char* defaultEventReferenceColumnName {"Token"};
  };

}

#endif
