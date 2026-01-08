/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef COLLECTIONSVC_COLLECTIONITERATOR_H
#define COLLECTIONSVC_COLLECTIONITERATOR_H

#include "CollectionSvc/ICollectionCursor.h"
#include "CollectionSvc/CollectionRowBuffer.h"

#include "CxxUtils/checker_macros.h"
#include <memory>

class Token;

namespace coral { class AttributeList; }

namespace pool {

   // forward declarations
   class IContainer;
   class ITokenIterator;
   class ICollectionDescription;
   
   /** 
    * @class ImplicitCollectionIterator ImplicitCollectionIterator.h Implicitcollection/ImplicitCollectionIterator.h
    *
    * Iterator over an implicit collection.
    * Class implementing Cursor interface
    */
   class ATLAS_NOT_THREAD_SAFE ImplicitCollectionIterator : public ICollectionCursor
   {
  public:
     /// Constructor
     ImplicitCollectionIterator(IContainer& container,
                                const pool::ICollectionDescription& description );

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

     virtual void close() {}


  protected:
     IContainer&        m_container;
     std::unique_ptr<ITokenIterator> m_tokenIterator;
     Token*             m_token;

     mutable CollectionRowBuffer        m_rowBuffer;
   };

}

#endif
