/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef RNTCOLLECTIONCURSOR_H
#define RNTCOLLECTIONCURSOR_H


#include "PersistentDataModel/Token.h"

#include "CollectionBase/CollectionRowBuffer.h"
#include "CollectionBase/ICollectionDescription.h"
#include "CollectionBase/ICollectionCursor.h"

#include "PersistencySvc/IPositionSeek.h"
#include "AthenaKernel/ICollectionSize.h"

#include <memory>
#include <map>

// for version checks
#include "TROOT.h"

namespace ROOT::Experimental {
   class RNTupleReader;
   template<class> class RNTupleView;
   class RFieldBase;
}

namespace pool {
   namespace RootCollection {

      using ROOT::Experimental::RNTupleReader;
      using ROOT::Experimental::RNTupleView;
      using ROOT::Experimental::RFieldBase;

      /** 
       * @class RNTCollectionCursor RNTCollectionCursor.h Rootcollection/RNTCollectionCursor.h
       *
       * An interface used to navigate the result of a query on a collection
       * stored in RNTuple
       */
      class RNTCollectionCursor : public ICollectionCursor,
                                  virtual public IPositionSeek,
                                  virtual public ICollectionSize
      {
      public:

         RNTCollectionCursor(
            const pool::ICollectionDescription& description,
            const pool::CollectionRowBuffer& collectionRowBuffer,
            RNTupleReader* reader );

        
         /// Advances the cursor to the next row of the query result set.
         virtual bool next() override;

         /// Returns the selected Tokens and Attributes for the current row of the query result set.
         virtual const pool::CollectionRowBuffer& currentRow() const override;

         /// Seeks the cursor to a given position in the collection.
         virtual bool seek(long long int position) override;

         /// Return the size of the collection.
         virtual int size() override;

         /// Returns the event reference Token for the current row.
         virtual const Token& eventRef() const override;

         /// Cleanup.
         virtual void close() override;

         virtual ~RNTCollectionCursor();

      protected:

         const ICollectionDescription&  m_description;

         RNTupleReader*                 m_reader;

         /// Row buffer containing Tokens and Attributes selected by query.
         pool::CollectionRowBuffer      m_collectionRowBuffer;

#if ROOT_VERSION_CODE >= ROOT_VERSION( 6, 31, 0 )
         std::vector< std::pair< std::unique_ptr<RFieldBase>, void* > >  m_attrFields;
#endif
         std::vector< std::pair< RNTupleView< std::string >, Token* > >  m_tokenFields;

         int                            m_idx;
         int64_t                        m_entries;
         bool                           m_dummyRef;
      };
   }
}

#endif
