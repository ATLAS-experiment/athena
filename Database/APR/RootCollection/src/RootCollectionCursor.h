/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ROOTCOLLECTION_ROOTCOLLECTIONCURSOR_H
#define ROOTCOLLECTION_ROOTCOLLECTIONCURSOR_H


#include "PersistentDataModel/Token.h"

#include "CollectionSvc/CollectionRowBuffer.h"
#include "CollectionSvc/ICollectionDescription.h"
#include "CollectionSvc/ICollectionCursor.h"

class TTree;
class TBranch;

namespace pool {
   namespace RootCollection {
      /** 
       * @class RootcollectionCursor RootcollectionCursor.h Rootcollection/RootcollectionCursor.h
       *
       * An interface used to navigate a collection.
       */
      class RootCollectionCursor : public ICollectionCursor
      {
     public:

        RootCollectionCursor(
           const pool::ICollectionDescription& description,
           const pool::CollectionRowBuffer& collectionRowBuffer,
           TTree *tree );

        
        /// Advances the cursor to the next row
        virtual bool next() override;

        /// Returns the selected Tokens and Attributes for the current row
        virtual const pool::CollectionRowBuffer& currentRow() const override;

        /// Return the size of the collection.
        virtual std::size_t size() override;

        /// Seeks the cursor to a given position in the collection.
        virtual bool seek(std::size_t position) override;

        /// Returns the event reference Token for the current row.
        virtual const Token& eventRef() const override;

        /// Cleanup.
        virtual void close() override;

        virtual ~RootCollectionCursor();

     protected:
    
        static const unsigned int       c_maxLengthOfStrings = 5000;    
        
        const ICollectionDescription    &m_description;

        /// Row buffer containing Tokens and Attributes
        pool::CollectionRowBuffer       m_collectionRowBuffer;

        char                             m_charBuffer[c_maxLengthOfStrings];

        typedef std::vector< std::pair<TBranch*, std::string*> >  AttrBranchVector_t;
        typedef std::vector< std::pair<TBranch*, Token*> >        TokenBranchVector_t;

        AttrBranchVector_t              m_attrBranches;
        TokenBranchVector_t             m_tokenBranches;

        std::size_t                     m_idx;
        std::size_t                     m_entries;
        bool                            m_dummyRef;
      };
   }
}

#endif


