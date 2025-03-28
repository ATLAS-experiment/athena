/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/


#ifndef ASSOCIATION_ENUMS_HH
#define ASSOCIATION_ENUMS_HH

#include <string>

namespace FlavorTagInference {
  enum class TrackLinkType {
    TRACK_PARTICLE,
    IPARTICLE
  };
  TrackLinkType trackLinkTypeFromString(const std::string&);
}

#endif
