/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef RNTCOLLECTION_COLLECTIONSCHEMAEDITOR_H
#define RNTCOLLECTION_COLLECTIONSCHEMAEDITOR_H

#include <string>
#include <vector>
#include <typeinfo>

#include "CollectionBase/CollectionDescription.h"
#include "CollectionBase/ICollectionSchemaEditor.h"

#include "CoralBase/MessageStream.h"


namespace ROOT::Experimental {
   class RNTupleReader;
}
using RNTupleReader = ROOT::Experimental::RNTupleReader;


namespace pool {
   class ICollectionColumn;

   namespace RootCollection {

      class RNTCollection;

      /**
       * @class RNTCollectionSchemaEditor RNTCollectionSchemaEditor.h CollectionBase/RNTCollectionSchemaEditor.h
       *
       * An interface used to define the schema of a collection.
       */
      class RNTCollectionSchemaEditor : public ICollectionSchemaEditor
      {

      friend class RNTCollection;

      public:

         RNTCollectionSchemaEditor( RNTCollection& collection,
                                    CollectionDescription& description,
                                    RNTupleReader& reader  );


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
         virtual const ICollectionColumn&   insertColumn(
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
            const std::string& annotation = "" );


         /// add annotation to column
         virtual  const ICollectionColumn&    annotateColumn(
            const std::string& columnName,
            const std::string& annotation );

         /**
          * Removes a column from the collection.
          *
          * @param columnName Name of column to be removed.
          */
         virtual void dropColumn( const std::string& columnName );

         /**
          * Renames a column of the collection.
          *
          * @param oldName Old name of column.
          * @param newName New name of column.
          */
         virtual void renameColumn( const std::string& oldName, const std::string& newName );

         /// get Collection Description from the RNTupleReader
         void        readSchema();

         /// destructor.
         virtual ~RNTCollectionSchemaEditor();

      protected:
         void        addRNTupleField( const std::string& name, const std::string& type_name );
         void        createRNTuple();

         RNTCollection                  &m_collection;

         CollectionDescription          &m_description;

         RNTupleReader                  &m_reader;

         coral::MessageStream           m_poolOut;
      };
   }
}
#endif

