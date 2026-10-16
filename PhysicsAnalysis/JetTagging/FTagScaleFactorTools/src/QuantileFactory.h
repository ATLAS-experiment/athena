/*
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef CP_QUANTILEFACTORY_H 
#define CP_QUANTILEFACTORY_H

#include "AthContainers/AuxElement.h"
#include <string>
#include <nlohmann/json.hpp>

/// @brief Builds binning and quantisation functions from a JSON configuration.
///
/// Several binning schemes are supported, each suited to a different
/// representation of the tagger output. The quantileFactory
/// dispatches to the appropriate scheme based on the configuration.
class QuantileFactory {

  public:
  using QuantileFunc = std::pair<size_t, std::function<int(const SG::AuxElement&)>>;
  /// @brief Dispatch to the appropriate binning scheme based on the JSON config.
  static QuantileFunc quantileFactory(const nlohmann::json& cfg);
  
  /// @brief Parse an ordered list of bin edges from the JSON config.
  /// Edges may be finite floats or the strings "inf", "+inf", "-inf".
  /// Throws if the resulting edges are not strictly sorted in ascending order.
  static std::vector<float> parseEdges(const nlohmann::json& cfg);

  private:
  /// @brief Maps discrete values to category indices.
  static QuantileFunc makeEnumerate(const nlohmann::json& cfg);

  /// @brief Converts a continuous variable into a bin index
  /// using an ordered list of bin edges.
  static QuantileFunc makeCategory(const nlohmann::json& cfg);

  /// @brief Builds a tree-based lookup structure for node-based binning,
  /// primarily used for pseudo-continuous 2D tagging.
  static QuantileFunc makeNodes(const nlohmann::json& cfg);

  /// @brief Combines multiple binning dimensions into a single flattened
  /// global index used for scale factor lookup.
  static QuantileFunc makeDense(const nlohmann::json& cfg);

};

#endif // CP_QUANTILEFACTORY_H
