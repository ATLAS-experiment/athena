/*
   Copyright (C) 2019-2024 CERN for the benefit of the ATLAS collaboration
 */

/*!
   \file
   \brief Header file for CrestContainer class.

   This file contains the CrestContainer class. Main goal of this class is store/load data for CREST. It will support (serealization and deserialization) different format of CREST payload. It is common class for prepare data for put/load payload to/from CREST server.  
 */


#ifndef CREST_COND_CONTAINER_H
#define CREST_COND_CONTAINER_H
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <utility>
#include <map>
#include "nlohmann/json.hpp"

namespace Crest {
  enum class TypeId {
    Bool, UChar, Int16, UInt16,
    Int32, UInt32, UInt63, Int64,
    Float, Double, String255, String4k,
    String64k, String16M, String128M, Blob64k, Blob16M, Blob128M, TypeIdCount
  };
  const static std::map<TypeId, std::string> s_typeToString = { 
    { TypeId::Bool, "Bool" },
    { TypeId::UChar, "UChar" },
    { TypeId::Int16, "Int16" },
    { TypeId::UInt16, "UInt16" },
    { TypeId::Int32, "Int32" },
    { TypeId::UInt32, "UInt32" },
    { TypeId::UInt63, "UInt63" },
    { TypeId::Int64, "Int64" },
    { TypeId::Float, "Float" },
    { TypeId::Double, "Double" },
    { TypeId::String255, "String255" },
    { TypeId::String4k, "String4k" },
    { TypeId::String64k, "String64k" },
    { TypeId::String16M, "String16M" },
    { TypeId::String128M,"String128M"},
    { TypeId::Blob64k, "Blob64k" },
    { TypeId::Blob16M, "Blob16M" },
    { TypeId::Blob128M, "Blob128M" } 
  };

  enum class ModeId{
    Standard, DCS, DCS_FULL
  };
  class CrestContainer{
    private:
      std::vector<std::pair<std::string,TypeId>> m_payload_spec;
      nlohmann::json m_payload={};
      nlohmann::json m_row={};
      nlohmann::json m_iov_data={};
      nlohmann::json m_vector_data=nlohmann::json::array();
      nlohmann::json m_full_data={};
      ModeId m_modeId;
      bool m_isVectorPayload=false;

      void validatePayloadSize(const nlohmann::json &data) const;
      nlohmann::json createRowArray(const nlohmann::json& data_row) const;
      const nlohmann::json& getRow();

    public:
      CrestContainer();
      ~CrestContainer();
      /**
       * @brief It adds a column to the payload specification.
       * @param name The name of the column.
       * @param type The type of the column.
       * 
       * This method adds a column to the payload specification. filling the vector m_payload_spec.
       * The methods with a different signature are just overloads to make it easier to use.
      */
      void addColumn(const std::string& name, TypeId type);
      void addColumn(const std::string& name, const char* type);
      void addColumn(const std::string& name, uint32_t type);

      /**
       * @brief It sets the mode of the container.
       */
      bool isVectorPayload();
      void setVectorPayload(bool isVectorPayload);

      Crest::TypeId stringToTypeId(const std::string& type) const;
      Crest::TypeId intToTypeId(int value) const;

      /**
       * @brief It adds row data to vector.
       *
       * This method should use for Vector type of data. It should call after all data of row is added. 
      */
      void putRow2Vector();

       /**
       * @brief It adds a null value to the payload.
       * @param name The name of the column.
       *
       * This method adds a null to the payload. It fills the json object m_row.
      */
      void addNullRecord(const char* name);

      /**
       * @brief It adds a record to the payload.
       * @param name The name of the column.
       * @param number The number of parameters.
       * @param ... The values of the record.
       * 
       * This method adds a record to the payload. It fills the json object m_row.
      */
      void addRecord(const char* name,int number, ...);
      /**
       * @brief It associate the payload row to a channel_id.
       * @param channel_id The channel_id to associate the payload row.
       *
       * The method adds the current payload row to the json object m_payload. The row is associated with the channel_id.
      */
      void addData(const char* channel_id);
      /**
       * @brief It associate the payload row to a channel_id.
       * @param channel_id The channel_id to associate the payload row.
       * @param data The data to be associated with the channel_id.
       * 
       * The method adds the payload row to the json object m_payload. The row is associated with the channel_id.
       * The data is the data to be associated with the channel_id, and they belong to a previously filled payload row.
      */
      void addExternalData(const char* channel_id, const nlohmann::json& data);
      /**
       * @brief It adds an IOV to the json object m_iov_data.
       * @param since The since value of the IOV.
       * 
       * The data is a json object containing all channels stored before, and kept in m_payload. 
      */
      void addIov(const uint64_t since);

      const std::vector<std::pair<std::string,TypeId>>& getMPayloadSpec();

      template<typename T> T getRecord(const std::string& name) 
      {
        for (const auto& column : m_payload_spec) {
          if (column.first == name) {
              try {
                  return m_row.at(name).get<T>();
              } catch (nlohmann::json::exception& e) {
                  throw std::runtime_error("JSON exception for key: " + name);
              }
          }
        }
        throw std::runtime_error("Column name not found or type mismatch.");
      }

      const nlohmann::json& getPayload();
      const nlohmann::json& getIovData();

      const nlohmann::json& getPayloadChannel(const char* channel_id);


      void setIovData(const nlohmann::json& j);
      void setPayload(const nlohmann::json& j);
      // return the payload spec as a copy
      nlohmann::json getPayloadSpec();
      void setPayloadSpec(const nlohmann::json& j);

      /**
       * @brief It encodes the input string to base64.
       */
      static std::string base64_encode(const uint8_t* bytes_to_encode, unsigned int in_len);
      /**
       * @brief It decodes the input string from base64.
       */
      static std::vector<unsigned char> base64_decode(const std::string& encodedData);
      /**
       * @brief It returns the index of the column with the given name.
       * @param name The name of the column.
       * It checks the payload spec array to get the index back.
      */
      int getColumnIndex(const std::string& name);
      /**
       * @brief It reinitializes the containers.
      */
      void flush();
      void clear();
      void selectIov(const uint64_t since);
      std::vector<std::string> channelIds();
      /**
       * @brief It returns the json representation of the container.
      */
      std::string getJsonPayload();
      /**
       * @brief It returns the json representation of the container.
      */
      std::string getJsonIovData();
      /**
       * @brief It creates a file with the json representation of the container.
      */
      void dump_json_to_file(const nlohmann::json& j, const std::string& filename);
      /**
       * @brief It reads a json file and returns the json object.
      */
      nlohmann::json read_json_from_file(const std::string& filename, const std::string& spec_filename);
      /**
       * @brief It reads a json object to fill the containers.
      */
      void from_json(const uint64_t since, const nlohmann::json& j);

      void parseOldFormat(std::string& colName, TypeId& typespec,const nlohmann::json & j);
  };
}
#endif
