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

private:
  std::unique_ptr<Crest::CrestApiBase> m_crestCl;
  const std::string m_crestTag;
  std::optional<Crest::TagMetaDto> m_TagMeta;
  std::optional<Crest::TagDto> m_Tag;
  std::optional<bool> m_isVectorPayload;
  std::string parseTypeName(const std::string & description);
};
#endif
