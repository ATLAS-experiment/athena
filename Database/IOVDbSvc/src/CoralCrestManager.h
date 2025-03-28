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
#include "CrestApi/CrestApiBase.h"
#include "CrestApi/CrestRequest.h"

class CoralCrestManager {
public:
  static inline const std::string prefix1{"http://"};
  static inline const std::string prefix2 {"https://"};
  CoralCrestManager(const std::string & crest_path, const std::string & crestTag);
  static std::map<std::string, std::string> getGlobalTagMap(const std::string & crest_path, const std::string& globaltag);
private:
  std::unique_ptr<Crest::CrestApiBase> m_crestCl;
  const std::string m_crestTag;
};
#endif
