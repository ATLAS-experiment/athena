/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef COLLECTIONBASE_COLLECTIONDESCRIPTION_H
#define COLLECTIONBASE_COLLECTIONDESCRIPTION_H

#include "CollectionBase/ICollectionDescription.h"

#include <map>


namespace pool {

  class CollectionColumn;
  class CollectionIndex;
  
  /** 
   * @class CollectionDescription CollectionDescription.h CollectionBase/CollectionDescription.h
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
     * via a call to `setEventReferenceColumnName' or `renameColumn()' or removed via a call to 
     * `dropColumn()' after the collection description object has been constructed. 
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

    /**
     * Assignment operator.
     *
     * @param rhs source CollectionDescription object to copy.
     */ 
    CollectionDescription& operator=( const CollectionDescription& rhs )
    { operator=( (const ICollectionDescription&) rhs ); return *this; }

    /**
     * Equality operator.
     *
     * @param rhs Collection description object to compare.
     */
    bool operator==( const CollectionDescription& rhs ) const;

    /**
     * Inequality operator.
     *
     * @param rhs Collection description object to compare.
     */
    bool operator!=( const CollectionDescription& rhs ) const;

    /**
     * Check if both Descriptions have the same columns
     *
     * @param rhs Collection description object to compare.
     */
    virtual bool equals( const ICollectionDescription& rhs ) const;

    /**
     * Check if all columns from this Description are present in the rhs Description
     * and if they have the same type
     *
     * @param rhs Collection description object to compare.
     */
    virtual bool isSubsetOf( const ICollectionDescription& rhs ) const;

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
       const std::string& annotation = "",
       int maxSize = 0,
       bool sizeIsFixed = true );

    /**
     * Adds a new column to the collection.
     * 
     * @param columnName Name of new column.
     * @param columnType Data type of new column.
     * @param maxSize Maximum size of column data type (useful for string or blob data types).
     * @param sizeIsFixed Flag indicating whether size of column data type is fixed (useful for string or blob data types).
     */
    virtual  const ICollectionColumn&   insertColumn(
       const std::string& columnName, 
       const std::type_info& columnType,
       const std::string& annotation = "",
       int maxSize = 0,
       bool sizeIsFixed = true );

    /**
     * Adds a new column of type pool::Token to the collection.
     *
     * @param columnName Name of new column.
     */
    virtual const ICollectionColumn&    insertTokenColumn(
       const std::string& columnName,
       const std::string& annotation = "");
    

    /// add annotation to column
    virtual  const ICollectionColumn&    annotateColumn(
       const std::string& columnName,
       const std::string& annotation ); 


   /**
    * Removes a column from the collection description.
    *
    * @param columnName Name of column to be removed.
    */
    virtual void dropColumn( const std::string& columnName );

    /**
     * Renames a column of the collection description.
     *
     * @param oldName Old name of column.
     * @param newName New name of column.
     */
    virtual void renameColumn( const std::string& oldName, const std::string& newName );

    /**
     * Changes the data type of a column in the collection description. Throws and exception if an attempt is 
     * made to change the data type of the event reference Token column.
     * 
     * @param columnName Name of column whose type is to be changed.
     * @param newType New data type assigned to column.
     * @param maxSize Maximum size of new data type (useful for string and blob types).
     * @param sizeIsFixed Flag indicating whether size of new data type is fixed (useful for string and blob types).
     */
    virtual void changeColumnType( const std::string& columnName,
                                   const std::string& newType,
                                   int maxSize = 0,
                                   bool sizeIsFixed = true );

    /**
     * Changes the data type of a column in the collection description. Throws and exception if an attempt is 
     * made to change the data type of the event reference Token column.
     * 
     * @param columnName Name of column whose type is to be changed.
     * @param newType New data type assigned to column.
     * @param maxSize Maximum size of new data type (useful for string and blob types).
     * @param sizeIsFixed Flag indicating whether size of new data type is fixed (useful for string and blob types).
     */
    virtual void changeColumnType( const std::string& columnName,
                                   const std::type_info& newType,
                                   int maxSize = 0,
                                   bool sizeIsFixed = true );

    /**
     * Creates an index on a column of the collection description. Automatically generates a unique
     * name for the index.
     * 
     * @param columnName Name of column for which index is created.
     * @param isUnique Flag to indicate whether indexed column values must be unique.
     */
    virtual void createIndex( std::string indexName, const std::string& columnName, bool isUnique = false );

    /**
     * Creates an index on one or more columns of the collection description. Automatically generates a
     * unique name for the index.
     *
     * @param columnNames Names of columns for which index is to be created.
     * @param isUnique Flag to indicates whether combination of indexed column values must be unique.
     */
    virtual void createIndex( std::string indexName, const std::vector< std::string >& columnNames, bool isUnique = false );

    /**
     * Removes an index from the collection description, given the name of the column on
     * which the index to be removed is applied.
     *
     * @param columnName Name of column on which index to be removed is applied.
     */
    virtual void dropIndex( const std::string& columnName );

    /**
     * Removes an index from a column of the collection description, given the names of the columns on
     * which the index to be removed is applied.
     *
     * @param columnNames Names of columns on which index to be removed is applied.
     */
    virtual void dropIndex( const std::vector<std::string>& columnNames );


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
     * Indicates whether the top level collection fragment contains the event reference column
     * and is therefore defined as a collection.
     */
    virtual bool hasEventReferenceColumn() const;

    /**
     * Returns the number of columns (including the event reference column if it is used) in 
     * the collection.
     */
    virtual int numberOfColumns() const;

    /**
     * Returns a description object for a column of the collection, given the name of
     * the column.
     *
     * @param columnName Name of column.
     */
    virtual const ICollectionColumn& column( const std::string& columnName ) const;

    /// return pointer to Column or NULL if it's not found (will not throw exceptions)
    virtual const ICollectionColumn* columnPtr( const std::string& columnName ) const;
    
    /**
     * Returns the number of Token columns (including the event reference column if it is used)
     */
    virtual int numberOfTokenColumns() const;

    /**
     * Returns a description object for a Token column of the collection, given the name of 
     * the column.
     *
     * @param columnName Name of column.
     */
    virtual const ICollectionColumn& tokenColumn( const std::string& columnName ) const; 

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
     * Returns a description object for an Attribute column of the collection, given the name of 
     * the column.
     *
     * @param columnName Name of column.
     */
    virtual const ICollectionColumn& attributeColumn( const std::string& columnName ) const; 

    /**
     * Returns a description object for an Attribute column of the collection, given the position
     * of the column.
     *
     * @param columnId Position of column in associated collection fragment.
     */
    virtual const ICollectionColumn& attributeColumn( int columnId ) const;

    /// Returns the Token column description objects.
    const std::vector< pool::CollectionColumn* >& tokenColumns() const { return m_tokenColumns; }

    /// Returns the Attribute column description objects.
    const std::vector< pool::CollectionColumn* >& attributeColumns() const { return m_attributeColumns; }

    /**
     * Returns the number of indices used by the collection.
     */
    virtual int numberOfIndices() const;

    /**
     * Returns a description object for an index of the collection, given the name of the column on which
     * the index is applied.
     *
     * @param columnName Name of column on which index is applied.
     */
    virtual const ICollectionIndex& index( const std::string& columnName ) const;

    /**
     * Returns a description object for an index of the collection, given the names of the columns on 
     * which the index is applied.
     *
     * @param columnNames Names of columns on which index is applied.
     */
    virtual const ICollectionIndex& index( const std::vector<std::string>& columnNames ) const;

    /**
     * Returns a description object for an index of the collection, given the ID number of 
     * the index.
     *
     * @param indexId ID of index.
     */
    virtual const ICollectionIndex& index( int indexId ) const;

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

    // check if the column contains tokens
    virtual bool 	isTokenColumn( const std::string& columnName, const std::string& method ) const;

    // this version includes the 'method name' in the error message
    virtual pool::CollectionColumn* column( const std::string& columnName, const std::string& methodName );
    virtual const pool::CollectionColumn* column( const std::string& columnName, const std::string& methodName ) const;

 public:
    /// print out the description (debugging)
    virtual void                printOut() const;

    
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

    /// Vector of CollectionIndex objects.
    std::vector< CollectionIndex* > m_indices;
  };
}

#endif

