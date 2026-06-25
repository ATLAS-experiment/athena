/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "ScaleFactorTools/VariableFactory.h"
#include "ScaleFactorTools/QuantileFactory.h"
#include "PathResolver/PathResolver.h"
#include <iostream>

std::vector<float> QuantileFactory::parseEdges(const json& cfg){
  std::vector<float> edges;
  edges.reserve(cfg.size());

  for (const auto& val : cfg) {
    if (val.is_number()) {
      edges.push_back(val.get<float>());
    }
    else if (val.is_string()) {
      const std::string s = val.get<std::string>();

      if (s == "inf" || s == "+inf") {
        edges.push_back(std::numeric_limits<float>::infinity());
      }
      else if (s == "-inf") {
        edges.push_back(-std::numeric_limits<float>::infinity());
      }
      else {
        throw std::runtime_error("Invalid edge value: " + s);
      }
    }
    else {
      throw std::runtime_error("Edge must be number or string");
    }
  }

  return edges;

}

QuantileFactory::QuantileFunc QuantileFactory::makeCategory(const json& cfg) {
  VariableFactory::IntFunc var = VariableFactory::intVariableFactory(cfg.at("variable"));
  std::vector<int> values = cfg.at("values").get<std::vector<int>>();

  std::unordered_map<int,int> mapping;
  for (int i = 0; i < (int)values.size(); ++i) {
    mapping[values[i]] = i;
  }

  return [var, mapping](const SG::AuxElement& el) -> int {
    int v = static_cast<int>(var(el));
    auto it = mapping.find(v);
    if (it != mapping.end()) {
      return it->second;
    }
    // need to decide what values to return here
    return 0;
  };

}

QuantileFactory::QuantileFunc QuantileFactory::makeEnumerate(const json& cfg) {
  VariableFactory::FloatFunc var = VariableFactory::floatVariableFactory(cfg.at("variable"));
  std::vector<float> edges = QuantileFactory::parseEdges(cfg.at("edges"));

  const bool useAbs = cfg.value("abs", false);

  return [var, edges, useAbs] (const SG::AuxElement& el) -> int {
    float v = var(el);
    if (useAbs)  v = std::abs(v);

    // TODO: need to decide what to do here for out-of-range jets
    if (v < edges.front()) {
      throw std::runtime_error("enumerate: value below minimum edge");
    }
    if (v >= edges.back()) {
      throw std::runtime_error("enumerate: value above maximum edge");
    }

    int bin = -1;
    for (int i = 0; i < (int)edges.size() - 1; ++i) {
      if (v >= edges[i] && v < edges[i+1]) {
        bin = i;
        break;
      }
    }

    return bin;
  };
}

// for 2-d tagging
QuantileFactory::QuantileFunc QuantileFactory::makeNodes(const json& cfg) {
  VariableFactory::FloatFunc var = VariableFactory::floatVariableFactory(cfg.at("variable"));
  std::vector<float> edges = QuantileFactory::parseEdges(cfg.at("edges"));

  std::vector<QuantileFactory::QuantileFunc> sub_nodes;
  for (const auto& node : cfg.at("nodes")) {
    sub_nodes.push_back(QuantileFactory::quantileFactory(node));
  }

  std::string numbering = cfg.value("numbering", "sequential");

  std::vector<int> offsets(sub_nodes.size(), 0);
  if (numbering == "sequential") {
    for (size_t i = 1; i < sub_nodes.size(); ++i) {
      int size = cfg.at("nodes")[i-1].at("edges").size() - 1;
      offsets[i] = offsets[i-1] + size;
    }
  }

  return [var, edges, sub_nodes, offsets, numbering]
         (const SG::AuxElement& el) -> int {
    float v = var(el);

    int region = -1;
    for (int i = 0; i < (int) edges.size() - 1; ++i) {
      if (v >= edges[i] && v < edges[i+1]) {
        region = i;
        break;
      }
    }

    int local = sub_nodes[region](el);

    if (numbering == "sequential") {
      return offsets[region] + local;
    }
    return local;
  };
}

// turn the already computed per-axis bin indices into a flattened index
QuantileFactory::QuantileFunc QuantileFactory::makeDense (const json& cfg) {
  std::vector<QuantileFactory::QuantileFunc> axes;

  for (const auto& axis : cfg.at("axes")) {
    axes.push_back(QuantileFactory::quantileFactory(axis));
  }

  std::vector<int> strides(axes.size(), 1);

  auto getSize = [](const json& axis) -> int {
    std::string type = axis.at("type");
    if (type == "enumerate") {
    return axis.at("edges").size() - 1;
    }
    if (type == "category") {
      return axis.at("values").size();
    }

    if (type == "nodes") {
      int total = 0;
      for (const auto& sub : axis.at("nodes")) {
        std::string subType = sub.at("type");
        if (subType == "enumerate")
          total += sub.at("edges").size() - 1;
        else if (subType == "category")
          total += sub.at("values").size();
        else
          throw std::runtime_error("Unsupported node subtype");
      }
      return total;
    }
    throw std::runtime_error("Unsupported axis type in dense: " + type);
  };

  for (int i = (int)axes.size() - 2; i >= 0; --i) {
    int size = getSize(cfg.at("axes")[i+1]);
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

QuantileFactory::QuantileFunc QuantileFactory::quantileFactory(const json& cfg) {

  std::string type = cfg.at("type");

  if (type == "category")  return QuantileFactory::makeCategory(cfg);
  if (type == "enumerate") return QuantileFactory::makeEnumerate(cfg);
  if (type == "nodes")     return QuantileFactory::makeNodes(cfg);
  if (type == "dense")     return QuantileFactory::makeDense(cfg);

  throw std::runtime_error("Unknown quantile type: " + type);

}

