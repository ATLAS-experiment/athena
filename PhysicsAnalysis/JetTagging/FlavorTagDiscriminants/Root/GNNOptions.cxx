/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "FlavorTagDiscriminants/GNNOptions.h"

#include "src/hash.h"

namespace FlavorTagDiscriminants {
  std::size_t GNNOptions::hash() const {
    size_t hash = getHash(flip_config);
    for (const auto& [k, v]: variable_remapping) {
      hash = combine(hash, getHash(k) ^ getHash(v));
    }
    hash = combine(hash, getHash(track_link_type));
    hash = combine(hash, getHash(default_output_value));
    return hash;
  }
  bool GNNOptions::operator==(const GNNOptions& o) const {
  return
    flip_config == o.flip_config &&
    variable_remapping == o.variable_remapping &&
    track_link_type == o.track_link_type &&
    default_output_value == o.default_output_value;
}

}

