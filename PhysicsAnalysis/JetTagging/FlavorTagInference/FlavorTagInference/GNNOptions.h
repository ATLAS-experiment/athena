/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GNN_OPTIONS_H
#define GNN_OPTIONS_H

#include "FlavorTagInference/FlipTagEnums.h"
#include "FlavorTagInference/AssociationEnums.h"

#include <map>
#include <string>
#include <cmath>

namespace FlavorTagInference {
  struct GNNOptions {
    FlipTagConfig flip_config = FlipTagConfig::STANDARD;
    std::map<std::string, std::string> variable_remapping = {};
    TrackLinkType track_link_type = TrackLinkType::TRACK_PARTICLE;
    float default_output_value = NAN;
    std::map<std::string, float> default_output_values;
    bool default_zero_tracks = false;
    bool operator==(const GNNOptions&) const;
    std::size_t hash() const;
  };
}

// some definitions to make sure we can use GNNOptions as a key
template<>
struct std::hash<FlavorTagInference::GNNOptions> {
  std::size_t operator()(const FlavorTagInference::GNNOptions& o) const {
    return o.hash();
  }
};

#endif
