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


bool
pool::CollectionDescription::equals( const ICollectionDescription& Irhs ) const
{
   return
      isSubsetOf(Irhs) &&
      numberOfColumns() == Irhs.numberOfColumns() &&
      eventReferenceColumnName() == Irhs.eventReferenceColumnName();  //MN: not so sure we need this
}
      

bool
pool::CollectionDescription::isSubsetOf( const ICollectionDescription& Irhs ) const
{
   const CollectionDescription *rhs = dynamic_cast<const CollectionDescription*>(&Irhs);
   if( !rhs ) throw std::runtime_error( "Dynamic cast from ICollectionDescription failed (APR: \" CollectionDescription::equals \" from \" CollectionBase" );
   if( this == rhs ) return true;
   // printOut();  rhs->printOut();

   for( ColumnByName::const_iterator mapIter = m_attributeColumnForColumnName.begin();
        mapIter != m_attributeColumnForColumnName.end(); ++mapIter )
   {
      const CollectionColumn *col = mapIter->second;
      const ICollectionColumn  *rhsCol = rhs->columnPtr( col->name() );
      if( !rhsCol )
         return false;  // rhs does not have that column
      if( col->type() != rhsCol->type() )
         return false;  // column types do not match
      // MN: not checking other column attributes for flexibility
   }

   for( ColumnByName::const_iterator mapIter = m_tokenColumnForColumnName.begin();
        mapIter != m_tokenColumnForColumnName.end(); ++mapIter )
   {
      const CollectionColumn *col = mapIter->second;
      // 
      if( col->name() == eventReferenceColumnName() ) {
         // MN: main Token columns may have different names?
         continue;
      }
      const ICollectionColumn  *rhsCol = rhs->columnPtr( col->name() );
      if( !rhsCol )
         return false;  // rhs does not have that column
      if( col->type() != rhsCol->type() )
         return false;  // column types do not match
      if( rhsCol->name() == rhs->eventReferenceColumnName() )
         return false;   // main Token column name clash
   }

   return true; 
}



bool
pool::CollectionDescription::operator==( const pool::CollectionDescription& rhs ) const
{
  if ( m_name != rhs.m_name ||
       m_type != rhs.m_type ||
       m_connection != rhs.m_connection ||
       m_eventReferenceColumnName != rhs.m_eventReferenceColumnName )
  {
    return false;
  }

  std::map< std::string, int >::const_iterator iColumnId = m_columnIdForColumnName.begin();
  for ( std::map< std::string, int >::const_iterator iColumnIdRhs =
        rhs.m_columnIdForColumnName.begin(); iColumnIdRhs != rhs.m_columnIdForColumnName.end(); 
        ++iColumnIdRhs, ++iColumnId )
  {
    if ( ( iColumnId->first !=  iColumnIdRhs->first ) || ( iColumnId->second != iColumnIdRhs->second ) )
    {
      return false;
    }
  }

  std::map< std::string, pool::CollectionColumn* >::const_iterator iTokenColumn = m_tokenColumnForColumnName.begin();
  for ( std::map< std::string, pool::CollectionColumn* >::const_iterator iColumnRhs =
        rhs.m_tokenColumnForColumnName.begin(); iColumnRhs != 
        rhs.m_tokenColumnForColumnName.end(); ++iColumnRhs, ++iTokenColumn )
  {
    if ( ( iTokenColumn->first !=  iColumnRhs->first ) || ( *(iTokenColumn->second) != *(iColumnRhs->second) ) )
    {
      return false;
    }
  }

  std::map< std::string, pool::CollectionColumn* >::const_iterator iAttributeColumn =
       m_attributeColumnForColumnName.begin();
  for ( std::map< std::string, pool::CollectionColumn* >::const_iterator iColumnRhs =
        rhs.m_attributeColumnForColumnName.begin(); iColumnRhs != 
        rhs.m_attributeColumnForColumnName.end(); ++iColumnRhs, ++iAttributeColumn )
  {
    if ( ( iAttributeColumn->first !=  iColumnRhs->first ) || ( *(iAttributeColumn->second) != *(iColumnRhs->second) ) )
    {
      return false;
    }
  }

  return true;
}


bool
pool::CollectionDescription::operator!=( const pool::CollectionDescription& rhs ) const
{
  return ( ! ( *this == rhs ) );
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
   // If event reference Token column exists then rename it. Otherwise just reset variable.
   if( m_tokenColumnForColumnName.find( eventReferenceColumnName() ) != m_tokenColumnForColumnName.end() )  {
      renameColumn( eventReferenceColumnName(), columnName );
   }
   else {
      m_eventReferenceColumnName = columnName;
   }
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
	      const std::type_info& columnType,
	      const std::string& annotation,
	      int maxSize,
	      bool sizeIsFixed )
{
   return insertColumn( columnName, coral::AttributeSpecification::typeNameForId( columnType ),
			annotation, maxSize, sizeIsFixed );
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


const pool::ICollectionColumn&
pool::CollectionDescription::annotateColumn(
   const std::string& columnName,
   const std::string& annotation )
{
   CollectionColumn* _column = column( columnName, "CollectionDescription::annotateColumn" );
   _column->setAnnotation( annotation );
   return *_column;
}


void 
pool::CollectionDescription::dropColumn( const std::string& columnName )
{
   // Check if description for column already exists and whether it is of type Token or Attribute.
   bool _isTokenColumn = isTokenColumn( columnName, "dropColumn" );
  
   // Delete descripton object for column and update vectors and maps.
   if( _isTokenColumn )	{
      for( std::vector< pool::CollectionColumn* >::iterator iColumn = m_tokenColumns.begin(); 
	          iColumn != m_tokenColumns.end(); ++iColumn )  {
         const std::string& name = (*iColumn)->name();
	      if( name == columnName ) {
	         m_columnIdForColumnName.erase( name );
	         m_tokenColumnForColumnName.erase( name );
	         delete *iColumn;
	         m_tokenColumns.erase( iColumn );
            break;
	      }
      }
   } else {
	   for( std::vector< pool::CollectionColumn* >::iterator iColumn = m_attributeColumns.begin();
            iColumn != m_attributeColumns.end(); ++iColumn ) {
	      const std::string& name = (*iColumn)->name();
	      if( name == columnName ) {
	         m_columnIdForColumnName.erase( name );
	         m_attributeColumnForColumnName.erase( name );
	         delete *iColumn;
            m_attributeColumns.erase( iColumn );
            break;
	      }
	   }
   }
}



//  MN:  FIX - I do not believe this is a full implementation yet!
void 
pool::CollectionDescription::renameColumn( const std::string& oldName, const std::string& newName )
{
   const std::string methodName("renameColumn");
   bool _isTokenColumn = isTokenColumn( oldName, methodName );
   checkNewColumnName( newName, "renameColumn" );

   std::vector< pool::CollectionColumn* >::iterator iColumn;
   if( _isTokenColumn )  {
      iColumn = m_tokenColumns.begin();
      while( (**iColumn).name() != oldName) ++iColumn;
      m_tokenColumnForColumnName.erase( oldName );
      m_tokenColumnForColumnName[ newName ] = *iColumn;
   } else {
      iColumn = m_attributeColumns.begin();
      while( (**iColumn).name() != oldName)	++iColumn;
      m_attributeColumnForColumnName.erase( oldName );
      m_attributeColumnForColumnName[ newName ] = *iColumn;
   }
   m_columnIdForColumnName.erase( oldName );
   m_columnIdForColumnName[ newName ] = (**iColumn).id();
   
   (**iColumn).setName( newName );

   // If column is event reference Token column reset name.
   if( oldName == eventReferenceColumnName() )  {
        m_eventReferenceColumnName = newName;
   }
}


void 
pool::CollectionDescription::changeColumnType( const std::string& columnName, 
                                               const std::string& newType,
                                               int maxSize,
                                               bool sizeIsFixed )
{
   const std::string methodName("changeColumnType");
  // Check that type change is not requested on event reference Token column.
   if( columnName == eventReferenceColumnName() )  {
      std::string errorMsg = "Cannot change the type of the event reference Token column.";
      throw std::runtime_error( errorMsg + " (APR: \" CollectionDescription::" + methodName + " \" from \" CollectionBase" );
   }

   // Check if description for column already exists and whether it is of type Token or Attribute.
   bool _isTokenColumn = isTokenColumn( columnName, methodName );
   int	variablePosition = column( columnName ).id();
   const std::string& annotation = column( columnName ).annotation();

   // Change the column type.
   if( _isTokenColumn )  {
        // Drop existing Token column.
      dropColumn( columnName );
      // Insert Attribute column with new type.
      insertColumn( columnName, newType, annotation, maxSize, sizeIsFixed );
      // retain the old column ID
      setColumnId( column(columnName, methodName), variablePosition );
   }
   else {
      if( newType == CollectionBaseNames::tokenTypeName ) {
	   // Drop existing Attribute column.
	   dropColumn( columnName );
	   // Insert new Token column.
	   insertTokenColumn( columnName, annotation );
	   // retain the old column ID
	   setColumnId( column(columnName, methodName), variablePosition );
   }
   else {
      // Change type of existing Attribute column.
      pool::CollectionColumn* column = m_attributeColumnForColumnName[ columnName ];
      column->setType( newType );
      column->setMaxSize( maxSize );
      column->setSizeIsFixed( sizeIsFixed );
    }
  }
} 


void 
pool::CollectionDescription::changeColumnType( const std::string& columnName, 
                                               const std::type_info& newType,
                                               int maxSize,
                                               bool sizeIsFixed )
{
   changeColumnType( columnName,
		     coral::AttributeSpecification::typeNameForId( newType ),
		     maxSize,
		     sizeIsFixed );
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


bool
pool::CollectionDescription::hasEventReferenceColumn() const
{
   return m_tokenColumnForColumnName.find( eventReferenceColumnName() )
      != m_tokenColumnForColumnName.end();
}


int 
pool::CollectionDescription::numberOfColumns() const
{
   // Return total number of columns
   return  m_attributeColumnForColumnName.size() + m_tokenColumnForColumnName.size();
}


const pool::ICollectionColumn&
pool::CollectionDescription::column( const std::string& name ) const
{
   return *column( name, "column" );
}


// public method without exceptions primarily for checking for column existence
const pool::ICollectionColumn*
pool::CollectionDescription::columnPtr( const std::string& name ) const
{
   std::map< std::string, pool::CollectionColumn* >::const_iterator iColumn;
   iColumn = m_attributeColumnForColumnName.find( name );
   if( iColumn == m_attributeColumnForColumnName.end() ) {
      iColumn = m_tokenColumnForColumnName.find( name );
      if( iColumn == m_tokenColumnForColumnName.end() )  {
         return NULL;
      }
   }
   return iColumn->second;
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
pool::CollectionDescription::tokenColumn( const std::string& columnName ) const
{
  std::map< std::string, pool::CollectionColumn* >::const_iterator iColumn = 
       m_tokenColumnForColumnName.find( columnName );

  if ( iColumn == m_tokenColumnForColumnName.end() )
  {
    std::string errorMsg = "Token column with name `" + columnName + "' does not exist.";
    throw std::runtime_error( errorMsg + " (APR: \" CollectionDescription::tokenColumn \" from \" CollectionBase" );
  }
 
  return *( iColumn->second );
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
pool::CollectionDescription::attributeColumn( const std::string& columnName ) const
{
  std::map< std::string, pool::CollectionColumn* >::const_iterator iColumn = 
       m_attributeColumnForColumnName.find( columnName );
  if ( iColumn == m_attributeColumnForColumnName.end() )
  {
    std::string errorMsg = "Attribute column with name `" + columnName + "' does not exist.";
    throw std::runtime_error( errorMsg + " (APR: \" CollectionDescription::attributeColumn\" from \" CollectionBase" );
  }
 
  return *( iColumn->second );
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


void
pool::CollectionDescription::printOut( ) const
{
   cout << "CollectionDescription: name=" <<  m_name << ", type=" << m_type
        << ", connection=" << m_connection << endl;
   cout << " Event Reference column=" << m_eventReferenceColumnName << endl;
      
   cout << "   Attributes: " << numberOfAttributeColumns() << endl; 
   for( int col_id = 0; col_id < numberOfAttributeColumns(); col_id++ ) {
	   const ICollectionColumn&	column = attributeColumn(col_id);
      cout << "    Attribute " << col_id+1 << ", name=" <<  column.name()
           << ", type=" <<  column.type() << ", maxSize=" <<  column.maxSize()
           << ", isFixed=" << column.sizeIsFixed() 
           << ", annotation=" << column.annotation() << endl;
   }
   cout << "   Tokens: " << numberOfTokenColumns() << endl;
   for( int col_id = 0; col_id < numberOfTokenColumns(); col_id++ ) {
	   const ICollectionColumn&	column = tokenColumn(col_id);
      cout << "    Token " << col_id+1 << ", name=" <<  column.name()
           << ", type=" <<  column.type()
           << ", annotation=" << column.annotation() << endl;
   }
   cout << endl;
}
