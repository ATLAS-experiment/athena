/*
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CP_TOOLUTILS_H 
#define CP_TOOLUTILS_H

#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class ToolUtils {

  public:
  using QuantileFunc = std::function<int(const SG::AuxElement&)>;
  using VariableFunc = std::function<float(const SG::AuxElement&)>;

  VariableFunc variableFactory(const json& cfg);
  QuantileFunc quantileFactory(const json& cfg);

  private:
  QuantileFunc makeEnumerate(const json& cfg);
  QuantileFunc makeNodes(const json& cfg);
  QuantileFunc makeDense(const json& cfg);

};

#endif // CP_TOOLUTILS_H
