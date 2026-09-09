/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONINFERENCE_MUONDVINFERENCERESULTAUXCONTAINER_H
#define XAODMUONINFERENCE_MUONDVINFERENCERESULTAUXCONTAINER_H

#include "xAODMuonInference/versions/MuonDVInferenceResultAuxContainer_v1.h"

namespace xAOD {
  /// Latest version of the event-level muon displaced-vertex inference result aux container.
  typedef MuonDVInferenceResultAuxContainer_v1 MuonDVInferenceResultAuxContainer;
}

#include "xAODCore/CLASS_DEF.h"
CLASS_DEF(xAOD::MuonDVInferenceResultAuxContainer, 245900433, 1)

#endif
