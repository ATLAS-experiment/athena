/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef COLLECTIONSVC_COLLECTIONDESCRIPTION_H
#define COLLECTIONSVC_COLLECTIONDESCRIPTION_H

#include "StorageSvc/DbType.h"

#include <map>
#include <string>
#include <vector>


namespace pool {

  class CollectionColumn;

  /**
   * @class CollectionDescription CollectionDescription.h CollectionSvc/CollectionDescription.h
   *
   * An implementation used to define the properties of
   * a collection to be constructed and to retrieve these properties after construction.
   */
  class CollectionDescription
  {
  public:
    /**
     * Constructor that takes as input the minimum amount of properties needed to describe
     * the collection.
     *
     * @param name Name of collection.
     * @param type Storage technology type of collection.
     * @param connection Connection to database containing collection.
     */
    CollectionDescription( const std::string& name,
                           const DbType& type,
                           const std::string& connection = "" );

    /**
     * Copy constructor.
     *
     * @param rhs Collection description object to copy.
     */
    CollectionDescription( const CollectionDescription& rhs );

    /// Default destructor.
    ~CollectionDescription();

    /**
     * Assignment operator.
     *
     * @param rhs source CollectionDescription object to copy.
     */
    CollectionDescription& operator=( const CollectionDescription& rhs );

    // Force the use of the user-defined copy operator (the default one leaks)
    CollectionDescription& operator= (CollectionDescription&& rhs)
    { operator=( (const CollectionDescription&) rhs ); return *this; }

    // Defaults should work for move.
    CollectionDescription (CollectionDescription&&) = default;

    /**
     * Sets the name of the collection.
     *
     * @param name Name of collection.
     */
    void setName( const std::string& name );

    /**
     * Sets the storage technology type of the collection.
     *
     * @param type Storage technology type of collection.
     */
    void setType( const DbType& type );

    /**
     * Sets the connection to the database containing the collection.
     *
     * @param connection Connection to database where collection is stored.
     */
    void setConnection( const std::string& connection );

    /**
     * Adds a new column to the collection.
     *
     * @param columnName Name of new column.
     * @param columnType Data type of new column.
     */
    const CollectionColumn&    insertColumn(
       const std::string& columnName,
       const std::string& columnType );

    /// Returns the name of the collection and the top level collection fragment.
    const std::string& name() const;

    /// Returns the storage technology type of the collection.
    const DbType& type() const;

    /// Returns the connection to the database containing the collection.
    const std::string& connection() const;

    /**
     * Returns a description object for the default Token column of the collection
     */
    static const CollectionColumn& tokenColumn()  { return m_tokenColumn; }

    /**
     * Returns the number of Attribute columns in the collection.
     */
    int numberOfAttributeColumns() const;

    /**
     * Returns a description object for an Attribute column of the collection, given the position
     * of the column.
     *
     * @param columnId Position of column in associated collection fragment.
     */
    const CollectionColumn& attributeColumn( int columnId ) const;

    // set column ID, return the ID
    int		setColumnId( const std::string& columnName, int id );

 protected:
    // some helper methods for internal use:

    /// make this description a copy of 'rhs'
    void	copyFrom( const CollectionDescription& rhs );

    // clear all internal structures
    void	clearAll();

    // set or assign new column ID
    // return the ID
    int 	setColumnId( pool::CollectionColumn *column, int id = -1 );

    // rise an exception if the column aleready exists
    void 	checkNewColumnName( const std::string& name ) const;

  private:
    /// Name of the collection
    std::string m_name;

    /// Storage technology type of collection.
    DbType m_type;

    /// Connection to database containing collection.
    std::string m_connection;

    // Token column description object
    static const pool::CollectionColumn	    m_tokenColumn;

    /// Attribute column description objects
    std::vector< pool::CollectionColumn* >	m_attributeColumns;

    /// Map of column ID numbers for column names
    /// IDs are unique in the collection
    std::map< std::string, int > m_columnIdForColumnName;

    typedef     std::map< std::string, CollectionColumn* >      ColumnByName;
    /// Map of Attribute CollectionColumn objects using column names as keys.
    ColumnByName        m_attributeColumnForColumnName;
  };
}

#endif
