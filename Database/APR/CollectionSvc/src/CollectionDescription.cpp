/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "CollectionSvc/CollectionDescription.h"
#include "CollectionSvc/CollectionColumn.h"
#include "CollectionSvc/CollectionNames.h"

#include "CoralBase/AttributeSpecification.h"

#include <exception>
#include <sstream>
#include <iostream>
#include <algorithm>

using namespace std;

pool::CollectionDescription::CollectionDescription( const std::string& name,
                                                    const pool::DbType& type,
                                                    const std::string& connection )
  : m_name( name ),
    m_type( type ),
    m_connection( connection ),
    m_eventReferenceColumnName( CollectionNames::defaultEventReferenceColumnName )
{
  // Insert a Token column for the event references by default.
  CollectionDescription::insertTokenColumn( m_eventReferenceColumnName );
}

// copy constructor
pool::CollectionDescription::
CollectionDescription( const pool::CollectionDescription& rhs )
{
   CollectionDescription::copyFrom( rhs );
}


pool::CollectionDescription::~CollectionDescription()
{
   CollectionDescription::clearAll();
}


void
pool::CollectionDescription::
copyFrom( const pool::CollectionDescription& rhs )
{
   clearAll();

   m_name = rhs.name();
   m_type = rhs.type();
   m_connection = rhs.connection();
   m_eventReferenceColumnName = rhs.eventReferenceColumnName();

   for( int col_id = 0; col_id < rhs.numberOfAttributeColumns(); col_id++ ) {
     const CollectionColumn& column = rhs.attributeColumn(col_id);
     insertColumn(column.name(), column.type());
     setColumnId(column.name(), column.id());
   }
   for( int col_id = 0; col_id < rhs.numberOfTokenColumns(); col_id++ ) {
     const CollectionColumn& column = rhs.tokenColumn(col_id);
     insertColumn(column.name(), column.type());
     setColumnId(column.name(), column.id());
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
pool::CollectionDescription::operator=( const pool::CollectionDescription& rhs )
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
pool::CollectionDescription::setType( const DbType& type )
{
  m_type = type;
}


void
pool::CollectionDescription::setConnection( const std::string& connection )
{
  m_connection = connection;
}

// set new column ID
// return the ID
int
pool::CollectionDescription::setColumnId( const std::string& columnName, int id )
{
   return setColumnId( column( columnName ), id );
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


const pool::CollectionColumn&
pool::CollectionDescription::
insertColumn( const std::string& columnName, const std::string& columnType )
{
  if( columnType == CollectionNames::tokenTypeName )  {
     return insertTokenColumn( columnName );
  }
  // Check if description for column already exists.
  checkNewColumnName( columnName );

  // Create and record a description object for new column.
  CollectionColumn* column = new CollectionColumn( columnName, columnType );
  setColumnId( column );
  m_attributeColumns.push_back( column );
  m_attributeColumnForColumnName[ columnName ] = column;
  return *column;
}


const pool::CollectionColumn&
pool::CollectionDescription::
insertTokenColumn( const std::string& columnName )
{
   if( columnName == eventReferenceColumnName() ) {
     std::map<std::string, CollectionColumn*>::const_iterator columnI =
         m_tokenColumnForColumnName.find(columnName);
     if( columnI != m_tokenColumnForColumnName.end() ) {
       return *columnI->second;
     }
   }

   // Check if description for this column already exists.
   checkNewColumnName( columnName );

   // Create and record a description object for new Token column.
   CollectionColumn* column = new CollectionColumn( columnName, CollectionNames::tokenTypeName);
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


const pool::DbType&
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
pool::CollectionDescription::column( const std::string& name )
{
   std::map< std::string, pool::CollectionColumn* >::const_iterator iColumn;
   iColumn = m_attributeColumnForColumnName.find( name );
   if( iColumn == m_attributeColumnForColumnName.end() ) {
      iColumn = m_tokenColumnForColumnName.find( name );
      if( iColumn == m_tokenColumnForColumnName.end() )  {
         std::string errorMsg = "Column with name `" + name + "' does NOT exist.";
         throw std::runtime_error( errorMsg + " (APR: \" CollectionDescription \" from \" CollectionSvc" );
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


const pool::CollectionColumn&
pool::CollectionDescription::tokenColumn( int columnId ) const
{
   if( columnId >= 0 && columnId < (int)m_tokenColumns.size() )    {
      return *( m_tokenColumns[ columnId ] );
   }
   else{
	 std::ostringstream strm;
	 strm << columnId;
	 std::string errorMsg = "Token column with ID " + strm.str() + " does not exist.";
         throw std::runtime_error( errorMsg + " (APR: \" CollectionDescription::tokenColumn \" from \" CollectionSvc" );
   }
}


int
pool::CollectionDescription::numberOfAttributeColumns() const
{
   // Return total number of Attributes in the collection.
   return m_attributeColumns.size();
}


const pool::CollectionColumn&
pool::CollectionDescription::attributeColumn( int columnId ) const
{
   if( columnId >= 0 && columnId < (int) m_attributeColumns.size() )  {
      return *( m_attributeColumns[ columnId ] );
   }
   else {
      std::ostringstream strm;
      strm << columnId;
      std::string errorMsg = "Attribute column with ID " + strm.str() + " does not exist.";
      throw std::runtime_error( errorMsg + " (APR: \" CollectionDescription::attributeColumn\" from \" CollectionSvc" );
   }
}


void
pool::CollectionDescription::checkNewColumnName( const std::string& name ) const
{
   if( m_attributeColumnForColumnName.find( name ) != m_attributeColumnForColumnName.end()
       ||  m_tokenColumnForColumnName.find( name ) != m_tokenColumnForColumnName.end() )
   {
      std::string errorMsg = "Column with name `" + name + "' already exists.";
      throw std::runtime_error( errorMsg + " (APR: \" CollectionDescription \" from \" CollectionSvc" );
   }
}
