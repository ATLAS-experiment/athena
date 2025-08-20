//
// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
//

// Local include(s):
#include "xAODTruth/versions/TruthParticleAuxContainer_v2.h"

namespace xAOD {

TruthParticleAuxContainer_v2::TruthParticleAuxContainer_v2()
    : AuxContainerBase() {

  AUX_VARIABLE(pdgId);
  AUX_VARIABLE(uid);
  AUX_VARIABLE(status);
  AUX_VARIABLE(prodVtxLink);
  AUX_VARIABLE(decayVtxLink);
  AUX_VARIABLE(px);
  AUX_VARIABLE(py);
  AUX_VARIABLE(pz);
  AUX_VARIABLE(e);
  AUX_VARIABLE(m);
}

}  // namespace xAOD
