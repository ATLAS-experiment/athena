/*
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef CP_TOOLUTILS_H 
#define CP_TOOLUTILS_H

#include "AthContainers/AuxElement.h"
#include <string>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

class ToolUtils {

  public:
  using QuantileFunc = std::function<int(const SG::AuxElement&)>;
  using VariableFunc = std::function<float(const SG::AuxElement&)>;

  static VariableFunc variableFactory(const json& cfg);
  static QuantileFunc quantileFactory(const json& cfg);

  private:
  static QuantileFunc makeEnumerate(const json& cfg);
  static QuantileFunc makeNodes(const json& cfg);
  static QuantileFunc makeDense(const json& cfg);

};

#endif // CP_TOOLUTILS_H
