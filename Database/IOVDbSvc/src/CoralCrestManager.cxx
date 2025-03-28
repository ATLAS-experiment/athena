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



