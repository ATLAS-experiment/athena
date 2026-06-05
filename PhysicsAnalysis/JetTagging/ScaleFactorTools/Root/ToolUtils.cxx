/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "ScaleFactorTools/ToolUtils.h"
#include "PathResolver/PathResolver.h"


ToolUtils::VariableFunc ToolUtils::variableFactory(const json& cfg) {

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
    const std::string mode = cfg.value("mode", "log_ratio");

    return [num_func, den_func] (const SG::AuxElement& el) -> float {
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

ToolUtils::QuantileFunc ToolUtils::makeEnumerate(const json& cfg) {
  ToolUtils::VariableFunc var = ToolUtils::variableFactory(cfg.at("variable"));
  std::vector<float> edges = cfg.at("edges").get<std::vector<float>>();

  const bool useAbs = cfg.value("abs", false);

  return [var, edges, useAbs] (const SG::AuxElement& el) -> int {
    float v = var(el);
    if (useAbs) = std::abs(v);

    int bin = 0;
    while (bin < (int)edges.size() && v > edges[bin]){
      bin++;
    }
    return bin;
  };
}

ToolUtils::QuantileFunc ToolUtils::makeNodes(const json& cfg) {
  ToolUtils::VariableFunc var = ToolUtils::variableFactory(cfg.at("variable"));
  std::vector<float> edges = cfg.at("edges");

  std::vector<ToolUtils::QuantileFunc> sub_nodes;
  for (const auto& node : cfg.at("nodes")) {
    sub_nodes.push_back(ToolUtils::quantileFactory(node));
  }

  std::string numbering = cfg.value("numbering", "sequential");

  std::vector<int> offsets(sub_nodes.size(), 0);
  if (numbering == "sequential") {
    for (size_t i = 1; i < sub_nodes.size(); ++i) {
      int size = cfg.at("nodes")[i-1].at("edges").size() + 1;
      offsets[i] = offsets[i-1] + size;
    }
  }

  return [var, edges, sub_nodes, numbering, offsets](const SG::AuxElement& el) -> int {
    float v = var(el);
    int region = 0;
    while (region < (int)edges.size() && v > edges[region]) {
      region++;
    }
    int local = sub_nodes[region](el);
    if (numbering == "sequential") {
      return offsets[region] + local;
    }
    // overlapping
    return local;
    };
}

ToolUtils::QuantileFunc ToolUtils::makeDense (const json& cfg) {
  std::vector<ToolUtils::QuantileFunc> axes;

  for (const auto& axis : cfg.at("axes")) {
    axes.push_back(ToolUtils::quantileFactory(axis));
  }

  std::vector<int> strides(axes.size(), 1);

  for (int i = (int)axes.size() - 2; i >= 0; --i) {
    int size = cfg.at("axes")[i+1].at("edges").size() + 1;
    strides[i] = strides[i+1] * size;
  }

  return [axes, strides](const SG::AuxElement& el) -> int {
    int index = 0;

    for (size_t i = 0; i < axes.size(); ++i) {
      int bin = axes[i](el);
      index += bin * strides[i];
    }

    return index;
  };
}

ToolUtils::QuantileFunc ToolUtils::quantileFactory(const json& cfg) {

  std::string type = cfg.at("type");

  if (type == "enumerate") return ToolUtils::makeEnumerate(cfg);
  if (type == "nodes")     return ToolUtils::makeNodes(cfg);
  if (type == "dense")     return ToolUtils::makeDense(cfg);

  throw std::runtime_error("Unknown quantile type: " + type);

}
