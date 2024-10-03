/*
   Copyright (C) 2019-2023 CERN for the benefit of the ATLAS collaboration
 */

/*!
   \file
   \brief Header file for CREST C++ Client Library

   This file contains the CrestClient class.
   (See CrestClientExt.h for CrestClient class extention with additional methods.)
   Here is the method description.
 */

#ifndef CRESTAPI_CRESAPI_H
#define CRESTAPI_CRESAPI_H

#include <string>
#include <map>
#include <list>
#include <iosfwd>
#include <cstdint>
#include "nlohmann/json.hpp"
#include <curl/curl.h>
#include <iostream>
#include <CrestApi/CrestModel.h>
#include <CrestApi/CrestApiBase.h>
#include <CrestApi/CrestRequest.h>

namespace Crest
{

    class CrestClient : public CrestApiBase
    {
    private:
        std::string makeUrl(const std::string &address) const;

        std::string m_PATH = DEFAULT_CREST_API_VERSION;

        inline static const std::string s_TAG_PATH = "/tags";
        inline static const std::string s_ADMIN_PATH = "/admin";
        inline static const std::string s_IOV_PATH = "/iovs";

        inline static const std::string s_IOV_SIZE_PATH = "/size";

        inline static const std::string s_GLOBALTAG_PATH = "/globaltags";
        inline static const std::string s_GLOBALTAG_MAP_PATH = "/globaltagmaps";
        inline static const std::string s_PAYLOAD_PATH = "/payloads";
        inline static const std::string s_MONITORING_PAYLOAD_PATH = "/monitoring/payloads";
        inline static const std::string s_META_PATH = "/meta";
        inline static const std::string s_DATA_PATH = "/data";

        inline static const std::string s_FOLDER_PATH = "/folders";
        inline static const std::string s_RUNINFO_PATH = "/runinfo";
        inline static const std::string s_RUNINFO_LIST_PATH = "/list";

        inline static const std::string s_METHOD_IOVS = "IOVS";
        inline static const std::string s_METHOD_GROUPS = "GROUPS";

        // CREST Server data paths:
        inline static const std::string s_MGMT_PATH = "/mgmt";
        inline static const std::string s_MGMT_INFO_PATH = "/info";
        inline static const std::string s_CREST_CLIENT_VERSION = "5.0";
        inline static const std::string s_MGMT_INFO_PATH_2 = "/actuator/info";

        // ... other constants

        std::string m_host;
        std::string m_port;
        std::string m_prefix = "http://";

        Crest::CrestRequest m_request = Crest::CrestRequest();

/**
 * This is an auxiliary method to read the CREST Server properties.
 * @return JSON with CREST server properties. 
 *
 * <pre>
 *
 * Example:
 *  {
 *   "build":{
 *     "artifact":"crestdb",
 *     "name":"crestdb",
 *     "time":"2023-12-02T15:21:57.045Z",
 *     "version":"4.2.1",
 *     "group":"hep.crest"
 *   }
 *  }
 * </pre>
 */
        nlohmann::json getMgmtInfo();

/**
 * This method returns the full CrestApi version.
 * @return CrestApi library version.
 */
        std::string getClientVersion();

/**
 * This method returns the full CREST Server version.
 * @return CREST server version.
 */
        std::string getCrestVersion();

/**
 * This is an auxiliary method to extract a major version number from 
 * full version string.
 * @return  major version number.
 */
        int getMajorVersion(std::string &str);

/**
 * Auxiliary method to convert string in to JSON object.
 * @param str - string (std::string)
 * @param method - method name, which calls this method. This parameter is used to throw an error exception.
 * @return - JSON object as nlohmann::json
 *
 */    
        nlohmann::json getJson(const std::string &str, const char *method) const;

/**
 * This method removes all XML/HTML tags from a string.
 * (It is an auxiliary method to clear the CREST Server response.)
 * @param xmlBuffer - the text (a std::string ) to be cleared.
 */     
        std::string parseXMLOutput(const std::string_view xmlBuffer) const;

/**
 * This method removes all end of line and carriage return symbols from a string.
 * (It is an auxiliary method to clear the CREST Server response.)
 * @param str - the text (a std::string ) to be cleared.
 */
        std::string removeCR(const std::string &str) const;


    public:

/**
 * CrestClient constructor. 
 * @param host - host name of the CREST Server.
 * @param port - port of the CREST Server.
 * @param check_version - the parameter to switch CREST version checking, if this parameter is true,
 * the CREST version test will be executed.
 */
        CrestClient(const std::string &host, const std::string &port, bool checkVersion = false);

/**
 * CrestClient constructor.
 * @param url - URL address of the CREST Server (with port).
 * @param check_version - the parameter to switch CREST version checking, if this parameter is true,
 * the CREST version test will be executed. <br>
 * 
 * Example:
 * <br>
 * <pre>
 *    std::string url = "http://mvg-test-pc-03.cern.ch:8090";
 *    CrestClient myCrestClient = CrestClient(url);
 * </pre>
 */
        CrestClient(std::string_view url, bool checkVersion = false);
      
        ~CrestClient();

        inline static const bool s_CREST_CLIENT_CHECK = false;

/**
 * This method is a CREST version test. It checks if the major CREST server
 * is equal to the major CrestApi vesrion.
 * If the versions are different an error is thrown.
 * \exception std::runtime_error - error, if the versions are different.
 */
        void checkCrestVersion();

        // Overrides
      
        // GlobaTag methods

/**
 * This method creates a global tag on CREST server.
 * @param globalTag - global tag as GlobalTagDto. 
 *
 * <pre>
 *
 * Example:
 *
 *  nlohmann::json js =
 *      {
 *          {"name", tagname},
 *          {"validity", 0},
 *          {"description", "test"},
 *          {"release", "1"},
 *          {"insertionTime", "2018-12-18T11:32:58.081+0000"},
 *          {"snapshotTime", "2018-12-18T11:32:57.952+0000"},
 *          {"scenario", "test"},
 *          {"workflow", "M"},
 *          {"type", "t"},
 *      };
 *
 *  GlobalTagDto dto = GlobalTagDto();
 *  dto = dto.from_json(js);
 *
 *  myCrestClient.createGlobalTag(dto);
 * </pre>
 */
        void createGlobalTag(GlobalTagDto &globalTag) override;

/**
 * This method finds a global tag by name on the CREST server. Only one global tag should be returned.
 * (This method is an analogue of the find_global_tag method in Python)
 * @param name - global tag name,
 * @return global tag as GlobalTagDto.
 */
        GlobalTagDto findGlobalTag(const std::string &name) override;

/**
 * This method finds the global tags on the CREST server.
 * @param name - global tag name pattern, "%" can be used for any symbols,
 * @param size - page size,
 * @param page - page number,
 * @param sort - sorting order (name:ASC or name:DESC),
 * @return global tag list as GlobalTagSetDto.
 */ 
        GlobalTagSetDto listGlobalTags(const std::string &name, int size, int page, const std::string &sort) override;

/**
 * This method removes a global tag on the CREST server.
 * (This method is an analogue of the remove_global_tag method in Python)
 * @param name  - global tag name
 */
        void removeGlobalTag(const std::string &name) override;
      
        // Tag methods

/**
 * This method creates a tag on the CREST server.
 * @param tag - tag as TagDto. 
 * 
 * <pre>
 *
 *  Example:
 *
 *  nlohmann::json js =
 *  {
 *    {"name", tagname},
 *    {"timeType", "time"},
 *    {"description", "test"},
 *    {"synchronization", "none"},
 *    {"insertionTime", "2018-12-18T11:32:58.081+0000"},
 *    {"modificationTime", "2018-12-18T11:32:57.952+0000"},
 *    {"payloadSpec", "JSON"},
 *    {"lastValidatedTime", 0.},
 *    {"endOfValidity", 0.},
 *  };
 *
 *  TagDto dto = TagDto();
 *  dto = dto.from_json(js);
 *
 *  myCrestClient.createTag(dto);
 *
 * </pre>
 */
        void createTag(TagDto &tag) override;

/**
 * This method finds a tag by the name on the CREST server. 
 * (This method is an analogue of the find_tag method in Python)
 * @param name - tag name
 * @return tag as TagDto object.
 */
        TagDto findTag(const std::string &name) override;

 /**
 * This method returns the tag list as TagSetDto from the CREST server. 
 * (This method is an analogue of the list_tags method in Python)
 * @param name - tag name pattern,
 * @param size - page size,
 * @param page - page number,
 * @param sort - sorting order (name:ASC or name:DESC).
 * @return tag list as TagSetDto object.
 */
        TagSetDto listTags(const std::string &name, int size, int page, const std::string &sort) override;

/**
 * This method removes a tag from the CREST server.
 * (This method is an analogue of the remove_tag method in Python)
 * @param tagName - tag name
 */
        void removeTag(const std::string &name) override;

/**
 * This method gets the number of IOVs for the given tag. 
 * (This method is an analogue of the get_size method in Python)
 * @param tagname - tag name.
 * @return IOV number.
 */
        int getSize(const std::string& tagname) override;

        // TagMeta methods

/**
 * This method creates a tag meta info on the CREST server.
 * @param tag - tag meta info as TagMetaDto. 
 *
 * <pre>
 * Example:
 *
 *  nlohmann::json channel = {{"0", "ATLAS_PREFERRED"}};
 *
 *  nlohmann::json chanList = nlohmann::json::array({channel});
 *
 *  nlohmann::json tagInfo =
 *      {
 *          {"channel_list", chanList},
 *          {"node_description", "description of the node"},
 *          {"payload_spec", "stave:Int32, eta:Int32, mag:Float, base:Float, free:Float"}};
 *
 *  nlohmann::json js =
 *      {
 *          {"tagName", tagname},
 *          {"description", "none"},
 *          {"chansize", 1},
 *          {"colsize", 6},
 *          {"tagInfo", tagInfo.dump()},
 *          {"insertionTime", "2020-12-04"}};
 *
 *  TagMetaDto dto = TagMetaDto();
 *  dto = dto.from_json(js);
 *
 *  myCrestClient.createTagMeta(dto);
 * </pre>
 */
        void createTagMeta(TagMetaDto &tag) override;

/**
 * This method updates a tag meta info on the CREST server.
 * @param tag - tag meta info as TagMetaDto. 
 */
        void updateTagMeta(TagMetaDto &tag) override;

/**
 * This method reads a tag meta info by the tag name from the CREST server.
 * @param name - tag name
 * @return tag meta info as a TagMetaDto.
 */
        TagMetaDto findTagMeta(const std::string &name) override;
      
        // GlobalTagMap methods

/**
 * This method creates a global tag map on the CREST server.
 * @param globalTagMap - the global tag map as GlobalTagMapDto. 
 *
 * <pre>
 * Example:
 *
 *  nlohmann::json js =
 *  {
 *    {"globalTagName", globaltag},
 *    {"record", "testing2"},
 *    {"label", "test2"},
 *    {"tagName", tagname}
 *  };
 *
 *  GlobalTagMapDto globalTagMap = GlobalTagMapDto();
 *  globalTagMap = globalTagMap.from_json(js);
 *
 *  myCrestClient.createGlobalTagMap(globalTagMap);
 * </pre>
 */
        void createGlobalTagMap(GlobalTagMapDto& globalTagMap) override;

/**
 * This method searches for tag mappings using the global tag name or tag name 
 * on the CREST server.
 * (This method is an analogue of the find_global_tag_map method in Python)
 * @param name - name of a global tag or a tag
 * @param xCrestMapMode - search mode (Trace or BackTrace). If it is set as "Trace" the global tag name will be used for searching, otherwise - the tag name.
 * @return global tag map list as GlobalTagMapSetDto.
 */
        GlobalTagMapSetDto findGlobalTagMap(const std::string& name, const std::string& xCrestMapMode) override;

/**
 * This method removes a global tag map on the CREST server.
 * @param name - the global tag name, 
 * @param label - label,
 * @param tagname - tag name.
 */
        void removeGlobalTagMap(const std::string& name, const std::string& record, const std::string& label, const std::string& tagname) override;

        // Iovs

/**
 * This method selects IOVs for a given tagname on the CREST server.
 * The result is an IOV list.
 * (This method is an analogue of the select_iovs method in Python)
 * @param name - tag name,
 * @param since - since time (the beginning of the time interval),
 * @param until - until time (end of the time interval),
 * @param snapshot - snapshot,
 * @param size - page size,
 * @param page - page number,
 * @param sort - sorting order (id.since:ASC or id.since:DESC)
 * @return an IOV list as IovSetDto.
 */
        IovSetDto selectIovs(const std::string &name, uint64_t since, uint64_t until, long snapshot, int size, int page, const std::string &sort) override;

/**
 * This method returns IOV groups for a given tagname on CREST server.
 * The result is an IOV list.
 * (This method is an analogue of the select_groups method in Python)
 * @param name - tag name,
 * @param snapshot - snapshot,
 * @param size - page size,
 * @param page - page number,
 * @param sort - sorting order (id.since:ASC or id.since:DESC)
 * @return an IOV groups as IovSetDto.
 */
        IovSetDto selectGroups(const std::string &name, long snapshot, int size, int page, const std::string &sort) override;
      
        // Payload methods

/**
 * This method stores several payloads in batch mode on the CREST server.
 * (This method is an analogue of the store_data method in Python)
 * @param tag - tag name.
 * @param storeSetJson - iov and payload data as a JSON object.
 * @param payloadFormat - payload format [FILE | JSON].
 * @param objectType - object type.
 * @param compressionType - compression type.
 * @param version - version.
 * @param endTime - end time, if endtime = 0, the server does not use this parameter in the internal check.
 * 
 * <pre>
 * Example how to use these parameters:
 *
 *    std::string tag = "test_MvG3a";
 *    uint64_t endtime = 200;
 *    std::string objectType = "test";
 *    std::string compressionType = "none";
 *    std::string version = "1.0";
 *    std::string payloadFormat = "JSON";
 *    std::string jsonString = R"({"size": 1, "datatype": "data", "format": "StoreSetDto", "resources": [{"since": 1000,"data": "Sample data","streamerInfo": "Info123"}]})";
 *    StoreSetDto storeSetJson = StoreSetDto::from_json(jsonString);
 * </pre>
 */
        void storeData(const std::string &tag,
                       const StoreSetDto &storeSetJson,
		         const std::string &payloadFormat="JSON",
                       const std::string &objectType="none",
                       const std::string &compressionType="none",
                       const std::string &version="1.0",
                       uint64_t endTime=-1) override;

        // Payload retrieval methods

/**
 * This method finds a payload resource associated to the hash on the CREST server. 
 * The payload returns as a string.
 * (This method is an analogue of the get_payload method in Python)
 * @param hash - hash.
 * @return payload as a std::string
 */
        std::string getPayload(const std::string &hash) override;

/**
 *  This method finds a payload meta info for the hash on the CREST server. 
 * (This method is an analogue of the get_payload_meta method in Python)
 * @param hash - hash.
 * @return payload meta info as PayloadDto.
 */
        PayloadDto getPayloadMeta(const std::string &hash) override;
    };

} // namespace Crest

#endif // CRESTAPI_CRESTCLIENT_H
