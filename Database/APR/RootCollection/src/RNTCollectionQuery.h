/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef RNTCOLLECTIONQUERY_H
#define RNTCOLLECTIONQUERY_H

#include "CoralBase/AttributeList.h"

#include "AthenaBaseComps/AthMessaging.h"

#include "CollectionBase/TokenList.h"
#include "CollectionBase/ICollectionQuery.h"
#include "CollectionBase/ICollectionDescription.h"
#include "CollectionBase/ICollectionCursor.h"

#include "RNTCollectionCursor.h"

#include <set>

namespace pool::RootCollection {

   /** 
    * @class RNTCollectionQuery RNTCollectionQuery.h Rootcollection/RNTCollectionQuery.h
    *
    * An interface used to query an RNTuple collection.
    */
   class RNTCollectionQuery : public ICollectionQuery, public AthMessaging
   {
   public:
      /// Constructor
      RNTCollectionQuery( const pool::ICollectionDescription& description, ROOT::RNTupleReader *reader );
    
      /// Destructor
      virtual ~RNTCollectionQuery();

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
        
        
      const ICollectionDescription   &m_description;
      ROOT::RNTupleReader            *m_reader {nullptr};   // owned by the Collection

      RNTCollectionCursor            *m_cursor {nullptr};

      pool::TokenList                 m_outputTokenList;
      coral::AttributeList            m_outputAttributeList;

      std::set< std::string >         m_selectedColumnNames;

      /// If false, the primary event reference is added always to the query result
      bool                            m_skipEventRef; 
   };

} // end namespace 

#endif
