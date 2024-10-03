/*
  Copyright (C) 2020-2024 CERN for the benefit of the ATLAS collaboration
*/

#include <CrestApi/CrestModel.h>
#include <iostream>
#include <sstream>
#include <string>
#include <iomanip>
#include "boost/regex.hpp"

json RespPage::to_json() const
{
    return {
        {"size", size},
        {"totalElements", totalElements},
        {"totalPages", totalPages},
        {"number", number}};
}
RespPage RespPage::from_json(const json &j)
{
    RespPage respPage;
    respPage.size = j.value("size", 0);
    respPage.totalElements = j.value("totalElements", 0);
    respPage.totalPages = j.value("totalPages", 0);
    respPage.number = j.value("number", 0);
    return respPage;
}
json GenericMap::to_json() const
{
    json additionalPropertiesJson;
    for (const auto &entry : additionalProperties)
    {
        additionalPropertiesJson[entry.first] = entry.second;
    }

    return {
        additionalPropertiesJson};
}

GenericMap GenericMap::from_json(const json &j)
{
    GenericMap genericMap;

    json additionalPropertiesJson = j.value("additionalProperties", json::object());
    for (auto it = additionalPropertiesJson.begin(); it != additionalPropertiesJson.end(); ++it)
    {
        genericMap.additionalProperties[it.key()] = it.value();
    }

    return genericMap;
}

json CrestBaseResponse::to_json() const
{
    json result = {};
    if (datatype.has_value())
        result["datatype"] = datatype.value();
    result["format"] = getFormat();
    result["size"] = getSize();
    
    // Check if the 'page' optional contains a value
    if (page.has_value())
    {
        // Check if the value of 'page' is not null
        if (!page.value().to_json().is_null())
        {
            result["page"] = page.value().to_json();
        }
    }

    // Check if the 'filter' optional contains a value
    if (filter.has_value())
    {
        // Check if the value of 'filter' is not null
        if (!filter.value().to_json().is_null())
        {
            result["filter"] = filter.value().to_json();
        }
    }

    return result;
}

void CrestBaseResponse::load_from_json(const json &j)
{
    if (j.contains("datatype"))
        datatype = j.value("datatype", "");
    // Check for the presence of "page" key and create an optional
    if (j.contains("page"))
    {
        if (!j["page"].is_null())
        {
            page = std::make_optional((RespPage::from_json(j["page"])));
        }
    }
   
    // Check for the presence of "filter" key and create an optional
    filter = j.contains("filter") ? std::make_optional(GenericMap::from_json(j["filter"])) : std::nullopt;
}
json GlobalTagSetDto::to_json() const
{
    json baseJson = CrestBaseResponse::to_json();
    json jsonResources = json::array();
    for (const auto &resource : resources)
    {
        jsonResources.push_back(((GlobalTagDto)resource).to_json());
    }
    baseJson["resources"] = jsonResources;
    return baseJson;
}

GlobalTagSetDto GlobalTagSetDto::from_json(const json &j)
{
    GlobalTagSetDto globalTagSet;
    globalTagSet.load_from_json(j);
    json jsonResources = j.value("resources", json::array());
    for (auto it = jsonResources.begin(); it != jsonResources.end(); ++it)
    {
        globalTagSet.resources.push_back(GlobalTagDto::from_json(*it));
    }
    
    return globalTagSet;
}

GlobalTagDto::GlobalTagDto() : name(""), validity(0), description(""), release(""),
                               snapshotTime(""), scenario(""), workflow(""), type("T"),
                               snapshotTimeMilli(0), insertionTimeMilli(0)
{
}

GlobalTagDto::GlobalTagDto(const char *l_name, const char *l_description, const char *l_release, const char *l_workflow) : name(l_name), validity(0), description(l_description), release(l_release),
                                                                                                                           snapshotTime(""), scenario(""), workflow(l_workflow), type("T"),
                                                                                                                           snapshotTimeMilli(0), insertionTimeMilli(0)
{
}

json GlobalTagDto::to_json() const
{
    json js = {};
    js["name"] = name;
    js["validity"] = validity;
    js["description"] = description;
    js["release"] = release;
    if (insertionTime.has_value())
        js["insertionTime"] = insertionTime.value();
    js["snapshotTime"] = snapshotTime;
    js["scenario"] = scenario;
    js["workflow"] = workflow;
    js["type"] = type;
    js["snapshotTimeMilli"] = snapshotTimeMilli;
    js["insertionTimeMilli"] = insertionTimeMilli;
    return js;
}
GlobalTagDto GlobalTagDto::from_json(const json &j)
{
    GlobalTagDto globalTag;
    globalTag.name = j.value("name", "");
    globalTag.validity = j.value<uint64_t>("validity", 0);
    globalTag.description = j.value("description", "");
    globalTag.release = j.value("release", "");
    if (j.contains(std::string{"insertionTime"}))
        globalTag.insertionTime = j.value("insertionTime", "");
    globalTag.snapshotTime = j.value("snapshotTime", "");
    globalTag.scenario = j.value("scenario", "");
    globalTag.workflow = j.value("workflow", "");
    globalTag.type = j.value("type", "");
    globalTag.snapshotTimeMilli = j.value("snapshotTimeMilli", 0);
    globalTag.insertionTimeMilli = j.value("insertionTimeMilli", 0);
    return globalTag;
}
json TagDto::to_json() const
{
    json js = {};
    js["name"] = name;
    js["timeType"] = timeType;
    js["payloadSpec"] = objectType;
    js["synchronization"] = synchronization;
    if (insertionTime.has_value())
        js["insertionTime"] = insertionTime.value();
    js["description"] = description;
    js["lastValidatedTime"] = lastValidatedTime;
    js["endOfValidity"] = endOfValidity;
    if (modificationTime.has_value())
        js["modificationTime"] = modificationTime.value();
    return js;
}
TagDto TagDto::from_json(const json &j)
{
    TagDto tag;
    auto it = j.find("name");
    if (it != j.end())
    {
        tag.name = j["name"];
    }
    else
    {
        throw Crest::CrestException("ERROR in TagDto.from_json: JSON contains no tag name.");
    }

    tag.timeType = j.value("timeType", "");
    tag.objectType = j.value("payloadSpec", "");
    tag.synchronization = j.value("synchronization", "none");
    tag.description = j.value("description", "");
    tag.lastValidatedTime = j.value<uint64_t>("lastValidatedTime", 0);
    tag.endOfValidity = j.value<uint64_t>("endOfValidity", 0);
    if (j.contains(std::string{"insertionTime"}))
        tag.insertionTime = j.value("insertionTime", "");
    tag.modificationTime = j.value("modificationTime", "");
    return tag;
}
json TagSetDto::to_json() const
{
    json baseJson = CrestBaseResponse::to_json();
    json jsonResources = json::array();
    for (const auto &resource : resources)
    {
        jsonResources.push_back(((TagDto)resource).to_json());
    }
    baseJson["resources"] = jsonResources;
    return baseJson;
}
TagSetDto TagSetDto::from_json(const json &j)
{
    TagSetDto tagSet;
    tagSet.load_from_json(j);
    json jsonResources = j.value("resources", json::array());
    for (auto it = jsonResources.begin(); it != jsonResources.end(); ++it)
    {
        tagSet.resources.push_back(TagDto::from_json(*it));
    }

    return tagSet;
}
json GlobalTagMapDto::to_json() const
{
    return {
        {"tagName", tagName},
        {"globalTagName", globalTagName},
        {"record", record},
        {"label", label}};
}
GlobalTagMapDto GlobalTagMapDto::from_json(const json &j)
{
    GlobalTagMapDto tagmap;
    tagmap.tagName = j.value("tagName", "");
    tagmap.globalTagName = j.value("globalTagName", "");
    tagmap.record = j.value("record", "none");
    tagmap.label = j.value("label", "none");
    return tagmap;
}

json GlobalTagMapSetDto::to_json() const
{
    json baseJson = CrestBaseResponse::to_json();
    json jsonResources = json::array();
    for (const auto &resource : resources)
    {
        jsonResources.push_back(((GlobalTagMapDto)resource).to_json());
    }
    baseJson["resources"] = jsonResources;
    return baseJson;
}
GlobalTagMapSetDto GlobalTagMapSetDto::from_json(const json &j)
{
    GlobalTagMapSetDto tagMapSet;
    tagMapSet.load_from_json(j);
    json jsonResources = j.value("resources", json::array());
    for (auto it = jsonResources.begin(); it != jsonResources.end(); ++it)
    {
        tagMapSet.resources.push_back(GlobalTagMapDto::from_json(*it));
    }
   
    return tagMapSet;
}

GlobalTagMapSetDto GlobalTagMapSetDto::from_fs_json(const json &j)
{
    GlobalTagMapSetDto tagMapSet;
    tagMapSet.load_from_json(j);
    // int n = j.size();

    for (auto it = j.begin(); it != j.end(); ++it)
    {
        tagMapSet.resources.push_back(GlobalTagMapDto::from_json(*it));
    }

    tagMapSet.datatype = "maps";

    return tagMapSet;
}

void ChannelSetDto::add(std::string id, std::string name)
{
    m_channels.push_back(std::pair<std::string, std::string>(id, name));
}
ChannelSetDto ChannelSetDto::from_json(const json &j)
{
    ChannelSetDto chDto;
    for (unsigned int i = 0; i < j.size(); i++)
    {
        for (auto &el : j[i].items())
        {
            chDto.add(el.key(), el.value());
            break;
        }
    }
    return chDto;
}
json ChannelSetDto::to_json() const
{
    json chJson = json::array();
    for (auto &ch : m_channels)
    {
        json obj = {};
        obj[ch.first] = ch.second;
        chJson.push_back(obj);
    }
    return chJson;
}
void PayloadSpecDto::add(std::string name, std::string type)
{
    m_row.push_back(std::pair<std::string, std::string>(name, type));
}
PayloadSpecDto PayloadSpecDto::from_json(const json &j)
{
    PayloadSpecDto chDto;
	//old format
    if(!j.is_object()&&!j.is_array()){
       std::string regex=R"delim(([^\s,:]*):\s?([^\s,]*),?)delim";
       boost::regex expression(regex);
       boost::smatch what;
       std::string input=j.dump();
       input=boost::regex_replace(input,boost::regex("\""),""); 
       bool match=boost::regex_search(input, what, expression);
       while (match){
         std::string n(what[1]);
         std::string t(what[2]);
	     chDto.add(boost::regex_replace(n, boost::regex("(^[ ]+)|([ ]+$)"),""),boost::regex_replace(t, boost::regex("(^[ ]+)|([ ]+$)"),""));
         input = what.suffix();
         match=boost::regex_search(input, what, expression);
      }
      return chDto;
    }	
    for (unsigned int i = 0; i < j.size(); i++)
    {
        for (auto &el : j[i].items())
        {
            chDto.add(el.key(), el.value());
            break;
        }
    }
    return chDto;
}
json PayloadSpecDto::to_json() const
{
    json chJson = json::array();
    for (auto &ch : m_row)
    {
        json obj = {};
        obj[ch.first] = ch.second;
        chJson.push_back(obj);
    }
    return chJson;
}
json TagInfoDto::to_json() const
{
    json tagInfo = {};
    tagInfo["node_description"] = m_node_description;
    tagInfo["payload_spec"] = m_payload_spec.to_json();
    tagInfo["channel_list"] = m_channel_list.to_json();
    return tagInfo;
}
TagInfoDto TagInfoDto::from_json(const json &j)
{
    std::string node_description = j.value("node_description", "");
    json jsonSpec = j.value("payload_spec", json::array());
    if (jsonSpec.is_string())
        jsonSpec = json::parse(to_string(jsonSpec));
    PayloadSpecDto payload_spec = PayloadSpecDto::from_json(jsonSpec);
    json jsonCh = j.value("channel_list", json::array());
    if (jsonCh.is_string())
        jsonCh = json::parse(to_string(jsonCh));
    ChannelSetDto channel_list = ChannelSetDto::from_json(jsonCh);
    TagInfoDto dto(node_description, payload_spec, channel_list);
    return dto;
}

json TagMetaDto::to_json() const
{
    json tagMeta = {};
    tagMeta["tagName"] = tagName;
    tagMeta["description"] = description;
    tagMeta["chansize"] = tagInfo.getChannelSize();
    tagMeta["colsize"] = tagInfo.getColumnSize();
    tagMeta["tagInfo"] = tagInfo.to_json().dump();
    if (insertionTime.has_value())
        tagMeta["insertionTime"] = insertionTime.value();
    return tagMeta;
}
TagMetaDto TagMetaDto::from_json(const json &j)
{
    TagMetaDto tag;
    tag.tagName = j.value("tagName", "");
    json infoJs = j["tagInfo"];
    if (infoJs.is_string())
    {
        std::istringstream ss(to_string(infoJs));
        std::string st;
        ss >> std::quoted(st);
        infoJs = json::parse(st);
    }
    tag.tagInfo = TagInfoDto::from_json(infoJs);

    tag.description = j.value("description", "");
    if (j.contains(std::string{"insertionTime"}))
        tag.insertionTime = j.value("insertionTime", "");
    return tag;
}

json TagMetaSetDto::to_json() const
{
    json baseJson = CrestBaseResponse::to_json();
    json jsonResources = json::array();
    for (const auto &resource : resources)
    {
        jsonResources.push_back(((TagMetaDto)resource).to_json());
    }
    baseJson["resources"] = jsonResources;
    return baseJson;
}
TagMetaSetDto TagMetaSetDto::from_json(const json &j)
{
    TagMetaSetDto tagSet;
    tagSet.load_from_json(j);
    json jsonResources = j.value("resources", json::array());
    for (auto it = jsonResources.begin(); it != jsonResources.end(); ++it)
    {
        tagSet.resources.push_back(TagMetaDto::from_json(*it));
    }
    
    return tagSet;
}

json IovDto::to_json() const
{
    json iov = {};
    iov["tagName"] = tagName;
    iov["since"] = since;
    if (insertionTime.has_value())
        iov["insertionTime"] = insertionTime.value();
    iov["payloadHash"] = payloadHash;
    return iov;
}

IovDto IovDto::from_json(const json &j)
{
    IovDto iov;
    iov.tagName = j.value("tagName", "");
    iov.since = j.value<uint64_t>("since", 0);

    if (j.contains("insertionTime"))
    {
        if (!j["insertionTime"].is_null())
        {
            iov.insertionTime = j["insertionTime"];
        }
        else
        {
            iov.insertionTime = "";
        }
    }
    else
    {
        iov.insertionTime = "";
    }

    if (j.contains("payloadHash"))
    {
        if (!j["payloadHash"].is_null())
        {
            iov.payloadHash = j["payloadHash"];
        }
        else
        {
            iov.payloadHash = "";
        }
    }
    else
    {
        iov.payloadHash = "";
    }

    return iov;
}

json IovSetDto::to_json() const
{
    json baseJson = CrestBaseResponse::to_json();
    json jsonResources = json::array();
    for (const auto &resource : resources)
    {
        jsonResources.push_back(((IovDto)resource).to_json());
    }
    baseJson["resources"] = jsonResources;
    return baseJson;
}

std::vector<uint64_t> IovSetDto::getListSince(){
  std::vector<uint64_t> v;
  for (IovDto item_iov: resources){
	uint64_t since = item_iov.since;
        v.emplace_back(since);
  }
  return v;
}

IovSetDto IovSetDto::from_json(const json &j)
{
    IovSetDto iovSet;
    iovSet.load_from_json(j);
    json jsonResources = j.value("resources", json::array());
    for (auto it = jsonResources.begin(); it != jsonResources.end(); ++it)
    {
        iovSet.resources.push_back(IovDto::from_json(*it));
    }
   
    return iovSet;
}

IovSetDto IovSetDto::from_fs_json(const json &j)
{
    IovSetDto iovSet;
    iovSet.load_from_json(j);
    // int n = j.size();

    for (auto it = j.begin(); it != j.end(); ++it)
    {
        iovSet.resources.push_back(IovDto::from_json(*it));
    }

    iovSet.datatype = "iovs";

    return iovSet;
}

json StoreDto::to_json() const
{
    json dto = {};
    dto["hash"] = hash;
    dto["since"] = since;
    dto["data"] = data;
    dto["streamerInfo"] = getStreamerInfo().dump();
    return dto;
}
json StoreDto::getStreamerInfo() const
{
    json js = {};
    if (m_app_version.has_value())
        js["app_version"] = m_app_version.value();
    if (m_app_name.has_value())
        js["app_name"] = m_app_name.value();
    return js;
}

StoreDto StoreDto::from_json(const json &j)
{
    StoreDto storeDto;
    storeDto.hash = j.value("hash", "");
    storeDto.since = j.value("since", 0.0);
    storeDto.data = j.value("data", "");
    //do we need get streamerInfo?
    return storeDto;
}

json StoreSetDto::to_json() const
{
    json baseJson = CrestBaseResponse::to_json();
    json jsonResources = json::array();
    for (const auto &resource : resources)
    {
        jsonResources.push_back(((StoreDto)resource).to_json());
    }
    baseJson["resources"] = jsonResources;
    return baseJson;
}
void StoreSetDto::push_back(StoreDto dto)
{
    resources.push_back(dto);
}
void StoreSetDto::clear()
{
    resources.clear();
}

StoreSetDto StoreSetDto::from_json(const json &j)
{
    StoreSetDto storeSetDto;
    json jsonResources = j.value("resources", json::array());
    for (auto it = jsonResources.begin(); it != jsonResources.end(); ++it)
    {
        storeSetDto.push_back(StoreDto::from_json(*it));
    }

    return storeSetDto;
}

// Function to serialize the object to JSON

json PayloadDto::to_json() const
{
    json payloadDto = {};
    payloadDto["hash"] = hash;
    payloadDto["version"] = version;
    payloadDto["objectType"] = objectType;
    payloadDto["objectName"] = objectName;
    payloadDto["compressionType"] = compressionType;
    payloadDto["checkSum"] = checkSum;
    payloadDto["size"] = size;
    if (insertionTime.has_value())
        payloadDto["insertionTime"] = insertionTime.value();
    return payloadDto;
}

// Function to deserialize the object from JSON

PayloadDto PayloadDto::from_json(const json &j)
{
    PayloadDto payloadDto;
    payloadDto.hash = j.value("hash", "");
    payloadDto.version = j.value("version", "");
    payloadDto.objectType = j.value("objectType", "");
    payloadDto.objectName = j.value("objectName", "");
    payloadDto.compressionType = j.value("compressionType", "");
    payloadDto.checkSum = j.value("checkSum", "");
    payloadDto.size = j.value("size", 0);
    if (j.contains(std::string{"insertionTime"}))
        payloadDto.insertionTime = j.value("insertionTime", "");
    return payloadDto;
}

// Function to serialize the object to JSON

json PayloadSetDto::to_json() const
{
    json baseJson = CrestBaseResponse::to_json();
    json jsonResources = json::array();
    for (const auto &resource : resources)
    {
        jsonResources.push_back(((PayloadDto)resource).to_json());
    }
    baseJson["resources"] = jsonResources;
    return baseJson;
}

// Function to deserialize the object from JSON

PayloadSetDto PayloadSetDto::from_json(const json &j)
{
    PayloadSetDto payloadSetDto;
    payloadSetDto.load_from_json(j);
    json jsonResources = j.value("resources", json::array());
    for (auto it = jsonResources.begin(); it != jsonResources.end(); ++it)
    {
        payloadSetDto.resources.push_back(PayloadDto::from_json(*it));
    }

    return payloadSetDto;
}
