/*
   Copyright (C) 2019-2025 CERN for the benefit of the ATLAS collaboration
 */

#ifndef CREST_DTOS_HPP
#define CREST_DTOS_HPP

#include <string>
#include <vector>
#include <optional>
#include "nlohmann/json.hpp"
#include <CrestApi/CrestException.h>

#include <optional>

using json = nlohmann::json;

class RespPage
{
public:
    int size{};
    int64_t totalElements{};
    int totalPages{};
    int number{};

    json to_json() const;
    static RespPage from_json(const json &j);
};

class GenericMap
{
public:
    std::map<std::string, std::string> additionalProperties;

    json to_json() const;

    static GenericMap from_json(const json &j);
};

class CrestBaseResponse
{
private:
public:
    virtual ~CrestBaseResponse() = default;
    std::optional<std::string> datatype;
    std::optional<RespPage> page;
    std::optional<GenericMap> filter;
    json to_json() const;
    void load_from_json(const json &j);
    virtual const char* getFormat() const =0;
    virtual int64_t getSize() const =0;
};

class GlobalTagDto
{
public:
    std::string name;
    int64_t validity{};
    std::string description;
    std::string release;
    std::optional<std::string> insertionTime;
    std::string snapshotTime;
    std::string scenario;
    std::string workflow;
    std::string type;
    int64_t snapshotTimeMilli{};
    int64_t insertionTimeMilli{};

    // Ctor
    GlobalTagDto(const char* _name, const char* _description, const char* _release, const char* _workflow);

    // Default Ctor
    GlobalTagDto();

    json to_json() const ;

    static GlobalTagDto from_json(const json &j);
};

class GlobalTagSetDto : public CrestBaseResponse
{
public:
    std::vector<GlobalTagDto> resources;
    const char* getFormat() const {return "GlobalTagSetDto";}
    int64_t getSize() const{return resources.size();}

    json to_json() const;

    static GlobalTagSetDto from_json(const json &j);
};

/**
 * @brief The TagDto class
 * It contains all fields of the TagDto class from the CREST API.
 * When using this to create a new Tag you don't need to set insertion and modification times.
 */
class TagDto
{
public:
    std::string name;
    std::string timeType;
    std::string objectType;
    std::string synchronization;
    std::string description;
    uint64_t lastValidatedTime{};
    uint64_t endOfValidity{};
    std::optional<std::string> insertionTime;
    std::optional<std::string> modificationTime;
    // Ctor
    TagDto(std::string _name, std::string _timeType, std::string _description): name(_name), timeType(_timeType), 
        objectType(""), 
        synchronization("none"), 
        description(_description),
        lastValidatedTime(0), endOfValidity(0){
        
    }
    // Default Ctor
    TagDto(): name(""), timeType(""), objectType(""), synchronization("none"), description(""), lastValidatedTime(0), endOfValidity(0) {
   
    }
    json to_json() const;
    static TagDto from_json(const json &j);
};

class TagSetDto : public CrestBaseResponse
{
public:
    std::vector<TagDto> resources;
    const char* getFormat() const {return "TagSetDto";}
    int64_t getSize() const{return resources.size();}

    json to_json() const;

    static TagSetDto from_json(const json &j);
};


class GlobalTagMapDto
{
public:
    std::string tagName;
    std::string globalTagName;
    std::string record;
    std::string label;
    // Ctor
    GlobalTagMapDto(const std::string& tagName, const std::string& globalTagName, const std::string& record):
        tagName(tagName), globalTagName(globalTagName), record(record), label("none"){
        
    }
    // Default Ctor
    GlobalTagMapDto(): tagName(""), globalTagName(""), record("none"), label("none"){
        
    }

    json to_json() const;
    static GlobalTagMapDto from_json(const json &j);
};

class GlobalTagMapSetDto : public CrestBaseResponse
{
public:
    std::vector<GlobalTagMapDto> resources;
    const char* getFormat() const {return "GlobalTagMapSetDto";}
    int64_t getSize() const{return resources.size();}

    json to_json() const;
    static GlobalTagMapSetDto from_json(const json &j);

    static GlobalTagMapSetDto from_fs_json(const json &j);
};

class ChannelSetDto{
private:
    std::vector< std::pair<std::string,std::string> > m_channels;
public:
    ChannelSetDto& operator=(const ChannelSetDto&) = default;
    ChannelSetDto& operator=(ChannelSetDto&&) = default;

    ChannelSetDto(){}
    ChannelSetDto(ChannelSetDto const& copy):m_channels(copy.getChannels()){}
    void add(const std::string& id,const std::string& name);
    const std::vector< std::pair<std::string,std::string> >& getChannels() const {return m_channels;}
    size_t getSize() const {return m_channels.size();}
    json to_json() const;
    static ChannelSetDto from_json(const json &j);
};

class PayloadSpecDto{
private:
    std::vector< std::pair<std::string,std::string> > m_row;
public:
    PayloadSpecDto& operator=(const PayloadSpecDto&) = default;
    PayloadSpecDto& operator=(PayloadSpecDto&&) = default;
    PayloadSpecDto(){}
    PayloadSpecDto(PayloadSpecDto const& copy):m_row(copy.getColumns()){}
    const std::vector< std::pair<std::string,std::string> >& getColumns()  const {return m_row;}
    size_t getSize() const {return m_row.size();}
    void add(const std::string& name,const std::string& type);
    json to_json() const;
    static PayloadSpecDto from_json(const json &j);
};

class TagInfoDto{
private:
    std::string m_node_description;
    PayloadSpecDto m_payload_spec;
    ChannelSetDto m_channel_list;
public:
    TagInfoDto(const std::string& description):m_node_description(description){
    }
    TagInfoDto(const std::string& description,PayloadSpecDto& payload_spec,ChannelSetDto& channel_list):
        m_node_description(description),m_payload_spec(payload_spec),m_channel_list(channel_list){
    }
    void setPayloadSpec(PayloadSpecDto& spec){
        m_payload_spec=spec;
    }
    void setChannel(ChannelSetDto& ch){
        m_channel_list=ch;
    }
    const PayloadSpecDto& getPayloadSpec()
    {
        return m_payload_spec;
    }
    const ChannelSetDto& getChannels()
    {
        return m_channel_list;
    }
    const std::string& getFolderDescription(){ return m_node_description;}
    size_t getChannelSize() const {return m_channel_list.getSize();}
    size_t getColumnSize() const {return m_payload_spec.getSize();}
    json to_json() const;
    static TagInfoDto from_json(const json &j);

};


class TagMetaDto
{
public:
    std::string tagName;
    std::string description;
    std::optional<std::string> insertionTime;
    TagInfoDto tagInfo;
    // Ctor
    TagMetaDto(const std::string& tagName, const std::string& description,  const TagInfoDto&  info):
        tagName(tagName), description(description),  tagInfo(info){
    }
    // Default Ctor
    TagMetaDto(): tagName(""), description(""), tagInfo(""){
    }

    json to_json() const;
    static TagMetaDto from_json(const json &j);
};

class TagMetaSetDto : public CrestBaseResponse
{
public:
    std::vector<TagMetaDto> resources;
    const char* getFormat() const {return "TagMetaSetDto";}
    int64_t getSize() const{return resources.size();}

    json to_json() const;
    static TagMetaSetDto from_json(const json &j);
};

class IovDto
{
public:
    std::string tagName;
    uint64_t since{};
    std::optional<std::string> insertionTime;
    std::string payloadHash;
    // Ctor
    IovDto(const std::string& tagName, uint64_t since, const std::string& payloadHash):
        tagName(tagName), since(since), payloadHash(payloadHash) {
    }
    // Default Ctor
    IovDto(): tagName(""), since(0), payloadHash("") {
    }

    json to_json() const;
    static IovDto from_json(const json &j);
};

class IovSetDto : public CrestBaseResponse
{
public:
    std::vector<IovDto> resources;
    const char* getFormat() const {return "IovSetDto";}
    int64_t getSize() const{return resources.size();}
    std::vector<uint64_t> getListSince();
    json to_json() const;
  
    static IovSetDto from_json(const json &j);
  
    static IovSetDto from_fs_json(const json &j);
};

class StoreDto
{
    std::optional<std::string> m_app_name;
    std::optional<std::string> m_app_version;
public:
    uint64_t since{};
    std::string hash;
    std::string data;
    std::optional<std::string> insertionTime;
    StoreDto(uint64_t l_since,  const std::string& l_data):
        since(l_since), hash(""), data(l_data){

    }
    // Ctor
    StoreDto(uint64_t since, const std::string& hash, const std::string& data):
        since(since), hash(hash), data(data){

    }
    // Default Ctor
    StoreDto(): since(0), hash(""), data(""){
    }

    void setAppVersion (const char* str){m_app_version=str;}
    void setAppName (const char* str){m_app_name=str;}
    json getStreamerInfo() const;
    json to_json() const;

    static StoreDto from_json(const json &j);
};

class StoreSetDto : public CrestBaseResponse
{
public:
    std::vector<StoreDto> resources;
    const char* getFormat() const {return "StoreSetDto";}
    int64_t getSize() const{return resources.size();}

    void push_back(const StoreDto & dto);
    void clear();
    json to_json() const;

    static StoreSetDto from_json(const json &j);
};

class PayloadDto 
{
public:
    std::string hash;
    std::string version;
    std::string objectType;
    std::string objectName;
    std::string compressionType;
    std::string checkSum;
    int size{};
    std::optional<std::string> insertionTime;

    // Function to serialize the object to JSON

    json to_json() const;

    // Function to deserialize the object from JSON

    static PayloadDto from_json(const json &j);
};

class PayloadSetDto : public CrestBaseResponse
{
public:
    std::vector<PayloadDto> resources;
    const char* getFormat() const {return "PayloadSetDto";}
    int64_t getSize() const{return resources.size();}

    // Function to serialize the object to JSON
    
    json to_json() const;

    // Function to deserialize the object from JSON
    
    static PayloadSetDto from_json(const json &j);
};

#endif // CREST_DTOS_HPP
