// Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration


#include "./TrigDBHelper.h"

#include "TrigConfIO/Exceptions.h"

#include "RelationalAccess/ISchema.h"
#include <iostream>
#include <format>

std::unique_ptr< coral::IQuery >
TrigConf::QueryDefinition::createQuery( coral::ISessionProxy * session )
{

   std::unique_ptr< coral::IQuery > query( session->nominalSchema().newQuery() );
   
   // set tables
   for( auto & table : m_tables ) {
      query->addToTableList( table.first, table.second );
   }
   
   // ensure that all the requested variables are bound
   for( auto & bindVar : m_bindList ) {
      const std::string & fieldName = bindVar.specification().name();
      if( m_bound.find( fieldName ) == m_bound.end() ) {
         throw std::runtime_error( "Column " + fieldName + " has been bound, but not set to a value" );
      }
   }
      
   // condition
   query->setCondition( m_condition, m_bindList );

   // output
   query->defineOutput( m_attList );
   for( const coral::Attribute & attr : m_attList ) {
      query->addToOutputList(attr.specification().name());
   }
   
   return query;
}

void
TrigConf::QueryDefinition::addToTableList(const std::string & table, const std::string & table_short)
{
   m_tables.emplace_back(table, table_short);
}

void
TrigConf::QueryDefinition::extendCondition(const std::string & condext) {
   if( m_condition.size()>0 && condext.size()>0 && m_condition.back() != ' ' && condext[0] != ' ') {
      m_condition += " ";
   }
   m_condition += condext;
}

void
TrigConf::blobToPtree( const coral::Blob & blob, boost::property_tree::ptree & pt ) {
   boost::iostreams::stream<boost::iostreams::array_source> stream( static_cast<const char*> ( blob.startingAddress()), 
                                                                    blob.size());
   boost::property_tree::read_json(stream, pt);
}

void
TrigConf::stringToPtree( const std::string & json_string, boost::property_tree::ptree & pt ) {
   boost::iostreams::array_source source(json_string.data(), json_string.size());
   boost::iostreams::stream<boost::iostreams::array_source> stream(source);
   boost::property_tree::read_json(stream, pt);
}

void
TrigConf::writeRawFile(const coral::Blob & data, const std::string & outFileName)
{
   try {
      std::ofstream outFile(outFileName, std::ofstream::binary);
      if (!outFile) {
         throw FileWritingException(std::format("Failed to open file {} for writing", outFileName));
      }
      outFile.write( static_cast<const char*> ( data.startingAddress()), data.size() );
      if (!outFile) {
         throw FileWritingException("Failed to write data to file " + outFileName);
      }
      outFile.close();
   } catch (const std::exception& e) {
      throw FileWritingException(std::format("Exception while writing file {} : {}", outFileName, e.what()));
   }
}

void
TrigConf::writeRawFile(const std::string & data, const std::string & outFileName)
{
   try {
      std::ofstream outFile(outFileName, std::ofstream::binary);
      if (!outFile) {
         throw FileWritingException(std::format("Failed to open file {} for writing", outFileName));
      }
      outFile.write(data.data(), data.size());
      if (!outFile) {
         throw FileWritingException("Failed to write data to file " + outFileName);
      }
      outFile.close();
   } catch (const std::exception& e) {
      throw FileWritingException(std::format("Exception while writing file {} : {}", outFileName, e.what()));
   }
}


