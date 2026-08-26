/*
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef CP_VARIABLEFACTORY_H 
#define CP_VARIABLEFACTORY_H

#include "AthContainers/AuxElement.h"
#include <string>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

class VariableFactory {

  public:
  using FloatFunc = std::function<float(const SG::AuxElement&)>;
  using IntFunc   = std::function<int(const SG::AuxElement&)>;

  static FloatFunc floatVariableFactory(const json& cfg);
  static IntFunc intVariableFactory(const json& cfg);

};
#endif // CP_VARIABLEFACTORY_H 
