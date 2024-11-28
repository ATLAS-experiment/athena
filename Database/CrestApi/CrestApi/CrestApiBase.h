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

#ifndef CRESTAPI_CRESAPIBASE_H
#define CRESTAPI_CRESAPIBASE_H

#include <string>
#include <map>
#include <list>
#include <iosfwd>
#include <cstdint>
#include "nlohmann/json.hpp"
#include <curl/curl.h>
#include <iostream>
#include <CrestApi/CrestModel.h>

#define DEFAULT_CREST_API_VERSION "/api-v5.0"

namespace Crest
{

class CrestApiBase {

private:

   inline static const std::string s_CREST_CLIENT_VERSION = "5.0";
  
protected:

/**
 *  This method method calculates the hash for string. 
 * @param str - string.
 * @return hash for a given string.
 */
    std::string getHash(std::string_view str);

/**
 *  This method method calculates the hash for a file. 
 * @param file - path to the file with the file name..
 * @return hash for a given file.
 */
    std::string getHashForFile(const std::string &file);

/**
 * This method returns the full CrestApi version.
 * @return CrestApi library version.
 */
    const std::string& getClientVersion();


public:
    virtual ~CrestApiBase() = default;
    // GlobaTag methods

/**
 * This method creates a global tag.
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
    virtual void createGlobalTag(GlobalTagDto& globalTag) = 0;


/**
 * This method finds a global tag by name. Only one global tag should be returned.
 * (This method is an analogue of the find_global_tag method in Python)
 * @param name - global tag name,
 * @return global tag as GlobalTagDto.
 */
    virtual GlobalTagDto findGlobalTag(const std::string& name) = 0;

/**
 * This method finds the global tags.
 * @param name - global tag name pattern, "%" can be used for any symbols,
 * @param size - page size,
 * @param page - page number,
 * @param sort - sorting order (name:ASC or name:DESC),
 * @return global tag list as GlobalTagSetDto.
 */  
    virtual GlobalTagSetDto listGlobalTags(const std::string& name, int size, int page, const std::string& sort) = 0;

/**
 * This method removes a global tag.
 * (This method is an analogue of the remove_global_tag method in Python)
 * @param name  - global tag name
 */ 
    virtual void removeGlobalTag(const std::string& name) = 0;
  
    // Tag methods

/**
 * This method creates a tag.
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
    virtual void createTag(TagDto& tag) = 0;

/**
 * This method finds a tag by the name. 
 * (This method is an analogue of the find_tag method in Python)
 * @param name - tag name
 * @return tag as TagDto object.
 */
    virtual TagDto findTag(const std::string& name) = 0;

 /**
 * This method returns the tag list as TagSetDto. 
 * (This method is an analogue of the list_tags method in Python)
 * @param name - tag name pattern,
 * @param size - page size,
 * @param page - page number,
 * @param sort - sorting order (name:ASC or name:DESC).
 * @return tag list as TagSetDto object.
 */ 
    virtual TagSetDto listTags(const std::string& name, int size, int page, const std::string& sort) = 0;

/**
 * This method removes a tag.
 * (This method is an analogue of the remove_tag method in Python)
 * @param tagName - tag name
 */  
    virtual void removeTag(const std::string& name) = 0;

/**
 * This method gets the number of IOVs for the given tag. 
 * (This method is an analogue of the get_size method in Python)
 * @param tagname - tag name.
 * @return IOV number.
 */
    virtual int getSize(const std::string& tagname) = 0;

    // Tag Meta Info methods

/**
 * This method creates a tag meta info.
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
    virtual void createTagMeta(TagMetaDto& tag) = 0;

/**
 * This method updates a tag meta info.
 * @param tag - tag meta info as TagMetaDto. 
 */
    virtual void updateTagMeta(TagMetaDto& tag) = 0;

/**
 * This method gets a tag meta info by the tag name.
 * @param name - tag name
 * @return tag meta info as a TagMetaDto.
 */
    virtual TagMetaDto findTagMeta(const std::string& name) = 0;


    // IOV methods

/**
 * This method selects IOVs for a given tagname. The result is an IOV list.
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
    virtual IovSetDto selectIovs(const std::string& name, uint64_t since, uint64_t until, long snapshot, int size, int page, const std::string& sort) = 0;

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
    virtual IovSetDto selectGroups(const std::string& name, long snapshot, int size, int page, const std::string& sort) = 0;

    // GlobalTagMap methods

/**
 * This method creates a global tag map.
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
    virtual void createGlobalTagMap(GlobalTagMapDto& globalTagMap) = 0;

/**
 * This method searches for tag mappings using the global tag name or tag name .
 * (This method is an analogue of the find_global_tag_map method in Python)
 * @param name - name of a global tag or a tag
 * @param xCrestMapMode - search mode (Trace or BackTrace). If it is set as "Trace" the global tag name will be used for searching, otherwise - the tag name.
 * @return global tag map list as GlobalTagMapSetDto.
 */
    virtual GlobalTagMapSetDto findGlobalTagMap(const std::string& name, const std::string& xCrestMapMode) = 0;

/**
 * This method removes a global tag map.
 * @param name - the global tag name, 
 * @param label - label,
 * @param tagname - tag name.
 */
    virtual void removeGlobalTagMap(const std::string& name, const std::string& record, const std::string& label, const std::string& tagname) = 0;

    // Payload methods

/**
 * This method stores several payloads in batch mode.
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
    virtual void storeData(const std::string& tag,
                       const StoreSetDto& storeSetJson,
		                 const std::string& payloadFormat,
                       const std::string& objectType,
                       const std::string& compressionType,
                       const std::string& version,
                       uint64_t endTime) = 0;

  
    // Payload retrieval methods

/**
 *  This method finds a payload resource associated to the hash. The payload returns as a string.
 * (This method is an analogue of the get_payload method in Python)
 * @param hash - hash.
 * @return payload as a std::string
 */
    virtual std::string getPayload(const std::string& hash) = 0;

/**
 *  This method finds a payload meta info for the hash. 
 * (This method is an analogue of the get_payload_meta method in Python)
 * @param hash - hash.
 * @return payload meta info as PayloadDto.
 */
    virtual PayloadDto getPayloadMeta(const std::string& hash) = 0;

/**
 * This method returns the full CREST Server version.
 * @return CREST server version.
 */
    virtual std::string getCrestVersion() = 0;


};

} // namespace Crest

#endif // CRESTAPI_CRESTCLIENT_H
