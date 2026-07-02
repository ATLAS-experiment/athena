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

  auto func = [var, mapping](const SG::AuxElement& el) -> int {
    int v = static_cast<int>(var(el));
    auto it = mapping.find(v);
    if (it != mapping.end()) {
      return it->second;
    }
    // need to decide what values to return here
    return 0;
  };

  return {values.size(), func};

}

QuantileFactory::QuantileFunc QuantileFactory::makeEnumerate(const json& cfg) {
  VariableFactory::FloatFunc var = VariableFactory::floatVariableFactory(cfg.at("variable"));
  std::vector<float> edges = QuantileFactory::parseEdges(cfg.at("edges"));

  const bool useAbs = cfg.value("abs", false);

  bool hasRangeHandling = cfg.contains("validRange");
  float minRange = 0.0;
  float maxRange = 0.0;
  std::string mode;
  if (hasRangeHandling) {
    const auto& r = cfg.at("validRange");
    minRange = r[0];
    maxRange = r[1];
    mode = cfg.at("OutOfRangeTreatment");
  }

  auto func = [var, edges, useAbs, mode, hasRangeHandling, minRange, maxRange] (const SG::AuxElement& el) -> int {
    float v = var(el);
    if (useAbs)  v = std::abs(v);
    int nBins = edges.size() - 1;

    if ( !hasRangeHandling || (v >= minRange && v < maxRange) )  {
      for (int i = 0; i < nBins; ++i) {
        if (v >= edges[i] && v < edges[i+1]) {
          return i;
        }
      }
    } else {
      if (mode == "neighboring") {
        if (v < edges[0])  return 0;
        if (v >= edges[nBins])  return nBins - 1;
      }
    }
    return -1;
  };

  return {edges.size() - 1, func};
}

// for 2-d tagging
QuantileFactory::QuantileFunc QuantileFactory::makeNodes(const json& cfg) {
  VariableFactory::FloatFunc var = VariableFactory::floatVariableFactory(cfg.at("variable"));
  std::vector<float> edges = QuantileFactory::parseEdges(cfg.at("edges"));

  std::vector<std::function<int(const SG::AuxElement&)>> sub_nodes;
  std::vector<size_t> sub_sizes;

  size_t totalSize = 0;

  for (const auto& node : cfg.at("nodes")) {
    auto [size, func] = QuantileFactory::quantileFactory(node);
    sub_nodes.push_back(func);
    sub_sizes.push_back(size);
    totalSize += size;
  }

  std::string numbering = cfg.value("numbering", "sequential");

  std::vector<int> offsets(sub_nodes.size(), 0);
  if (numbering == "sequential") {
    for (size_t i = 1; i < sub_nodes.size(); ++i) {
      offsets[i] = offsets[i-1] + sub_sizes[i-1];
    }
  }

  auto func = [var, edges, sub_nodes, offsets, numbering]
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

  return {totalSize, func};
}

// turn the already computed per-axis bin indices into a flattened index
QuantileFactory::QuantileFunc QuantileFactory::makeDense (const json& cfg) {
  std::vector<std::function<int(const SG::AuxElement&)>> axes;
  std::vector<size_t> axis_sizes;

  for (const auto& axis : cfg.at("axes")) {
    auto [size, func] = QuantileFactory::quantileFactory(axis);
    axes.push_back(func);
    axis_sizes.push_back(size);
  }

  std::vector<int> strides(axes.size(), 1);
  for (int i = (int)axes.size() - 2; i >= 0; --i) {
    strides[i] = strides[i+1] * axis_sizes[i+1];
  }

  auto func = [axes, strides](const SG::AuxElement& el) -> int {
    int index = 0;

    for (size_t i = 0; i < axes.size(); ++i) {
      int bin = axes[i](el);
      if (bin < 0) {return -1;}
      index += bin * strides[i];
    }

    return index;
  };

  size_t total_size = 1;
  for (size_t size : axis_sizes) {
    total_size *= size;
  }

  return {total_size, func};

}

QuantileFactory::QuantileFunc QuantileFactory::quantileFactory(const json& cfg) {

  std::string type = cfg.at("type");

  if (type == "category")  return QuantileFactory::makeCategory(cfg);
  if (type == "enumerate") return QuantileFactory::makeEnumerate(cfg);
  if (type == "nodes")     return QuantileFactory::makeNodes(cfg);
  if (type == "dense")     return QuantileFactory::makeDense(cfg);

  throw std::runtime_error("Unknown quantile type: " + type);

}

