/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GNN_OPTIONS_H
#define GNN_OPTIONS_H

#include "FlavorTagDiscriminants/FlipTagEnums.h"
#include "FlavorTagDiscriminants/AssociationEnums.h"

#include <map>
#include <string>
#include <cmath>

namespace FlavorTagDiscriminants {
  struct GNNOptions {
    FlipTagConfig flip_config = FlipTagConfig::STANDARD;
    std::map<std::string, std::string> variable_remapping = {};
    TrackLinkType track_link_type = TrackLinkType::TRACK_PARTICLE;
    float default_output_value = NAN;
  };
}

#endif
