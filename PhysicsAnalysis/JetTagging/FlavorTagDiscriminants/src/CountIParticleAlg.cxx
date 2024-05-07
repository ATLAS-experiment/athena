/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "CountIParticleAlg.h"

namespace FlavorTagDiscriminants {

  CountIParticleAlg::CountIParticleAlg(
    const std::string& name,
    ISvcLocator* loc):
    detail::CountIParticle_t(name, loc)
  {
  }
}
