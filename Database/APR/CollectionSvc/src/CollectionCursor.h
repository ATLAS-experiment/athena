/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef COLLECTIONCURSOR_H
#define COLLECTIONCURSOR_H

#include "PersistentDataModel/Token.h"

#include "CollectionSvc/CollectionRowBuffer.h"
#include "CollectionSvc/CollectionDescription.h"
#include "CollectionSvc/ICollectionCursor.h"
#include "StorageSvc/DbContainer.h"

#include <memory>
#include <map>


namespace pool {

    using ContainerMap = std::map< std::string, DbContainer >;

      /**
       * @class CollectionCursor CollectionCursor.h 
       *
       * Reader object for Collections
       */
      class CollectionCursor : public ICollectionCursor
      {
      public:

         CollectionCursor(
            const CollectionDescription& description,
            const CollectionRowBuffer& collectionRowBuffer,
            ContainerMap& attrContainers );

         /// Advances the cursor to the next row of the result set.
         virtual bool next() override final;

         /// Returns the selected Tokens and Attributes for the current row of the result set.
         virtual const CollectionRowBuffer& currentRow() const override final;

         /// Return the size of the collection.
         virtual std::size_t size() override final;

         /// Seeks the cursor to a given position in the collection.
         virtual bool seek(std::size_t position) override final;

         /// Returns the event reference Token for the current row.
         virtual const Token& eventRef() const override final;

         virtual ~CollectionCursor() = default;

      protected:
         const CollectionDescription&        m_description;

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
