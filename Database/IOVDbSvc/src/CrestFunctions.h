/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file CrestFunctions.h
 * @brief Header for CrestFunctions utilities
 * @author Shaun Roe
 * @date 1 July 2019
 */

#ifndef IOVDBSVC_CRESTFUNCTIONS_H
#define IOVDBSVC_CRESTFUNCTIONS_H

#include <vector>
#include <string>
#include <map> //also contains std::pair
#include <string_view>

#include "CoolKernel/ChannelId.h"

#include "nlohmann/json.hpp"

#include <CrestApi/CrestApiBase.h>


namespace IOVDbNamespace{
  typedef std::pair<std::string,std::string> IovHashPair; // <IOV,Hash> pairs extracted from Json

  class CrestFunctions
  {

    public:

    CrestFunctions(const std::string & crest_path);

    const std::string & getURLBase();

    void setURLBase(const std::string & crest_path);

    std::string
    extractHashFromJson(const std::string & jsonReply);

    std::vector<IovHashPair>
    getIovsForTag(const std::string & tag, uint64_t since, uint64_t until);

    std::string
    getLastHashForTag(const std::string & tag);
    
    std::string 
    getPayloadForHash(const std::string & hash);

    std::string 
    folderDescriptionForTag(const std::string & tag);

    std::string 
    extractDescriptionFromJson(const std::string & jsonReply);

    std::map<std::string, std::string>
    getGlobalTagMap(const std::string& globaltag);

    nlohmann::json getTagInfo(const std::string & tag);

    nlohmann::json getTagProperties(const std::string & tag);

    std::string
    getTagInfoElement(nlohmann::json tag_info, const std::string & key);

    std::pair<std::vector<cool::ChannelId> , std::vector<std::string>>
    extractChannelListFromString(const std::string & chanString);

    std::vector<uint64_t>
    getIovGroups(const std::string & tag);

    std::pair<uint64_t,uint64_t>
    getSinceUntilPair(std::vector<uint64_t> v, const uint64_t since, const uint64_t until);

    int
    getTagSize(const std::string& tagname);

    std::pair<uint64_t,uint64_t>
    getIovInterval(const std::string&  tag, const uint64_t since, const uint64_t until);

    private:

    std::unique_ptr<Crest::CrestApiBase> m_crestCl;
    
    std::string m_CREST_PATH = "";
    
    nlohmann::json getResources(nlohmann::json& js);

  };
}
#endif
