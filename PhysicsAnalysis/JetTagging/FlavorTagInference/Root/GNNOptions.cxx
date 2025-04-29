/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "FlavorTagInference/GNNOptions.h"

#include "src/hash.h"

namespace FlavorTagInference {
  std::size_t GNNOptions::hash() const {
    size_t hash = getHash(flip_config);
    for (const auto& [k, v]: variable_remapping) {
      hash = combine(hash, getHash(k) ^ getHash(v));
    }
    hash = combine(hash, getHash(track_link_type));
    hash = combine(hash, getHash(default_output_value));
    for (const auto& [k, v]: default_output_values) {
      hash = combine(hash, getHash(k) ^ getHash(v));
    }
    hash = combine(hash, getHash(default_zero_tracks));
    return hash;
  }
  bool GNNOptions::operator==(const GNNOptions& o) const {
    return
      flip_config == o.flip_config &&
      variable_remapping == o.variable_remapping &&
      track_link_type == o.track_link_type &&
      default_output_value == o.default_output_value &&
      default_output_values == o.default_output_values && 
      default_zero_tracks == o.default_zero_tracks;
  }
}

