/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "xAODMuonInference/versions/MuonDVInferenceResultAuxContainer_v1.h"

namespace xAOD {

  MuonDVInferenceResultAuxContainer_v1::MuonDVInferenceResultAuxContainer_v1()
      : AuxContainerBase() {
    AUX_VARIABLE(valid);
    AUX_VARIABLE(pass);
    AUX_VARIABLE(score);
    AUX_VARIABLE(rawOutput);
    AUX_VARIABLE(decisionValue);
    AUX_VARIABLE(cutValue);
    AUX_VARIABLE(nNodes);
    AUX_VARIABLE(nMuonNodes);
    AUX_VARIABLE(nCaloNodes);
    AUX_VARIABLE(nEdges);
  }

}  // namespace xAOD
