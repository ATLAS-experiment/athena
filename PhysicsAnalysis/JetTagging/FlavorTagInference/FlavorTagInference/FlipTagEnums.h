/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/


#ifndef FLIP_TAG_ENUMS_HH
#define FLIP_TAG_ENUMS_HH

#include <string>

namespace FlavorTagInference {
  // note that all the "non-standard" ones here use the default flip
  // config for SV1, JF, and IPxD. The variants are just for the RNN.
  enum class FlipTagConfig {
    STANDARD,                   // use all tracks
    NEGATIVE_IP_ONLY,           // use only negative IP, flip sign
    FLIP_SIGN,                  // just flip the sign of IP3D_signed_d0_significance, IP3D_signed_z0_significance, use all tracks
    SIMPLE_FLIP,                // flip the sign of d0, z0SinTheta, IP3D_signed_d0_significance, IP3D_signed_z0_significance, use all tracks
  };
  FlipTagConfig flipTagConfigFromString(const std::string&);
}

#endif
