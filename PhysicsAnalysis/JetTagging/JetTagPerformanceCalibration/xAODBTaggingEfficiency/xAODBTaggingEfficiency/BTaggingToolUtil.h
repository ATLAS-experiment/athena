/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef BTAGGINGTOOLUTIL_H
#define BTAGGINGTOOLUTIL_H

#include <nlohmann/json.hpp>
#include <string>

class BTaggingToolUtil {

  public:
  // Multiply a value in MeV by this factor to get it in GeV
  static constexpr double MeVToGeV = 1e-3;

  static float getExtendedFloat(const nlohmann::json &pt);
  static std::string getExtendedString(const nlohmann::json &pt);
};

#endif // BTAGGINGTOOLUTIL_H
