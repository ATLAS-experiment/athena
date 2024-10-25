/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
// @file CrestFunctions.cxx
// Implementation for CrestFunctions utilities
// @author Shaun Roe
// @date 1 July 2019

#include "CrestFunctions.h"
#include "CrestApi/CrestApi.h"
#include "CrestApi/CrestApiFs.h"
#include <iostream>
#include <exception>
#include <regex>
#include "IOVDbStringFunctions.h"
#include <string>
#include <algorithm>
#include <map>

namespace IOVDbNamespace{

  CrestFunctions::CrestFunctions(const std::string & crest_path){
    setURLBase(crest_path);

    const std::string prefix1 = "http://";
    const std::string prefix2 = "https://";
    if (crest_path.starts_with(prefix1) || crest_path.starts_with(prefix2)){
      m_crestCl = std::unique_ptr<Crest::CrestClient>(new Crest::CrestClient(getURLBase()));
    }
    else{
      m_crestCl = std::unique_ptr<Crest::CrestFsClient>(new Crest::CrestFsClient(true,getURLBase()));
    }
  }

  const std::string &
  CrestFunctions::getURLBase(){
    return m_CREST_PATH;
  }

  void 
  CrestFunctions::setURLBase(const std::string & crest_path){
    m_CREST_PATH = crest_path;
  }


  std::string
  CrestFunctions::extractHashFromJson(const std::string & jsonReply){
    std::string hash{};
    try{
      std::string_view signature="payloadHash\":\"";
      auto signaturePosition=jsonReply.rfind(signature);
      if (signaturePosition == std::string::npos) throw std::runtime_error("signature "+std::string(signature)+" not found");
      auto startOfHash=signaturePosition + signature.size();
      auto endOfHash=jsonReply.find('\"',startOfHash);
      auto len=endOfHash-startOfHash;
      if (startOfHash > jsonReply.size()) throw std::runtime_error("Hash start is beyond end of string");
      hash=jsonReply.substr(startOfHash, len);
    } catch (std::exception & e){
      std::cerr<<__FILE__<<":"<<__LINE__<< ": "<<e.what()<<" while trying to find the hash in "<<jsonReply<<std::endl;
    }
    return hash;
  }

  
  std::string 
  CrestFunctions::getLastHashForTag(const std::string & tag){
    char tu[] = "";
    strfry(tu);
    std::string reply = "";

    try{
      IovSetDto dto = m_crestCl->selectIovs(tag, 0, -1, 0, 10000, 0, "id.since:ASC");

      nlohmann::json iov_data = dto.to_json();
      nlohmann::json iov_list = getResources(iov_data);
      reply = iov_list.dump();
    } catch (std::exception & e){
      std::cerr<<__FILE__<<":"<<__LINE__<< ": "<<e.what()<<" while trying to find the IOVs"<<std::endl;
      return "";
    }

    return extractHashFromJson(reply);
  }

  std::string 
  CrestFunctions::getPayloadForHash(const std::string & hash){
    std::string reply = "";  
    
    try{
	reply = m_crestCl->getPayload(hash);
    } catch (std::exception & e){
        std::cerr<<__FILE__<<":"<<__LINE__<< ": "<<e.what()<<" while trying to find the payload"<<std::endl;
        return "";
    }
    
    return reply;
  }
 

  std::string 
  CrestFunctions::extractDescriptionFromJson(const std::string & jsonReply){
    std::string description{};
    try{
      const std::string_view signature="node_description\\\":\\\"";
      const auto signaturePosition = jsonReply.find(signature);
      if (signaturePosition == std::string::npos) throw std::runtime_error("signature "+std::string(signature)+" not found");
      const auto startOfDescription= signaturePosition + signature.size();
      const std::string_view endSignature = "\\\",\\\"payload_spec";
      const auto endOfDescription=jsonReply.find(endSignature);
      if (endOfDescription == std::string::npos) throw std::runtime_error("end signature "+std::string(endSignature)+" not found");
      const auto len=endOfDescription-startOfDescription;
      description=jsonReply.substr(startOfDescription, len);
    } catch (std::exception & e){
      std::cerr<<__FILE__<<":"<<__LINE__<< ": "<<e.what()<<" while trying to find the description in "<<jsonReply<<std::endl;
    }
    
    return unescapeQuotes(unescapeBackslash(description));
  }  


  std::string 
  CrestFunctions::folderDescriptionForTag(const std::string & tag){
    
    std::string jsonReply = "";
    
    TagMetaDto dto = m_crestCl->findTagMeta(tag);
    jsonReply = dto.tagInfo.getFolderDescription();
    return jsonReply;
  }

  
  std::map<std::string, std::string>
  CrestFunctions::getGlobalTagMap(const std::string& globaltag){
    std::map<std::string, std::string> tagmap;
    try{
      GlobalTagMapSetDto dto = m_crestCl->findGlobalTagMap(globaltag,"Trace");
      nlohmann::json globaltag_map_data = dto.to_json();
      nlohmann::json j = getResources(globaltag_map_data);
      int n = j.size();
      for (int i = 0; i < n; i++ ){
	nlohmann::json j_item = j[i];
        if (j_item.contains("label") && j_item.contains("tagName") ){
          tagmap[j_item["label"]] = j_item["tagName"];
        }
      }
    } catch (std::exception & e){
      std::cerr<<__FILE__<<":"<<__LINE__<< ": " << e.what() << " Cannot get a global tag map for " << globaltag << std::endl;
    }

    return tagmap;
  }


  nlohmann::json CrestFunctions::getTagInfo(const std::string & tag){
    try{
      TagMetaDto dto = m_crestCl->findTagMeta(tag);     
      nlohmann::json meta_info = dto.to_json();
      
      if (meta_info.contains("tagInfo")){
	std::string metainf = meta_info["tagInfo"];
	nlohmann::json js = nlohmann::json::parse(metainf);
	return js;
      }

    } catch (std::exception & e){
      std::cerr<<__FILE__<<":"<<__LINE__<< ": " << e.what() << " Cannot get a tag meta info " << tag << std::endl;
    }
    return nullptr;
  }

  nlohmann::json 
  CrestFunctions::getTagProperties(const std::string & tag){
    try{
      TagDto dto = m_crestCl->findTag(tag);
      return dto.to_json();
    } catch (std::exception & e){
      std::cerr<<__FILE__<<":"<<__LINE__<< ": " << e.what() << " Cannot get a tag Properties of " << tag << std::endl;
    }
    return nullptr;
  }

  std::string 
  CrestFunctions::getTagInfoElement(nlohmann::json tag_info, const std::string & key){
    if (tag_info.contains(key)){
      if (key == "channel_list"){ 
        return  tag_info[key].dump();
      } else if (key== "node_description"){
        std::string v;
        tag_info[key].get_to(v);
        return v;
      } else{
        return nlohmann::to_string(tag_info[key]);
      }
    }
    return "";
  }

  std::pair<std::vector<cool::ChannelId> , std::vector<std::string>>
  CrestFunctions::extractChannelListFromString(const std::string & chanString){
    std::vector<cool::ChannelId> list;
    std::vector<std::string> names;
    nlohmann::json js = nlohmann::json::parse(chanString);
    int n = js.size();

    for (int i = 0; i <= n; i++) {
      nlohmann::json j_object = js[i];
      for (auto& [key, val] : j_object.items()){
        list.push_back(std::stoll(key));
        names.push_back(val);
      }
    }

    // if all the names are empty, these are unnamed channels, and can just return an empty vector for the names
    auto isEmpty=[](const std::string & s){return s.empty();};
    if ( std::all_of(names.begin(), names.end(), isEmpty)) names.clear();
    return std::make_pair(std::move(list), std::move(names));
  }

  nlohmann::json 
  CrestFunctions::getResources(nlohmann::json& js) {
    nlohmann::json js2 = json::array();
    nlohmann::json result = js.value("resources", js2);
    return result;
  }

  std::vector<uint64_t>
  CrestFunctions::getIovGroups(const std::string & tag){
    std::vector<uint64_t> v;
    try{
      IovSetDto dto = m_crestCl->selectGroups(tag, 0, 10000, 0, "id.since:ASC");
      const std::vector<IovDto> & res = dto.resources;
      for (const IovDto & item_iov: res){
        v.emplace_back(item_iov.since);
      }
    } catch (std::exception & e){
      std::cerr<<__FILE__<<":"<<__LINE__<< ": "<<e.what()<<" while trying to find the IOVs"<<std::endl;
      return {};
    }
    return v;
  }


  std::pair<uint64_t,uint64_t>
  CrestFunctions::getSinceUntilPair(std::vector<uint64_t> v, const uint64_t since, const uint64_t until){
    uint64_t new_since = 0;
    uint64_t new_until = 0;
    std::pair<uint64_t,uint64_t> answer = std::make_pair(0,0);

    if (until < since){
	std::cerr << "Wrong since/until." << std::endl;
	return answer;
    }

    int N = v.size();
    for (int i = 0; i < N; i++) {
      if(v[i] <= since && since < v[i+1]){
	new_since = v[i];
	break;
      }
    }

    for (int i = 0; i < N; i++) {
      if(v[i] < until && until <= v[i+1]){
	new_until = v[i+1];
	break;
      }
    }    

    answer = std::make_pair(new_since,new_until);
    return answer; 
  }


  int CrestFunctions::getTagSize(const std::string& tagname){
    int res = 0; 
    try{
      res = m_crestCl->getSize(tagname);
    } catch (std::exception & e){
      std::cerr<<__FILE__<<":"<<__LINE__<< ": " << e.what() << " Cannot get the tag size for " << tagname << std::endl;
    }
    return res;
  }

  std::pair<uint64_t,uint64_t>
  CrestFunctions::getIovInterval(const std::string&  tag, const uint64_t since, const uint64_t until){
    std::vector<uint64_t> v = getIovGroups(tag);
    v.push_back(std::numeric_limits<uint64_t>::max()); // added "infinity" as the last item
    return getSinceUntilPair(std::move(v), since, until);
  }


  std::vector<IovHashPair>
  CrestFunctions::getIovsForTag(const std::string & tag, uint64_t since, uint64_t until){
    std::vector<IovHashPair> iovHashPairs;
    int iovNumber = getTagSize(tag);
    try{
      IovSetDto dto;
      if (iovNumber <=1000) {
        dto = m_crestCl->selectIovs(tag, 0, -1, 0, 10000, 0, "id.since:ASC");
      } else {
        const auto &[s_time, u_time] = getIovInterval(tag, since, until);
        if (s_time == 0 && u_time == 0){ // data out of range
	        return iovHashPairs;
	      } else {
          dto = m_crestCl->selectIovs(tag, s_time, u_time, 0, 10000, 0, "id.since:ASC");
	      }
      }
      std::vector<IovDto> res = dto.resources;
      std::map<uint64_t, std::string> hashmap;
      for (const IovDto & item: res) {
        hashmap[item.since] = item.payloadHash;
      } 
      for (auto& t : hashmap){
        iovHashPairs.emplace_back(std::to_string(t.first),t.second);
      }
    } catch (std::exception & e){
      std::cerr<<__FILE__<<":"<<__LINE__<< ": "<<e.what()<<" while trying to find the IOVs"<<std::endl;
      return {};
    }
    return iovHashPairs;
  }
}
