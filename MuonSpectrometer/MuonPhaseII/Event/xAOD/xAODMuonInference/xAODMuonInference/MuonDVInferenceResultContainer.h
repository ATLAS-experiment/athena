/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONINFERENCE_MUONDVINFERENCERESULTCONTAINER_H
#define XAODMUONINFERENCE_MUONDVINFERENCERESULTCONTAINER_H

#include "xAODMuonInference/versions/MuonDVInferenceResultContainer_v1.h"

namespace xAOD {
  /// Latest version of the event-level muon displaced-vertex inference result container.
  typedef MuonDVInferenceResultContainer_v1 MuonDVInferenceResultContainer;
}

#include "xAODCore/CLASS_DEF.h"
CLASS_DEF(xAOD::MuonDVInferenceResultContainer, 245900432, 1)

#endif
