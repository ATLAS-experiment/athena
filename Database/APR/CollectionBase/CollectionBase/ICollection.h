/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef COLLECTIONBASE_ICOLLECTION_H
#define COLLECTIONBASE_ICOLLECTION_H

#include <string>

namespace pool {

  class ICollectionDescription;
  class ICollectionQuery;
  class CollectionRowBuffer;

  /** 
   * @class ICollection ICollection.h CollectionBase/ICollection.h
   *
   * An interface to a storage technology specific collection of event references
   * and attributes
   */
   class ICollection
  {
  public:
    /// Enumeration of the possible open modes of the collection.
    typedef enum { CREATE, CREATE_AND_OVERWRITE, UPDATE, READ } OpenMode;

    /// Returns the open mode of the collection for the present transaction.
    virtual ICollection::OpenMode openMode() const = 0;

    /// Opens the collection and initializes it if necessary.
    virtual void open() = 0;

    /// Checks if the collection is open.
    virtual bool isOpen() const = 0;

    /// Initialize a new RowBuffer by adding all Attributes adn Tokens of this collection to it
    virtual void initNewRow( pool::CollectionRowBuffer& row ) const;

    /// Adds a new row of data to the collection.
    virtual void insertRow( const pool::CollectionRowBuffer& inputRowBuffer ) = 0;

    /// Commits the latest changes made to the collection.
    virtual void commit( bool restartTransaction = true ) = 0;

    /// Closes the collection and terminates any database connections.
    virtual void close() = 0;

    /// Returns an object used to describe the collection properties.
    virtual const ICollectionDescription& description() const = 0;

    /// Returns an object used to query the collection.
    virtual ICollectionQuery* 		newQuery() = 0;
    
    /// Empty destructor.
    virtual ~ICollection() = default;

  };

}

#endif
