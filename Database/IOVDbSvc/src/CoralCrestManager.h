/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file CoralCrestManager.h
 * @brief Header for CoralCrestManager class.
 * @author Evgeny Alexandrov
 * @date 24 February 2025
 */

#ifndef IOVDBSVC_CORALCRESTMANAGER_H
#define IOVDBSVC_CORALCRESTMANAGER_H
#include <string>
#include <iostream>
#include <map>
#include <list>
#include <regex>
#include <iosfwd>
#include <cstdint>
#include "CrestApi/CrestApiBase.h"
#include "CrestApi/CrestRequest.h"
#include "FolderTypes.h"
#include "CoolKernel/ChannelSelection.h"
#include "CoolKernel/ValidityKey.h"
#include "CoolKernel/IFolder.h"
#include "CrestContainer.h"

class CoralCrestManager {
public:
  static inline const std::string prefix1{"http://"};
  static inline const std::string prefix2 {"https://"};

  CoralCrestManager(const std::string & crest_path, const std::string & crestTag);

  static std::map<std::string, std::string> getGlobalTagMap(const std::string & crest_path, const std::string& globaltag);

  void loadTagInfo();

  std::string getFolderDescription();

  Crest::TagDto& getTagDto();

  std::string getPayloadSpec();

  IOVDbNamespace::FolderType determineFolderType();

  std::pair<std::vector<cool::ChannelId> , std::vector<std::string>> getChannelList();

  Crest::TagInfoDto getTagInfoDto();

  bool isVectorPayload();

  coral::AttributeListSpecification* getAttributeListSpec();
  std::vector<std::pair<cool::ValidityKey,std::string>> getIovsForTag(uint64_t since, uint64_t until);
  std::vector<uint64_t> loadPayloadForHash(uint64_t since,const std::string & hash);
  std::vector<coral::AttributeList> getVectorPayload(coral::AttributeListSpecification*  pSpec,const std::string & chId);
  std::string dumpPayload(cool::ValidityKey since);
  coral::AttributeList getPayload(coral::AttributeListSpecification*  pSpec,const std::string & chId);
  void selectIov(cool::ValidityKey since);
  std::vector<std::string> channelIds(cool::ValidityKey since);

private:
  std::unique_ptr<Crest::CrestApiBase> m_crestCl;
  const std::string m_crestTag;
  std::optional<Crest::TagMetaDto> m_TagMeta;
  std::optional<Crest::TagDto> m_Tag;
  std::optional<bool> m_isVectorPayload;
  std::string parseTypeName(const std::string & description);
  coral::AttributeList createAttributeList(coral::AttributeListSpecification * pSpec,nlohmann::json& j ,const std::vector<std::pair<std::string, Crest::TypeId>> & tSpec);
  std::pair<uint64_t,uint64_t> getIovInterval(const std::string&  tag, const uint64_t since, const uint64_t until);
  std::pair<uint64_t,uint64_t> getSinceUntilPair(std::vector<uint64_t>& v, const uint64_t since, const uint64_t until);
  void initCrestContainer();
  std::optional<Crest::CrestContainer> m_crest_cont;

};
#endif
