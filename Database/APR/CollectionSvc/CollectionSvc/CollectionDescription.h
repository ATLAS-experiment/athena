/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef COLLECTIONSVC_COLLECTIONDESCRIPTION_H
#define COLLECTIONSVC_COLLECTIONDESCRIPTION_H

#include "ICollectionDescription.h"

#include <map>
#include <vector>


namespace pool {

  class CollectionColumn;
  
  /** 
   * @class CollectionDescription CollectionDescription.h CollectionSvc/CollectionDescription.h
   *
   * An implementation of the ICollectionDescription interface used to define the properties of
   * a collection to be constructed and to retrieve these properties after construction. The 
   * schema editor of the collection should be used for any modifications to these properties 
   * after construction. 
   */
  class CollectionDescription : virtual public ICollectionDescription
  {
  public:
    /**
     * Constructor that takes as input the minimum amount of properties needed to describe
     * the collection. An event reference Token column is inserted by default but may be renamed 
     * via a call to `setEventReferenceColumnName'.
     *
     * @param name Name of collection.
     * @param type Storage technology type of collection.
     * @param connection Connection to database containing collection.
     * @param eventReferenceColumnName Name of event reference column.
     */
    CollectionDescription( const std::string& name,
                           const std::string& type,
                           const std::string& connection = "",
                           const std::string& eventReferenceColumnName = "" );
    
    /**
     * Copy constructor.
     *
     * @param rhs Collection description object to copy.
     */
    CollectionDescription( const ICollectionDescription& rhs );
    CollectionDescription( const CollectionDescription& rhs );

    /// Default destructor.
    virtual ~CollectionDescription();

    /**
     * Assignment operator.
     *
     * @param rhs source ICollectionDescription object to copy.
     */
    CollectionDescription& operator=( const ICollectionDescription& rhs );

    // Redirect to the copy operator accepting interface reference
    CollectionDescription& operator=( const CollectionDescription& rhs )
    { operator=( (const ICollectionDescription&) rhs ); return *this; }

    // Force the use of the user-defined copy operator (the default one leaks)
    CollectionDescription& operator= (CollectionDescription&& rhs)
    { operator=( (const ICollectionDescription&) rhs ); return *this; }

    // Defaults should work for move.
    CollectionDescription (CollectionDescription&&) = default;

    /**
     * Sets the name of the collection.
     *
     * @param name Name of collection.
     */
    virtual void setName( const std::string& name );

    /**
     * Sets the storage technology type of the collection.
     *
     * @param type Storage technology type of collection.
     */
    virtual void setType( const std::string& type );

    /**
     * Sets the connection to the database containing the collection.
     *
     * @param connection Connection to database where collection is stored.
     */
    virtual void setConnection( const std::string& connection );

    /**
     * Sets the name of the event reference Token column. Otherwise a default name is used.
     *
     * @param columnName Name of event reference Token column.
     */
    virtual void setEventReferenceColumnName( const std::string& columnName );

    /**
     * Adds a new column to the collection.
     *
     * @param columnName Name of new column.
     * @param columnType Data type of new column.
     * @param maxSize Maximum size of column data type (useful for string or blob data types).
     * @param sizeIsFixed Flag indicating whether size of column data type is fixed (useful for string or blob data types).
     */
    virtual const ICollectionColumn&    insertColumn(
       const std::string& columnName, 
       const std::string& columnType,
       int maxSize = 0,
       bool sizeIsFixed = true );

    /**
     * Adds a new column of type pool::Token to the collection.
     *
     * @param columnName Name of new column.
     */
    virtual const ICollectionColumn&    insertTokenColumn(
       const std::string& columnName);
    

    /// Returns the name of the collection and the top level collection fragment.
    virtual const std::string& name() const;

    /// Returns the storage technology type of the collection.
    virtual const std::string& type() const;

    /// Returns the connection to the database containing the collection.
    virtual const std::string& connection() const;

    /** 
     * Returns the name reserved for the event reference Token column. If the name has not
     * been set by the user a default name is returned.
     */
    virtual const std::string& eventReferenceColumnName() const;

    /**
     * Returns the number of Token columns (including the event reference column if it is used)
     */
    virtual int numberOfTokenColumns() const;

    /**
     * Returns a description object for a Token column of the collection, given the position
     * of the column.
     *
     * @param columnId Position of column in associated collection fragment.
     */
    virtual const ICollectionColumn& tokenColumn( int columnId ) const; 

    /** 
     * Returns the number of Attribute columns in the collection.
     */
    virtual int numberOfAttributeColumns() const;

    /**
     * Returns a description object for an Attribute column of the collection, given the position
     * of the column.
     *
     * @param columnId Position of column in associated collection fragment.
     */
    virtual const ICollectionColumn& attributeColumn( int columnId ) const;

    // set column ID, return the ID
    virtual int		setColumnId( const std::string& columnName, int id, const std::string& methodName );
    
 protected:
    // some helper methods for internal use:

    /// make this description a copy of 'rhs'
    virtual void	copyFrom( const ICollectionDescription& rhs );
    
    // clear all internal structures
    virtual void	clearAll();

    // set or assign new column ID
    // return the ID
    virtual int 	setColumnId( pool::CollectionColumn *column, int id = -1 );

    // rise an exception if the column aleready exists
    virtual void 	checkNewColumnName( const std::string& name, const std::string& method ) const;

    // this version includes the 'method name' in the error message
    virtual pool::CollectionColumn* column( const std::string& columnName, const std::string& methodName );
    virtual const pool::CollectionColumn* column( const std::string& columnName, const std::string& methodName ) const;

  private:
    /// Name of the collection
    std::string m_name;

    /// Storage technology type of collection.
    std::string m_type;

    /// Connection to database containing collection.
    std::string m_connection;

    /// Name of event reference column.
    std::string m_eventReferenceColumnName;

    // Token column description objects
    std::vector< pool::CollectionColumn* >	m_tokenColumns;

    /// Attribute column description objects
    std::vector< pool::CollectionColumn* >	m_attributeColumns;

    /// Map of column ID numbers for column names
    /// IDs are unique in the collection
    std::map< std::string, int > m_columnIdForColumnName;

    typedef     std::map< std::string, CollectionColumn* >      ColumnByName;
    /// Map of Token CollectionColumn objects using column names as keys.
    ColumnByName        m_tokenColumnForColumnName;

    /// Map of Attribute CollectionColumn objects using column names as keys.
    ColumnByName        m_attributeColumnForColumnName;
  };
}

#endif

