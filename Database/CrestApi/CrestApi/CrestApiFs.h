/*
   Copyright (C) 2019-2024 CERN for the benefit of the ATLAS collaboration
 */

/*!
   \file
   \brief Header file for CREST C++ Client Library

   This file contains the CrestClient class.
   (See CrestClientExt.h for CrestClient class extention with additional methods.)
   Here is the method description.
 */

#ifndef CRESTAPI_CRESFSAPI_H
#define CRESTAPI_CRESFSAPI_H

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

namespace Crest
{

  class CrestFsClient : public CrestApiBase
  {
  private:
    std::string m_root_folder{};
    std::string m_data_folder{};
    std::map<std::string, nlohmann::json> m_data{};
    bool m_isRewrite{};

    // parameters for CREST file storage:

    inline static const std::string s_FS_TAG_PATH = "/tags";
    inline static const std::string s_FS_GLOBALTAG_PATH = "/globaltags";
    inline static const std::string s_FS_DATA_PATH = "/data";

    inline static const std::string s_FS_TAG_FILE = "/tag.json";
    inline static const std::string s_FS_IOV_FILE = "/iovs.json";
    inline static const std::string s_FS_TAGMETAINFO_FILE = "/tagmetainfo.json";

    inline static const std::string s_FS_META_FILE = "/meta.json";
    inline static const std::string s_FS_PALOAD_FILE = "/payload.json";

    inline static const std::string s_FS_GLOBALTAG_FILE = "/globaltag.json";
    inline static const std::string s_FS_MAP_FILE = "/maps.json";

    inline static const std::string s_FS_PATH = "";

    inline static const int s_FS_PREFIX_LENGTH = 3;
    std::string m_currentTag{};

/**
 * Auxiliary method to check if the CrestApi library initialized to work with the file system.
 * It was used to throw an exception if the library method not implemented for file system.
 * @param method_name - method name.
 */
    void checkFsException(const char *method_name);

/**
 * Auxiliary method to get a file as a string.
 * (The old method name -  get_file_string)
 * @param path - path to the file.
 *
 */
    std::string getFileString(const std::string &path); // method to read file as a string

/**
 * Auxiliary method to get a file list in the given directory.
 * (The old method name - get_file_list)
 * @param path - path to the directory.
 *
 */
    void getFileList(const std::string &path);

/**
 * Auxiliary method flush the data to the file system.
 *
 */
    void flush();

  public:

/**
 * CrestFsClient constructor. 
 * @param rewriteIfExist - this parameter sets if the data can be rewritten.
 * @param root_folder - the folder where the data have to be stored.
 */
    CrestFsClient(bool rewriteIfExists, const std::string &root_folder);
    
    ~CrestFsClient();

    // Overrides
    
    // GlobaTag methods

/**
 * This method creates a global tag on the file storage.
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
 * This method finds a global tag by name on the file storage.
 * Only one global tag should be returned.
 * (This method is an analogue of the find_global_tag method in Python)
 * @param name - global tag name,
 * @return global tag as GlobalTagDto.
 */
    GlobalTagDto findGlobalTag(const std::string &name) override;

/**
 * This method finds the global tags on the file storage.
 * @param name - global tag name pattern, "%" can be used for any symbols,
 * @param size - page size,
 * @param page - page number,
 * @param sort - sorting order (name:ASC or name:DESC),
 * @return global tag list as GlobalTagSetDto.
 */
    GlobalTagSetDto listGlobalTags(const std::string &name, int size, int page, const std::string &sort) override;

/**
 * This method removes a global tag.
 * (This method is an analogue of the remove_global_tag method in Python)
 * @param name  - global tag name
 */ 
    void removeGlobalTag(const std::string &name) override;

    // Tag methods

/**
 * This method creates a tag on the file storage.
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
 * This method finds a tag by the name on the file storage. 
 * (This method is an analogue of the find_tag method in Python)
 * @param name - tag name
 * @return tag as TagDto object.
 */
    TagDto findTag(const std::string &name) override;

 /**
 * This method returns the tag list as TagSetDto. 
 * (This method is an analogue of the list_tags method in Python)
 * @param name - tag name pattern,
 * @param size - page size,
 * @param page - page number,
 * @param sort - sorting order (name:ASC or name:DESC).
 * @return tag list as TagSetDto object.
 */
    TagSetDto listTags(const std::string &name, int size, int page, const std::string &sort) override;

/**
 * This method removes a tag from the file storage.
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
    int getSize(const std::string &tagname) override;

    // TagMeta methods

/**
 * This method creates a tag meta info on the file storage.
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
 * This method updates a tag meta info on the file storage.
 * @param tag - tag meta info as TagMetaDto. 
 */
    void updateTagMeta(TagMetaDto &tag) override;

/**
 * This method gets a tag meta info by the tag name from the file storage.
 * @param name - tag name
 * @return tag meta info as a TagMetaDto.
 */
    TagMetaDto findTagMeta(const std::string &name) override;
    
    // Iovs

/**
 * This method selects IOVs for a given tagname from the file storage.
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
 * This method returns IOV groups for a given tagname. The result is an IOV list.
 * (This method is an analogue of the select_groups method in Python)
 * @param name - tag name,
 * @param snapshot - snapshot,
 * @param size - page size,
 * @param page - page number,
 * @param sort - sorting order (id.since:ASC or id.since:DESC)
 * @return an IOV groups as IovSetDto.
 */
    IovSetDto selectGroups(const std::string &name, long snapshot, int size, int page, const std::string &sort) override;

    // GlobalTagMap methods

/**
 * This method creates a global tag map on the file storage.
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
    void createGlobalTagMap(GlobalTagMapDto &globalTagMap) override;

/**
 * This method searches for tag mappings using the global tag name or tag name
 * on the file storage. (This method is an analogue of the find_global_tag_map method in Python)
 * @param name - name of a global tag or a tag
 * @param xCrestMapMode - search mode (Trace or BackTrace). If it is set as "Trace" the global tag name will be used for searching, otherwise - the tag name.
 * @return global tag map list as GlobalTagMapSetDto.
 */
    GlobalTagMapSetDto findGlobalTagMap(const std::string &name, const std::string &xCrestMapMode) override;

/**
 * This method removes a global tag map from the file storage.
 * @param name - the global tag name, 
 * @param label - label,
 * @param tagname - tag name.
 */
    void removeGlobalTagMap(const std::string &name, const std::string &record, const std::string &label, const std::string &tagname) override;

    
    // Payload methods

/**
 * This method stores several payloads in batch mode on the file storage.
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
 * This method finds a payload resource associated to the hash
 * on the file storage. The payload returns as a string.
 * (This method is an analogue of the get_payload method in Python)
 * @param hash - hash.
 * @return payload as a std::string
 */
    std::string getPayload(const std::string &hash) override;

/**
 *  This method finds a payload meta info for the hash on the file storage. 
 * (This method is an analogue of the get_payload_meta method in Python)
 * @param hash - hash.
 * @return payload meta info as PayloadDto.
 */
    PayloadDto getPayloadMeta(const std::string &hash) override;

/**
 * This method returns the full CREST Server version.
 * @return CREST server version.
 */
        std::string getCrestVersion() override;

    
  private:
    std::string getDateAndTime();

/**
 * This is an auxillary method extract first letters from the string (hash).
 * The letter number is in the s_FS_PREFIX_LENGTH variable.
 * This method is used to create a catalogue sub directory in the file storage.
 * @param str - string to extract first letters.
 * @return - string with first letters
 */
    std::string getFirstLetters(const std::string &str);

    // auxiliary methods to extract a single page of the data:
    nlohmann::json getPage(nlohmann::json data, int size, int page);
    std::vector<std::string> getVectorPage(const std::vector<std::string>& data, int size, int page);

    // auxiliary method to get folder names:
    std::vector<std::string> nameList(std::string & folder, bool ascending = true);

    // Auxiliary methods to build the path to the file.
    std::string buildPath(const std::string &path, const std::string &file);

    // Auxiliary methods to sort the JSON array by the key value:
    nlohmann::json sortJson(nlohmann::json js, const std::string & par, bool order);
    nlohmann::json sortIOVJson(nlohmann::json js, bool order);
    
    // auxiliary methods to check if the string corresponds to the mask:
    bool isMatch(const std::string& word, long unsigned int n, const std::string& pattern, long unsigned int m);
    bool isMatch(const std::string& word, const std::string& pattern);

/**
 * This auxiliary method finds all iovs for a given tag name. The result is a JSON object.
 * @param tagname - tag name.
 * @return JSON array with the IOV list. Example:  <br>
 *
 * <pre>
 * IOV list =
 * [
 *    {
 *       "tagName": "CaloOflHadDMCorr2-R123-FTFP-BERT-IOVDEP-01",
 *       "since": 0,
 *       "insertionTime": "2023-09-13T12:53:46+0000",
 *       "payloadHash": "ba26ca6b4e17b0c33b008045c5e703bdf049a1546e731ec7c3d2d39789b1ce1f"
 *   },
 *   ...
 * ]
 * </pre>
 * <b>IOV JSON parameters:</b>
 * <pre>
 *  tagName          string              tag name
 *  since            number              since time parameter
 *  insertionTime    string($date-time)  insertion time
 *  payloadHash      string              payload hash
 * </pre>
 */
    nlohmann::json findAllIovs(const std::string &tagname);

/**
 * This auxiliary method stores a single payload (with since time) on the file system.
 * @param tag - tag name.
 * @param since - since parameter,
 * @param js - a string with a payload or file (the path and the file name) with payload.
 * @param payloadFormat - payload format [FILE | JSON].
 * @param objectType - object type.
 * @param compressionType - compression type.
 * @param version - version.
 * @param endTime - end time, if endtime = 0, the server does not use this parameter in the internal check.
 */   
    void storePayloadDump(const std::string &tag,
                          uint64_t since,
                          const std::string &js,
                          const std::string &payloadFormat,
                          const std::string &objectType,
                          const std::string &compressionType,
                          const std::string &version,
                          const std::string &streamerInfo);
  };

} // namespace Crest

#endif // CRESTAPI_CRESTCLIENT_H
