/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "xAODBTaggingEfficiency/BTaggingToolUtil.h"

float BTaggingToolUtil::getExtendedFloat(const nlohmann::json& pt) {
  if (pt.is_string() && pt.get<std::string>() == "inf") {
    return std::numeric_limits<float>::infinity();
  } else {
    return pt.get<float>();
  }
}

std::string BTaggingToolUtil::getExtendedString(const nlohmann::json& pt) {
  if (pt.is_string()){
    return pt.get<std::string>();
  }
  else{ 
    return pt.dump();
  }
}

