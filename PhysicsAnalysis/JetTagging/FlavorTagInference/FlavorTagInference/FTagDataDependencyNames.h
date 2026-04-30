/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef FTAG_DATA_DEPENDENCY_NAMES_H
#define FTAG_DATA_DEPENDENCY_NAMES_H

#include <set>
#include <string>

namespace FlavorTagInference {
  struct FTagDataDependencyNames {
    std::set<std::string> trackInputs;
    std::set<std::string> electronInputs;
    std::set<std::string> muonInputs;
    std::set<std::string> bTagInputs;
    std::set<std::string> bTagOutputs;
    //coverity[PASS_BY_VALUE]
    FTagDataDependencyNames operator+(FTagDataDependencyNames rhs) const;
    //coverity[PASS_BY_VALUE]
    FTagDataDependencyNames& operator+=(FTagDataDependencyNames d);
    bool operator==(const FTagDataDependencyNames&) const = default;
  };

}

#endif
