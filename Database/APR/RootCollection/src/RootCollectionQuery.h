/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ROOTCOLLECTION_COLLECTIONQUERY_H
#define ROOTCOLLECTION_COLLECTIONQUERY_H

#include "CoralBase/AttributeList.h"

#include "CollectionBase/TokenList.h"
#include "CollectionBase/ICollectionQuery.h"
#include "CollectionBase/ICollectionDescription.h"
#include "CollectionBase/ICollectionCursor.h"

#include "RootCollectionCursor.h"

#include "TTree.h"

#include <set>

namespace pool {
   namespace RootCollection {

      /** 
       * @class RootCollectionQuery RootCollectionQuery.h Rootcollection/RootCollectionQuery.h
       *
       * An interface used to query a collection.
       */
      class RootCollectionQuery : public ICollectionQuery
      {
     public:
        /// Constructor
        RootCollectionQuery( const pool::ICollectionDescription& description, TTree *tree );
    
        /// Destructor
        virtual ~RootCollectionQuery();

        /// Adds all Attribute columns to the query select list.
        virtual void selectAllAttributes();

        /// Adds all Token columns to the query select list.
        virtual void selectAllTokens();

        /// Adds all Token and Attribute columns to the query select list.
        virtual void selectAll();

        /// Processes the query and returns a cursor over the query result.
        virtual pool::ICollectionCursor& execute();

     protected:

        void                addToTokenOutputList( const std::string& columnName );

        void                addToAttributeOutputList( const std::string& columnName );
        
        
        const ICollectionDescription    &m_description;

        TTree                           *m_tree;

        RootCollectionCursor            *m_cursor;

        pool::TokenList                 m_outputTokenList;
        coral::AttributeList            m_outputAttributeList;

        std::set< std::string >         m_selectedColumnNames;
        std::set< std::string >         m_collectionFragmentNames;

        /// If false, the primary event reference is added always to the query result
        bool                            m_skipEventRef; 
      };

   }
}

#endif


