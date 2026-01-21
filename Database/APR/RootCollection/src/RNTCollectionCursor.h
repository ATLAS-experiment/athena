/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef RNTCOLLECTIONCURSOR_H
#define RNTCOLLECTIONCURSOR_H

#include "PersistentDataModel/Token.h"

#include "CollectionSvc/CollectionRowBuffer.h"
#include "CollectionSvc/ICollectionDescription.h"
#include "CollectionSvc/ICollectionCursor.h"

#include <memory>

#include "RVersion.h"

namespace ROOT {
   class REntry;
   class RNTupleReader;
}

namespace pool {
   namespace RootCollection {

      /** 
       * @class RNTCollectionCursor RNTCollectionCursor.h Rootcollection/RNTCollectionCursor.h
       *
       * An interface used to navigate
       * stored in RNTuple
       */
      class RNTCollectionCursor : public ICollectionCursor
      {
      public:

         RNTCollectionCursor(
            const pool::ICollectionDescription& description,
            const pool::CollectionRowBuffer& collectionRowBuffer,
            ROOT::RNTupleReader* reader );

        
         /// Advances the cursor to the next row of the result set.
         virtual bool next() override;

         /// Returns the selected Tokens and Attributes for the current row of the result set.
         virtual const pool::CollectionRowBuffer& currentRow() const override;

         /// Return the size of the collection.
         virtual std::size_t size() override;

         /// Seeks the cursor to a given position in the collection.
         virtual bool seek(std::size_t position) override;

         /// Returns the event reference Token for the current row.
         virtual const Token& eventRef() const override;

         /// Cleanup.
         virtual void close() override;

         virtual ~RNTCollectionCursor();

      protected:

         const ICollectionDescription&  m_description;

         ROOT::RNTupleReader*                 m_RNTReader;

         /// RNtuple row with Field addresses set to collectionRowBuffer attributes
         std::unique_ptr< ROOT::REntry >      m_RNTEntry;

         /// Row buffer containing Tokens and Attributes
         pool::CollectionRowBuffer      m_collectionRowBuffer;

         /// "Token rowBuffer" for reading Tokens as strings and converting them later
         std::vector< std::pair< Token*, std::string > >  m_tokens;

	 std::size_t                    m_idx;
         bool                           m_dummyRef;
      };
   }
}

#endif
