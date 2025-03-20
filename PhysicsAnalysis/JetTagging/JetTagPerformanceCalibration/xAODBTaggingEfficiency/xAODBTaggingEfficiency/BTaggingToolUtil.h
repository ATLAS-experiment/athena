/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef BTAGGINGTOOLUTIL_H
#define BTAGGINGTOOLUTIL_H

#include <nlohmann/json.hpp>
#include <string>

class BTaggingToolUtil {

  public:
  static float getExtendedFloat(const nlohmann::json &pt);
  static std::string getExtendedString(const nlohmann::json &pt);
};

#endif // BTAGGINGTOOLUTIL_H
