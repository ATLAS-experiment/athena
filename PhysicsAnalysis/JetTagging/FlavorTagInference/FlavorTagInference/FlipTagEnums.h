/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/


#ifndef FLIP_TAG_ENUMS_HH
#define FLIP_TAG_ENUMS_HH

#include <string>

namespace FlavorTagInference {
  // note that all the "non-standard" ones here use the default flip
  // config for SV1, JF, and IPxD. The variants are just for the RNN.
  // The variables each config inverts are listed in flip_variable_regex,
  // in ConstituentsLoader.cxx. They are inverted on the track, electron
  // and muon inputs alike.
  enum class FlipTagConfig {
    STANDARD,                   // use all tracks
    NEGATIVE_IP_ONLY,           // use only negative IP, flip the lifetime-signed impact parameters
    FLIP_SIGN,                  // flip the lifetime-signed impact parameters, use all tracks
    SIMPLE_FLIP,                // also flip the perigee-signed impact parameters, use all tracks
  };
  FlipTagConfig flipTagConfigFromString(const std::string&);
}

#endif
