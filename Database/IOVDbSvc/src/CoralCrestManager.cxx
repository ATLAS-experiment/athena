/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
// @file CoralCrestManager.cxx
// Implementation for CrestFunctions utilities
// @author Evgeny Alexandrov
// @date 24 February 2025

#include "CoralCrestManager.h"
#include "CrestApi/CrestApi.h"
#include "CrestApi/CrestApiFs.h"
#include "CoralBase/AttributeList.h"
#include "CoralBase/Attribute.h"
#include "CoolKernel/StorageType.h"
#include "CoolKernel/RecordSpecification.h"
#include "CoolKernel/Record.h"

 CoralCrestManager::CoralCrestManager(const std::string & crest_path, const std::string & crestTag):m_crestTag(crestTag){
    if(crest_path.length()==0)
      return;
    if (crest_path.starts_with(CoralCrestManager::prefix1) || crest_path.starts_with(CoralCrestManager::prefix2)){
      m_crestCl = std::make_unique<Crest::CrestApi>(Crest::CrestApi(crest_path));
    }
    else{
      m_crestCl = std::make_unique<Crest::CrestApiFs>(Crest::CrestApiFs(true,crest_path));
    }

  }
  std::map<std::string, std::string> CoralCrestManager::getGlobalTagMap(const std::string & crest_path, const std::string& globaltag){
    Crest::CrestApiBase* crestCl=NULL;
    if (crest_path.starts_with(CoralCrestManager::prefix1) || crest_path.starts_with(CoralCrestManager::prefix2)){
      crestCl = new Crest::CrestApi(crest_path);
    }
    else{
      crestCl = new Crest::CrestApiFs(true,crest_path);
    }
    std::map<std::string, std::string> tagmap;
    try{
      Crest::GlobalTagMapSetDto dto = crestCl->findGlobalTagMap(globaltag,"Trace");
      for (const auto &tagMapDto : dto.getResources()){
	tagmap[tagMapDto.getLabel()]=tagMapDto.getTagName();
      }
    } catch (std::exception & e){
      std::cerr<<__FILE__<<":"<<__LINE__<< ": " << e.what() << " Cannot get a global tag map for " << globaltag << std::endl;
    }
    if(crestCl!=NULL){
      delete crestCl;
      crestCl=NULL;
    }
    return tagmap;
  }
  std::string CoralCrestManager::parseTypeName(const std::string & description){
    std::string regex=R"delim(<typeName>\s*([^\s]+)\s*</typeName>)delim";
    std::regex re(regex);
    std::smatch typeMatch;
    bool match=std::regex_search(description, typeMatch,re);
    return (match) ? std::string(typeMatch[1]) : std::string("");
  }
 IOVDbNamespace::FolderType CoralCrestManager::determineFolderType(){
    Crest::TagInfoDto info = getTagInfoDto();
    std::string folderDescription = info.getFolderDescription();
    if (folderDescription.find("<coracool>") != std::string::npos) return IOVDbNamespace::CoraCool;
    const std::string typeName = parseTypeName(folderDescription);
    if (typeName=="CondAttrListVec"){
        m_isVectorPayload = true;
        return IOVDbNamespace::CoolVector;
    }
    m_isVectorPayload = false;
    std::vector< std::pair<std::string,std::string> > spec= info.getPayloadSpec().getColumns();
    std::vector< std::pair<std::string,std::string> > chs = info.getChannels().getChannels();
    for (auto &p : spec){
      if(p.first=="PoolRef" && p.second=="String4k"){
        int id=std::stoll(chs[0].first);
        if(chs.size()==1 && id==0)
          return IOVDbNamespace::PoolRef;
        else
          return IOVDbNamespace::PoolRefColl;
      }
    }
    if (typeName == "CondAttrListCollection") return IOVDbNamespace::AttrListColl;
    return IOVDbNamespace::AttrList;
  }

  bool CoralCrestManager::isVectorPayload(){
    if(!m_isVectorPayload.has_value())
       determineFolderType();
    return m_isVectorPayload.value();
  }

  void CoralCrestManager::loadTagInfo(){
    if(m_TagMeta.has_value()){
      return;
    }

    try{
      m_TagMeta = m_crestCl->findTagMeta(m_crestTag);
    } catch (std::exception & e){
      std::cerr<<__FILE__<<":"<<__LINE__<< ": " << e.what() << " Cannot get a tag meta info " << m_crestTag << std::endl;
    }
    return;
  }

  Crest::TagInfoDto CoralCrestManager::getTagInfoDto(){
    if(!m_TagMeta.has_value())
      loadTagInfo();
    return m_TagMeta.value().getTagInfoDto();
  }
  Crest::TagDto& CoralCrestManager::getTagDto(){
    if(!m_Tag.has_value()){
      m_Tag = m_crestCl->findTag(m_crestTag);
    }
    return m_Tag.value();
  }

  std::string CoralCrestManager::getPayloadSpec(){
    return getTagInfoDto().getPayloadSpec().toJson().dump();
  }

  std::string CoralCrestManager::getFolderDescription(){
    Crest::TagInfoDto info = getTagInfoDto();
    return info.getFolderDescription();
  }
  std::pair<std::vector<cool::ChannelId> , std::vector<std::string>> CoralCrestManager::getChannelList(){
    if(!m_TagMeta.has_value())
      loadTagInfo();
    Crest::TagInfoDto info = getTagInfoDto();
    std::vector<cool::ChannelId> list;
    std::vector<std::string> names;
    std::vector< std::pair<std::string,std::string> > res = info.getChannels().getChannels();
    for (auto &p : res){
      list.push_back(std::stoll(p.first));
      names.push_back(p.second);
    }
    return std::make_pair(std::move(list), std::move(names));
  }

