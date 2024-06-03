/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "CountTrackParticleAlg.h"

namespace FlavorTagDiscriminants {

  CountTrackParticleAlg::CountTrackParticleAlg(
    const std::string& name,
    ISvcLocator* loc):
    detail::CountTrackParticle_t(name, loc)
  {
  }
}
