/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "ScaleFactorTools/VariableFactory.h"
#include "PathResolver/PathResolver.h"
#include <iostream>

VariableFactory::FloatFunc VariableFactory::floatVariableFactory(const json& cfg) {

  // for simple variables
  if (cfg.is_string()) {
    std::string name = cfg.get<std::string>();
    return [name](const SG::AuxElement& el) -> float {
      return el.auxdata<float>(name);
    };
  }

  // structured variables
  if (cfg.is_object()) {
    auto buildTerm = [](const json& terms) {
      return [terms](const SG::AuxElement& el) -> float {
        float sum = 0.0f;
        for (const auto& t : terms) {
          std::string var = t[0];
          float weight    = t[1];
          sum += weight * el.auxdata<float>(var);
        }
        return sum;
      };
    };

    auto num_func = buildTerm(cfg.at("numerator"));
    auto den_func = buildTerm(cfg.at("denominator"));
    std::string mode = cfg.contains("mode") ? cfg.at("mode").get<std::string>() : "log_ratio";

    return [num_func, den_func, mode] (const SG::AuxElement& el) -> float {
      float num = num_func(el);
      float den = den_func(el);
      if (mode == "log_ratio" ) {
        return (den > 0.0f && num > 0.0f) ? std::log(num / den) : 0.0f;
      }
      else {
        throw std::runtime_error("Unknown variable mode: " + mode);
      }
    };
  }

  throw std::runtime_error("Invalid variable config");

}

VariableFactory::IntFunc VariableFactory::intVariableFactory(const json& cfg) {
  if (cfg.is_string()) {
    std::string name = cfg.get<std::string>();
    return [name](const SG::AuxElement& el) -> int {
      return el.auxdata<int>(name);
    };
  }
  throw std::runtime_error("Invalid variable config");
}

