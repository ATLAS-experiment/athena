/*
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef CP_QUANTILEFACTORY_H 
#define CP_QUANTILEFACTORY_H

#include "AthContainers/AuxElement.h"
#include <string>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

class QuantileFactory {

  public:
  using QuantileFunc = std::function<int(const SG::AuxElement&)>;
  static QuantileFunc quantileFactory(const json& cfg);

  private:
  static QuantileFunc makeCategory(const json& cfg);
  static QuantileFunc makeEnumerate(const json& cfg);
  static QuantileFunc makeNodes(const json& cfg);
  static QuantileFunc makeDense(const json& cfg);

};

#endif // CP_QUANTILEFACTORY_H
