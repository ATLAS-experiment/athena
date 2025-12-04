/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "CollectionBase/CollectionDescription.h"
#include "CollectionBase/CollectionColumn.h"
#include "CollectionBase/CollectionBaseNames.h"

#include "CoralBase/AttributeSpecification.h"

#include <exception>
#include <sstream>
#include <iostream>
#include <algorithm>

using namespace std;

pool::CollectionDescription::CollectionDescription( const std::string& name,
                                                    const std::string& type,
                                                    const std::string& connection,
                                                    const std::string& eventReferenceColumnName ) 
  : m_name( name ),
    m_type( type ),
    m_connection( connection ),
    m_eventReferenceColumnName( eventReferenceColumnName )
{
  // Insert a Token column for the event references by default.
  if( !m_eventReferenceColumnName.size() )  {
     m_eventReferenceColumnName = CollectionBaseNames::defaultEventReferenceColumnName;
  }
  CollectionDescription::insertTokenColumn( m_eventReferenceColumnName );
}

// NOT a copy constructor
pool::CollectionDescription::
CollectionDescription( const pool::ICollectionDescription& rhs )
{
   CollectionDescription::copyFrom( rhs );
}

// Real copy constructor
pool::CollectionDescription::
CollectionDescription( const pool::CollectionDescription& rhs )
      : ICollectionDescription()
{
   CollectionDescription::copyFrom( rhs );
}


pool::CollectionDescription::~CollectionDescription()
{
   CollectionDescription::clearAll();
}


void
pool::CollectionDescription::
copyFrom( const pool::ICollectionDescription& rhs )
{
   clearAll();
   
   m_name = rhs.name();
   m_type = rhs.type();
   m_connection = rhs.connection();
   m_eventReferenceColumnName = rhs.eventReferenceColumnName();

   for( int col_id = 0; col_id < rhs.numberOfAttributeColumns(); col_id++ ) {
     const ICollectionColumn& column = rhs.attributeColumn(col_id);
     insertColumn(column.name(), column.type(), column.annotation(),
                  column.maxSize(), column.sizeIsFixed());
     setColumnId(column.name(), column.id(), "CollectionDescription");
   }
   for( int col_id = 0; col_id < rhs.numberOfTokenColumns(); col_id++ ) {
     const ICollectionColumn& column = rhs.tokenColumn(col_id);
     insertColumn(column.name(), column.type(), column.annotation(),
                  column.maxSize(), column.sizeIsFixed());
     setColumnId(column.name(), column.id(), "CollectionDescription");
   }
}


void
pool::CollectionDescription::clearAll()
{
   std::map< std::string, pool::CollectionColumn* >::iterator iColumn;
   for( iColumn = m_tokenColumnForColumnName.begin(); iColumn != m_tokenColumnForColumnName.end(); ++iColumn )   {
      // cout << "** Deleting column " << iColumn->first << " @ " << (void*) iColumn->second << " This= " << this << endl;
      delete iColumn->second;
   }
   m_tokenColumnForColumnName.clear();
   m_tokenColumns.clear();

   for( iColumn = m_attributeColumnForColumnName.begin(); iColumn != m_attributeColumnForColumnName.end(); ++iColumn )   {
      delete iColumn->second;
   }
   m_attributeColumnForColumnName.clear();
   m_attributeColumns.clear();
   m_columnIdForColumnName.clear();
}


pool::CollectionDescription&
pool::CollectionDescription::operator=( const pool::ICollectionDescription& rhs )
{
   if( this != &rhs )
      copyFrom( rhs );
   return *this;
}


void 
pool::CollectionDescription::setName( const std::string& name )
{
   m_name = name;
}


void 
pool::CollectionDescription::setType( const std::string& type )
{
  m_type = type;
}


void
pool::CollectionDescription::setConnection( const std::string& connection )
{
  m_connection = connection;
}


void 
pool::CollectionDescription::setEventReferenceColumnName( const std::string& columnName )
{
   if( eventReferenceColumnName() == columnName ) {
      // nothing to do
      return;
   }
   m_eventReferenceColumnName = columnName;
}


// set new column ID
// return the ID
int
pool::CollectionDescription::setColumnId( const std::string& columnName, int id, const std::string& methodName )
{
   return setColumnId( column( columnName, methodName ), id );
}


// set or assign new column ID
// return the ID
int pool::CollectionDescription::setColumnId(pool::CollectionColumn* column, int id) {
  if (id < 0) {
    // find the highest column ID in the collection
    std::map<std::string, int>::const_iterator column_iter = m_columnIdForColumnName.begin();
    while (column_iter != m_columnIdForColumnName.end()) {
      if (id < column_iter->second)
        id = column_iter->second;
      ++column_iter;
    }
    id++;
  }
  column->setId(id);
  m_columnIdForColumnName[column->name()] = id;
  return id;
}


const pool::ICollectionColumn&
pool::CollectionDescription::
insertColumn( const std::string& columnName,
	      const std::string& columnType,
	      const std::string& annotation,
	      int maxSize,
	      bool sizeIsFixed )
{
  if( columnType == CollectionBaseNames::tokenTypeName )  {
     return insertTokenColumn( columnName, annotation );
  }
  const std::string methodName("insertColumn");
  
   // Check if description for column already exists.
  checkNewColumnName( columnName, methodName );

  // Create and record a description object for new column.
  CollectionColumn* column = new CollectionColumn( columnName, columnType, maxSize, sizeIsFixed );
  column->setAnnotation( annotation );
  setColumnId( column );
  m_attributeColumns.push_back( column );
  m_attributeColumnForColumnName[ columnName ] = column;
  return *column;
}


const pool::ICollectionColumn&
pool::CollectionDescription::
insertTokenColumn( const std::string& columnName, const std::string& annotation )
{
   const std::string methodName("insertTokenColumn");

   if( columnName == eventReferenceColumnName() ) {
     std::map<std::string, CollectionColumn*>::const_iterator columnI =
         m_tokenColumnForColumnName.find(columnName);
     if( columnI != m_tokenColumnForColumnName.end() ) {
       // only set annotation for existing EventRef column
       columnI->second->setAnnotation(annotation);
       return *columnI->second;
     }
   }

   // Check if description for this column already exists.
   checkNewColumnName( columnName, methodName );

   // Create and record a description object for new Token column.
   CollectionColumn* column = new CollectionColumn( columnName, CollectionBaseNames::tokenTypeName, 0, true );
   column->setAnnotation( annotation );
   setColumnId( column );
   m_tokenColumns.push_back( column );  
   m_tokenColumnForColumnName[ columnName ] = column;
   return *column;
}


const std::string& 
pool::CollectionDescription::name() const
{
  return m_name;
}


const std::string& 
pool::CollectionDescription::type() const
{
  return m_type;
}


const std::string& 
pool::CollectionDescription::connection() const
{
  return m_connection;
}


const std::string&
pool::CollectionDescription::eventReferenceColumnName() const
{
  return m_eventReferenceColumnName;
}


// internal use protected method (when non-const column is needed). throws exceptions
pool::CollectionColumn *
pool::CollectionDescription::column( const std::string& name, const std::string& method )
{
   std::map< std::string, pool::CollectionColumn* >::const_iterator iColumn;
   iColumn = m_attributeColumnForColumnName.find( name );
   if( iColumn == m_attributeColumnForColumnName.end() ) {
      iColumn = m_tokenColumnForColumnName.find( name );
      if( iColumn == m_tokenColumnForColumnName.end() )  {
         std::string errorMsg = "Column with name `" + name + "' does NOT exist.";
         throw std::runtime_error( errorMsg + " (APR: \" CollectionDescription::" + method + " \" from \" CollectionBase" );
      }
   }
   return iColumn->second;
}
   


const pool::CollectionColumn *
pool::CollectionDescription::column( const std::string& name, const std::string& method ) const
{
   std::map< std::string, pool::CollectionColumn* >::const_iterator iColumn;
   iColumn = m_attributeColumnForColumnName.find( name );
   if( iColumn == m_attributeColumnForColumnName.end() ) {
      iColumn = m_tokenColumnForColumnName.find( name );
      if( iColumn == m_tokenColumnForColumnName.end() )  {
         std::string errorMsg = "Column with name `" + name + "' does NOT exist.";
         throw std::runtime_error( errorMsg + " (APR: \" CollectionDescription::" + method + " \" from \" CollectionBase" );
      }
   }
   return iColumn->second;
}
   

int 
pool::CollectionDescription::numberOfTokenColumns() const
{
   // Return total number of Tokens
   return m_tokenColumnForColumnName.size();
}


const pool::ICollectionColumn&
pool::CollectionDescription::tokenColumn( int columnId ) const
{
   if( columnId >= 0 && columnId < (int)m_tokenColumns.size() )    {
      return *( m_tokenColumns[ columnId ] );
   }
   else{
	 std::ostringstream strm;
	 strm << columnId;
	 std::string errorMsg = "Token column with ID " + strm.str() + " does not exist.";
         throw std::runtime_error( errorMsg + " (APR: \" CollectionDescription::tokenColumn \" from \" CollectionBase" );
   }
}


int 
pool::CollectionDescription::numberOfAttributeColumns() const
{
   // Return total number of Attributes in the collection.
   return m_attributeColumns.size();
}


const pool::ICollectionColumn&
pool::CollectionDescription::attributeColumn( int columnId ) const
{
   if( columnId >= 0 && columnId < (int) m_attributeColumns.size() )  {
      return *( m_attributeColumns[ columnId ] );
   }
   else {
      std::ostringstream strm;
      strm << columnId;
      std::string errorMsg = "Attribute column with ID " + strm.str() + " does not exist.";
      throw std::runtime_error( errorMsg + " (APR: \" CollectionDescription::attributeColumn\" from \" CollectionBase" );
   }
}


void
pool::CollectionDescription::checkNewColumnName( const std::string& name, const std::string& method ) const
{
   if( m_attributeColumnForColumnName.find( name ) != m_attributeColumnForColumnName.end()
       ||  m_tokenColumnForColumnName.find( name ) != m_tokenColumnForColumnName.end() )
   {
      std::string errorMsg = "Column with name `" + name + "' already exists.";
      throw std::runtime_error( errorMsg + " (APR: \" CollectionDescription::" + method + " \" from \" CollectionBase" );
   }
}


// Check if description object for column already exists and whether it is of type Token or Attribute.
bool
pool::CollectionDescription::isTokenColumn( const std::string& columnName, const std::string& method ) const
{
   return column(columnName, method)->type() == CollectionBaseNames::tokenTypeName;
}
