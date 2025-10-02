/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef IMPLICITCOLLECTION_COLLECTIONITERATOR_H
#define IMPLICITCOLLECTION_COLLECTIONITERATOR_H

#include "CollectionBase/ICollectionQuery.h"
#include "CollectionBase/ICollectionCursor.h"
#include "CollectionBase/CollectionRowBuffer.h"

#include "CxxUtils/checker_macros.h"

class Token;

namespace coral { class AttributeList; }

namespace pool {

   // forward declarations
   class IContainer;
   class ITokenIterator;
   class TokenList;
   class ICollectionDescription;
   
   /** 
    * @class ImplicitCollectionIterator ImplicitCollectionIterator.h Implicitcollection/ImplicitCollectionIterator.h
    *
    * Iterator over an implicit collection.
    * Single class implementing both Query and Cursor interfaces
    * to simplify backward compatibility
    */
   class ATLAS_NOT_THREAD_SAFE ImplicitCollectionIterator : public ICollectionQuery,
                                                            public ICollectionCursor
   {
  public:
     /// Constructor
     ImplicitCollectionIterator(IContainer& container,
                                const pool::ICollectionDescription& description );

     // ------------------- Query interface
     
     /// Processes the query and returns a cursor over the query result.
     /// this method returns self
     virtual pool::ICollectionCursor& execute();


     // ------------------- Cursor interface 

     /** Retrieves the next token in the collection.
         Returns false if the end of the collection has been reached.
     */
     virtual bool                         next();

     /// Returns the token of the current position
     virtual Token*                         token() const;
     virtual const Token&                 eventRef() const { return *token(); }

     virtual const CollectionRowBuffer& currentRow() const;

     // ------------------- Seek interface 
     /**
      * @brief Seek to a given position in the collection
      * @param position  The position to which to seek.
      * @returns True if successful, false otherwise.
      */
     virtual bool seek(std::size_t position);

     /**
      * @brief Return the size of the collection.
      */
     virtual std::size_t size();

     
     //------------------------------------------

     /// Destructor
     virtual ~ImplicitCollectionIterator();


     // ------------------- Unimplemented methods

     virtual void selectAllAttributes() {}
     virtual void selectAllTokens() {}
     virtual void selectAll() {}

     virtual void close() {}


  protected:
     IContainer&        m_container;
     ITokenIterator*    m_tokenIterator;
     Token*             m_token;

     mutable CollectionRowBuffer        m_rowBuffer;
   };

}

#endif


