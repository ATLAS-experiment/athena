/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
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

  /**
    * @brief It compares timestamp in string format.
  */
  static bool compareStrTimestamp(std::string& as, std::string& bs);

  class CrestContainer{
    public:
      /**
       * @brief Constructor of CrestContainer.
       * @param mode Set mode of data. In current moment support two mode: Standard (default mode) and DCS_FULL (DCS mode with full data for each timestamp)
       */	    
      CrestContainer(ModeId mode=ModeId::Standard);
       /**
       * @brief Destructor of CrestContainer.
       */
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
       /**
       * @brief It adds a column to the payload specification.
       * @param name The name of the column.
       * @param type The type of the column in string format.
       *
       * This method adds a column to the payload specification. filling the vector m_payload_spec.
       * The methods with a different signature are just overloads to make it easier to use.
      */      
      void addColumn(const std::string& name, const std::string& type);
      /**
       * @brief It sets the Vector mode of the container.
       */
      bool isVectorPayload();
      /**
       * @brief Set the Vectore mode of the container.
       * @param isVectorPayload True if it is Vector payload.
       */
      void setVectorPayload(bool isVectorPayload);
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
      void addNullRecord(const std::string& name);
      /**
       * @brief It adds a record to the payload.
       * @param name The name of the column.
       * @param number The number of parameters.
       * @param ... The values of the record.
       * 
       * This method adds a record to the payload. It fills the json object m_row.
      */
      void addRecord(const std::string& name,int number, ...);
      /**
       * @brief It associate the payload row to a channel_id.
       * @param channel_id The channel_id to associate the payload row.
       *
       * The method adds the current payload row to the json object m_payload. The row is associated with the channel_id.
      */
      void addData(const std::string& channel_id);
      /**
       * @brief It associate the payload row to a channel_id.
       * @param channel_id The channel_id to associate the payload row.
       * @param data The data to be associated with the channel_id.
       * 
       * The method adds the payload row to the json object m_payload. The row is associated with the channel_id.
       * The data is the data to be associated with the channel_id, and they belong to a previously filled payload row.
      */
      void addExternalData(const std::string& channel_id, const nlohmann::json& data);
      /**
       * @brief It adds an IOV to the json object m_iov_data.
       * @param since The since value of the IOV.
       * 
       * The data is a json object containing all channels stored before, and kept in m_payload. 
      */
      void addIov(const uint64_t since);
      /**
       * @brief Return tag specification in vector format. Elements of vector is pair: name and type.
      */
      const std::vector<std::pair<std::string,TypeId>>& getMPayloadSpec();
      /**
       * @brief Return current payload in json format. 
      */
      const nlohmann::json& getPayload();
     /**
       * @brief Return selected IOV in json format. JSON has two filed: since (for timestamp) and data (for payload).
      */
      const nlohmann::json& getIovData();
     /**
       * @brief Return payload in json format for selected channel id.
       * @param channel_id The channel id.
      */
      const nlohmann::json getPayloadChannel(const std::string& channel_id);

      /**
       * @brief Return payload specification in json format.
      */
      nlohmann::json getPayloadSpec();
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
      /**
       * @brief It clear data the container.
      */
      void clear();
      /**
       * @brief It select timestamp as active.
       * @param since It is selected timastamp.
      */
      void selectIov(const uint64_t since);
      /**
       * @brief Return list of channel id in string format.
      */
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
      void dumpJsonToFile(const nlohmann::json& j, const std::string& filename);
      /**
       * @brief It reads a json file and returns the json object.
      */
      nlohmann::json readJsonFromFile(const std::string& filename, const std::string& spec_filename);
      /**
       * @brief It reads a json object to fill the container.
      */
      std::vector<uint64_t> fromJson(uint64_t since,const nlohmann::json& j);
      /**
       * @brief It return StoreSetDto in json format.
       * @param period It set max interval for DSC data. Value '-1' mean that store all DCS data in one StoreDto. This parameter do not use for standard mode.
      */
      nlohmann::json getStoreSetDto(uint64_t period=-1);
       /**
       * @brief It return minimal timestamp in concainer.
      */
      uint64_t getFirstTime();
      /**
       * @brief It reads a json object, parse it and fill the container.
       * @param values The input json with data.
      */
      void parseData(const nlohmann::json & values);

    private:
      std::vector<std::pair<std::string,TypeId>> m_payload_spec;
      nlohmann::json m_payload={};
      nlohmann::json m_row={};
      nlohmann::json m_iov_data={};
      nlohmann::json m_vector_data=nlohmann::json::array();
      std::map<uint64_t,nlohmann::json> m_full_data;
      ModeId m_modeId;
      bool m_isVectorPayload=false;

      nlohmann::json createRowArray(const nlohmann::json& data_row) const;
      const nlohmann::json& getRow();
      void parseOldFormat(const std::string& colName, const TypeId& typespec, const nlohmann::json & j);
      void readCommonType(uint64_t since, const nlohmann::json &j_in);
      std::vector<uint64_t> readDcsFullType(const nlohmann::json &j_in);
      void setIovData(const nlohmann::json& j);
      void setPayload(const nlohmann::json& j);
      void setPayloadSpec(const nlohmann::json& j);

  };
}
#endif
