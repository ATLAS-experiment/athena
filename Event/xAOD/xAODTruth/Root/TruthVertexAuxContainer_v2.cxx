//
// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
//

// Local include(s):
#include "xAODTruth/versions/TruthVertexAuxContainer_v2.h"

namespace xAOD {

TruthVertexAuxContainer_v2::TruthVertexAuxContainer_v2() : AuxContainerBase() {

  AUX_VARIABLE(status);
  AUX_VARIABLE( uid );
  AUX_VARIABLE(incomingParticleLinks);
  AUX_VARIABLE(outgoingParticleLinks);
  AUX_VARIABLE(x);
  AUX_VARIABLE(y);
  AUX_VARIABLE(z);
  AUX_VARIABLE(t);
}

}  // namespace xAOD
