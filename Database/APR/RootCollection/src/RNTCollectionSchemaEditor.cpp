/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "CollectionBase/CollectionBaseNames.h"
#include "CollectionBase/ICollectionColumn.h"

#include "RNTCollectionSchemaEditor.h"

#include "POOLCore/Exception.h"
#include "RootUtils/APRDefaults.h"

#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeList.h"
#define corENDL coral::MessageStream::endmsg

// for version checks
#include "TROOT.h"
#if ROOT_VERSION_CODE >= ROOT_VERSION( 6, 31, 0 )
#include "ROOT/RNTupleReader.hxx"
#endif
#include "ROOT/RNTuple.hxx"


pool::RootCollection::RNTCollectionSchemaEditor::
RNTCollectionSchemaEditor( RNTCollection& collection,
                           CollectionDescription& description,
                           RNTupleReader& reader
                           ) :
   m_collection( collection ),
   m_description( description ),
   m_reader( reader ),
   m_poolOut("RNTCollectionSchemaEditor")
{ }


pool::RootCollection::RNTCollectionSchemaEditor::
~RNTCollectionSchemaEditor()
{ }


void
pool::RootCollection::RNTCollectionSchemaEditor::
setEventReferenceColumnName( const std::string& columnName )
{
   const std::string oldName = m_description.eventReferenceColumnName();
   renameColumn( oldName, columnName );
}


const pool::ICollectionColumn&
pool::RootCollection::RNTCollectionSchemaEditor::
insertColumn( const std::string& columnName,
              const std::string& columnType,
              const std::string& annotation,
              int maxSize,
              bool sizeIsFixed)
{
   //addTreeBranch( columnName, columnType );
   const ICollectionColumn& column = m_description.insertColumn( columnName, columnType,
                                                                 annotation,
                                                                 maxSize, sizeIsFixed );
   //m_collection.dataEditor().clearRowBuffers();
   return column;
}


const pool::ICollectionColumn&
pool::RootCollection::RNTCollectionSchemaEditor::
insertColumn( const std::string& columnName,
              const std::type_info& columnType,
              const std::string& annotation,
              int maxSize,
              bool sizeIsFixed)
{
   return insertColumn( columnName, columnType.name(), annotation, maxSize, sizeIsFixed );
}


const pool::ICollectionColumn&
pool::RootCollection::RNTCollectionSchemaEditor::
insertTokenColumn( const std::string& columnName,
                   const std::string& annotation )
{
   return insertColumn( columnName, CollectionBaseNames::tokenTypeName, annotation );
}



const pool::ICollectionColumn&
pool::RootCollection::RNTCollectionSchemaEditor::
annotateColumn( const std::string& columnName, const std::string& annotation )
{
   return m_description.annotateColumn( columnName, annotation );
}


void
pool::RootCollection::RNTCollectionSchemaEditor::
dropColumn( const std::string& columnName )
{
   m_description.dropColumn( columnName );
   // MN: only doable before writing
   // m_collection.dataEditor().clearRowBuffers();
}


void
pool::RootCollection::RNTCollectionSchemaEditor::
renameColumn( const std::string& oldName,  const std::string& newName )
{
   m_description.renameColumn( oldName, newName );
   // MN: again, only doable before writing
   //m_collection.dataEditor().clearRowBuffers();
}


void
pool::RootCollection::RNTCollectionSchemaEditor::
addRNTupleField( const std::string& name, const std::string& type_name )
{
   // MN: TODO: implement
   m_poolOut << coral::Debug << "Created Field " <<  name
             << ", Type=" <<  type_name << coral::MessageStream::endmsg;
}


void
pool::RootCollection::RNTCollectionSchemaEditor::
readSchema()
{
   CollectionDescription desc( m_description.name(),
                               m_description.type(),
                               m_description.connection() );
   // clear the description
   m_description = desc;
   bool      foundToken = false;

#if ROOT_VERSION_CODE >= ROOT_VERSION( 6, 31, 0 )
   const auto& rntdesc = m_reader.GetDescriptor();
   for( const auto &f : rntdesc.GetTopLevelFields() ) {
      const std::string field_name = f.GetFieldName();
#else
   const auto rntdesc = m_reader.GetDescriptor();
   for( const auto &f : rntdesc->GetTopLevelFields() ) {
      const std::string field_name = f.GetFieldName();
#endif
      // ignore the index column, it's not a user data
      if( field_name == APRDefaults::IndexColName )
         continue;
      std::string field_type = f.GetTypeName();

      m_poolOut << coral::Debug << "  + field name: " << field_name <<  corENDL;
      m_poolOut << coral::Debug << "    field type: " << field_type <<  corENDL;

      // MN: TODO : may need to fix coral::Attribute to recognize the "new" typenames
      static const std::map< std::string, std::string > typenameConv = {
         { "std::uint64_t", "unsigned long" },
         { "std::uint32_t", "unsigned int" },
         { "std::uint16_t", "unsigned short" },
         { "std::int64_t", "long" },
         { "std::int32_t", "int" },
         { "std::int16_t", "short" } };
      auto it = typenameConv.find( field_type );
      if( it != typenameConv.end() ) {
         m_poolOut << coral::Debug << "Replaced type  " << field_type << " with " << it->second << corENDL;
         field_type = it->second;
      }

      if( (field_name == "Token" || field_name ==  m_description.eventReferenceColumnName())
          and foundToken ) {
         throw pool::Exception( "can't reconstruct Description if more than one Token column",
                                "pool::RNTCollection::readSchema",
                                "RNTCollection" );
      }
      if( field_name ==  m_description.eventReferenceColumnName() ) {
         foundToken = true;
         // do nothing more
      } else if( field_name == "Token" ) {
         m_description.setEventReferenceColumnName( field_name );
         foundToken = true;
      } else {
         m_description.insertColumn( field_name, field_type );
      }
   }
   if( !foundToken ) {
      m_description.setEventReferenceColumnName( "DummyRef" );
   }
}


void
pool::RootCollection::RNTCollectionSchemaEditor::
createRNTuple()
{
   for( int col_id = 0; col_id < m_description.numberOfTokenColumns(); col_id++ ) {
      std::string columnName = m_description.tokenColumn(col_id).name();
      addRNTupleField( columnName, CollectionBaseNames::tokenTypeName );
   }
   for( int col_id = 0; col_id < m_description.numberOfAttributeColumns(); col_id++ ) {
      const ICollectionColumn& column = m_description.attributeColumn(col_id);
      addRNTupleField( column.name(), column.type() );
   }
}
