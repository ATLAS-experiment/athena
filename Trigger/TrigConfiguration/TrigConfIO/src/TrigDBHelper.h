/* Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration */

#ifndef TRIGCONFIO_TRIGDBHELPER_H
#define TRIGCONFIO_TRIGDBHELPER_H

#include "CoralBase/Blob.h"
#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeList.h"
#include "RelationalAccess/ISessionProxy.h"
#include "RelationalAccess/IQuery.h"

#define BOOST_BIND_GLOBAL_PLACEHOLDERS // Needed to silence Boost pragma message
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"
#include "boost/iostreams/stream.hpp"

#include <vector>
#include <string>
#include <memory>
#include <set>

#include "RelationalAccess/ICursor.h"
#include "RelationalAccess/ITransaction.h"
#include "RelationalAccess/SchemaException.h"

#include "TrigConfBase/MsgStream.h"

/** helper class to store a query */
namespace TrigConf {
   class QueryDefinition {
   public:

      void addToTableList(const std::string & table, const std::string & table_short = "");

      void extendCondition(const std::string & condext);

      template<typename T>
      void extendOutput(const std::string & fieldName);

      template<typename T>
      void extendBinding(const std::string & fieldName);

      template<typename T>
      void setBoundValue(const std::string & fieldName, const T & value);

      std::unique_ptr< coral::IQuery >
      createQuery( coral::ISessionProxy * session );

      void setDataName(const std::string & dataName) {
         m_dataName = dataName;
      }

      const std::string & dataName() {
         return m_dataName;
      }

   private:
      std::vector<std::pair<std::string,std::string>> m_tables{}; // tables needed in the query
      std::string m_condition{""};  // where clause
      coral::AttributeList m_attList;  // select variables
      coral::AttributeList m_bindList; // bound variables
      std::string m_dataName; // the name of the field with the datablob
      std::set<std::string> m_bound; // bound variables that were set
   };

   template<typename T>
   void QueryDefinition::extendOutput(const std::string & fieldName) {
      m_attList.extend<T>( fieldName );
   }

   template<typename T>
   void QueryDefinition::extendBinding(const std::string & fieldName) {
      m_bindList.extend<T>( fieldName );
   }

   template<typename T>
   void QueryDefinition::setBoundValue(const std::string & fieldName, const T & value) {
      m_bindList[fieldName].setValue(value);
      m_bound.insert(fieldName);
   }

   void blobToPtree( const coral::Blob & blob, boost::property_tree::ptree & pt );

   void stringToPtree( const std::string & json_string, boost::property_tree::ptree & pt );

   /** @brief write coral data blob to file
    *
    * This is used by loader classes to write the DB content to file without going through a ptree
    *
    * @param data [in] coral blob to be written
    * @param outFileName [in] name of file to write out the loaded data (if an empty string, no file will be written)
    * @throws TrigConf::JsonFileWritingException if writing was successfull
    */
   void writeRawFile(const coral::Blob & data, const std::string & outFileName);

   /** @brief write string into file
    *
    * This is used by loader classes to write the DB content from CREST to file without going through a ptree
    *
    * @param data [in] crest data string to be written
    * @param outFileName [in] name of file to write out the loaded data (if an empty string, no file will be written)
    * @throws TrigConf::JsonFileWritingException if writing was successfull
    */
   void writeRawFile(const std::string & data, const std::string & outFileName);

}

#endif
