/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "VariableFactory.h"
#include <iostream>
using json = nlohmann::json;

VariableFactory::FloatFunc VariableFactory::floatVariableFactory(const json& cfg) {

  // for simple variables
  if (cfg.is_string()) {
    std::string name = cfg.get<std::string>();
    return [acc = SG::ConstAccessor<float>(name)](const SG::AuxElement& el) -> float {
      return acc(el);
    };
  }

  // structured variables
  if (cfg.is_object()) {
    auto buildTerm = [](const json& terms) {
      std::vector<std::pair<float, SG::ConstAccessor<float>>> accessors;

      for (const auto& t : terms) {
        std::string var = t[0];
        float weight = t[1];
        accessors.emplace_back(weight, SG::ConstAccessor<float>(var));
      }

      return [accessors](const SG::AuxElement& el) -> float {
        float sum = 0.0f;
        for (const auto& [weight, acc] : accessors) {
          sum += weight * acc(el);
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
        // Smallest positive normal float; below this, reciprocals may overflow.
        const float ep = std::numeric_limits<float>::min();
        const float ratio = (std::abs(den) < ep ? std::numeric_limits<float>::infinity()
                                                : num / den);
        return (std::abs(ratio) < ep ? -std::numeric_limits<float>::infinity()
                                     : std::log(ratio));
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
    return [acc = SG::ConstAccessor<int>(name)](const SG::AuxElement& el) -> int {
      return acc(el);
    };
  }
  throw std::runtime_error("Invalid variable config");
}

