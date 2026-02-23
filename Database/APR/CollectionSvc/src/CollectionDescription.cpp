/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "CollectionSvc/CollectionDescription.h"
#include "CollectionSvc/CollectionColumn.h"

#include "CoralBase/AttributeSpecification.h"

#include <exception>
#include <sstream>
#include <iostream>
#include <algorithm>


const pool::CollectionColumn  pool::CollectionDescription::m_tokenColumn( "Token", "Token" );


pool::CollectionDescription::CollectionDescription( const std::string& name,
                                                    const pool::DbType& type,
                                                    const std::string& connection )
  : m_name( name ),
    m_type( type ),
    m_connection( connection )
{ }

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

   for( int col_id = 0; col_id < rhs.numberOfAttributeColumns(); col_id++ ) {
     const CollectionColumn& column = rhs.attributeColumn(col_id);
     insertColumn(column.name(), column.type());
     setColumnId(column.name(), column.id());
   }
}


void
pool::CollectionDescription::clearAll()
{
   for( auto &iColumn : m_attributeColumnForColumnName ) {
      delete iColumn.second; iColumn.second = nullptr;
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
   auto iColumn = m_attributeColumnForColumnName.find( columnName );
   if( iColumn == m_attributeColumnForColumnName.end() ) {
      std::string errorMsg = "Attribute column with name `" + columnName + "' does NOT exist.";
      throw std::runtime_error( errorMsg + " (APR: \" CollectionDescription \" from \" CollectionSvc" );
   }
   return setColumnId( iColumn->second, id );
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
  if( columnType == tokenColumn().type() )  {
	   std::string errorMsg = "Adding additional Token columns is not supported anymore.";
      throw std::runtime_error( errorMsg + " (APR: \" CollectionDescription::insertColumn \" from \" CollectionSvc" );

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
       ||  name == tokenColumn().name() )
   {
      std::string errorMsg = "Column with name `" + name + "' already exists.";
      throw std::runtime_error( errorMsg + " (APR: \" CollectionDescription \" from \" CollectionSvc" );
   }
}
