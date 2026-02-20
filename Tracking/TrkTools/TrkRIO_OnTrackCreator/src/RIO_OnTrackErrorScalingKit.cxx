/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/
#include "RIO_OnTrackErrorScalingKit.h"
#include <cstring> //for strcmp

size_t RIO_OnTrackErrorScalingKit::getParamIndex(const std::string &name) const {
  const char* const* param_names = paramNames();
  size_t idx{};
  for(; idx<nParametres(); ++idx) {
    if (std::strcmp(param_names[idx],name.c_str())==0) break;
  }
  if (idx == nParametres()){
    throw std::runtime_error("RIO_OnTrackErrorScaling parameter " + name + " not found.");
  }
  return idx;
}

