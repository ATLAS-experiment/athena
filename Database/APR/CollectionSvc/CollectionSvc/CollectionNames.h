/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef COLLECTIONSVC_COLLECTIONNAMES_H
#define COLLECTIONSVC_COLLECTIONNAMES_H

namespace pool {

  /**
   * @namespace CollectionNames CollectionNames.h Collection/CollectionNames.h
   *
   * string literals for the Collection package.
   */
  namespace CollectionNames 
  {
    /// The type name for objects of type pool::Token.
    static constexpr const char* tokenTypeName {"Token"};


    /// The default name assigned to the event reference Token column of a collection.
    static constexpr const char* defaultEventReferenceColumnName {"Token"};
  };

}

#endif
