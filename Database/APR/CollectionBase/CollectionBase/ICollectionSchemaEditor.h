/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef COLLECTIONBASE_ICOLLECTIONSCHEMAEDITOR_H
#define COLLECTIONBASE_ICOLLECTIONSCHEMAEDITOR_H

#include <string>
#include <vector>
#include <typeinfo>


namespace pool {
   class ICollectionColumn;
   
  /** 
   * @class ICollectionSchemaEditor ICollectionSchemaEditor.h CollectionBase/ICollectionSchemaEditor.h
   *
   * An interface used to define the schema of a collection.
   */
  class ICollectionSchemaEditor
  {
  public:
    /**
     * Sets the name of the event reference Token column. Otherwise a default name is used.
     *
     * @param columnName Name of event reference Token column.
     */
    virtual void setEventReferenceColumnName( const std::string& columnName ) = 0;

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
       bool sizeIsFixed = true ) = 0;
    
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
       bool sizeIsFixed = true ) = 0;

    /**
     * Adds a new column of type pool::Token to the collection.
     *
     * @param columnName Name of new column.
     */
     virtual  const ICollectionColumn&    insertTokenColumn(
       const std::string& columnName,
       const std::string& annotation = "") = 0;


    /// add annotation to column
    virtual  const ICollectionColumn&    annotateColumn(
       const std::string& columnName,
       const std::string& annotation ) = 0; 
 

    /**
     * Removes a column from the collection.
     *
     * @param columnName Name of column to be removed.
     */
    virtual void dropColumn( const std::string& columnName ) = 0;

    /**
     * Renames a column of the collection.
     *
     * @param oldName Old name of column.
     * @param newName New name of column.
     */
    virtual void renameColumn( const std::string& oldName, const std::string& newName ) = 0;

    // TODO protected: 
    /// Empty destructor.
    virtual ~ICollectionSchemaEditor() {}
  };
}

#endif

