/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef COLLECTIONSVC_ICOLLECTIONDESCRIPTION_H
#define COLLECTIONSVC_ICOLLECTIONDESCRIPTION_H

#include "StorageSvc/DbType.h"

#include <string>


namespace pool {

  class ICollectionColumn;

  /**
   * @class ICollectionDescription ICollectionDescription.h CollectionSvc/ICollectionDescription.h
   *
   * An interface used to define the properties of a collection to be constructed and to retrieve
   * these properties after construction. The schema editor of the collection should be used for
   * any modifications to these properties after construction.
   */
  class ICollectionDescription
  {
  public:
    ICollectionDescription() = default;

    /// Returns the name of the collection
    virtual const std::string& name() const = 0;

    /// Returns the storage technology type of the collection.
    virtual const DbType& type() const = 0;

    /// Returns the connection to the database containing the collection.
    virtual const std::string& connection() const = 0;

    /**
     * Returns the name reserved for the event reference Token column. If the name has not
     * been set by the user a default name is returned.
     */
    virtual const std::string& eventReferenceColumnName() const = 0;

    /**
     * Returns the number of Token columns (including the event reference column if it is used)
     */
    virtual int numberOfTokenColumns() const = 0;

    /**
     * Returns a description object for a Token column of the collection, given the position
     * of the column
     *
     * @param columnId Position of column.
     */
    virtual const ICollectionColumn& tokenColumn( int columnId ) const = 0;

    /**
     * Returns the number of Attribute columns
     */
    virtual int numberOfAttributeColumns( ) const = 0;

    /**
     * Returns a description object for an Attribute column of the collection, given the position
     * of the column.
     *
     * @param columnId Position of column in associated collection fragment.
     */
    virtual const ICollectionColumn& attributeColumn( int columnId ) const = 0;

  protected:
    /// Empty destructor.
    virtual ~ICollectionDescription() {}
  };

}

#endif

