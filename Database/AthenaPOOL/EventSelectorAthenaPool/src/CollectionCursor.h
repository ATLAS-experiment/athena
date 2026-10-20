/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef COLLECTIONCURSOR_H
#define COLLECTIONCURSOR_H

#include "PersistentDataModel/Token.h"

#include "CollectionRowBuffer.h"
#include "StorageSvc/DbContainer.h"

#include <cstddef>
#include <map>
#include <string>


namespace pool {

    using ContainerMap = std::map< std::string, DbContainer >;

      /**
       * @class CollectionCursor CollectionCursor.h 
       *
       * Reader object for Collections
       */
      class CollectionCursor
      {
      public:

         CollectionCursor(
            const CollectionRowBuffer& collectionRowBuffer,
            ContainerMap& attrContainers );

         /// Advances the cursor to the next row of the result set.
         bool next();

         /// Returns the selected Tokens and Attributes for the current row of the result set.
         const CollectionRowBuffer& currentRow() const;

         /// Return the size of the collection.
         std::size_t size();

         /// Seeks the cursor to a given position in the collection.
         bool seek(std::size_t position);

         /// Returns the event reference Token for the current row.
         const Token& eventRef() const;

      private:
         /// Row buffer containing the Token and Attributes
         CollectionRowBuffer                 m_collectionRowBuffer;

         ContainerMap&                       m_attrContainers;

         /// Container for the event reference
         DbContainer&                        m_tokenContainer;

         /// Temporary storage for Event Reference in string format as it is in RNTuple
         std::string                         m_tokenStr;

         std::size_t                         m_idx;
      };

}

#endif
