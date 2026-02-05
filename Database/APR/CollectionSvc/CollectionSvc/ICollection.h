/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef COLLECTIONSVC_ICOLLECTION_H
#define COLLECTIONSVC_ICOLLECTION_H

#include <string>

namespace pool {

  class ICollectionCursor;
  class CollectionDescription;
  class CollectionRowBuffer;

  /** 
   * @class ICollection ICollection.h Collection/ICollection.h
   *
   * An interface to a storage technology specific collection of event references
   * and attributes
   */
   class ICollection
  {
  public:
    /// Enumeration of the possible open modes of the collection.
    typedef enum { CREATE_AND_OVERWRITE, READ } OpenMode;

    /// Opens the collection and initializes it if necessary.
    virtual void open() = 0;

    /// Initialize a new RowBuffer by adding all Attributes adn Tokens of this collection to it
    virtual void initNewRow( pool::CollectionRowBuffer& row ) const;

    /// Adds a new row of data to the collection.
    virtual void insertRow( const pool::CollectionRowBuffer& inputRowBuffer ) = 0;

    /// Commits the latest changes made to the collection.
    virtual void commit( bool restartTransaction = true ) = 0;

    /// Closes the collection and terminates any database connections.
    virtual void close() = 0;

    /// Returns an object used to describe the collection properties.
    virtual const CollectionDescription& description() const = 0;

    /// Returns an cursor for the collection.
    virtual ICollectionCursor& cursor() = 0;
    
    /// Empty destructor.
    virtual ~ICollection() = default;

  };

}

#endif
