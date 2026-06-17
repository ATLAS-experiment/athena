/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONINFERENCE_MUONDVINFERENCERESULT_H
#define XAODMUONINFERENCE_MUONDVINFERENCERESULT_H

#include "xAODMuonInference/versions/MuonDVInferenceResult_v1.h"

namespace xAOD {
  /// Latest version of the event-level muon displaced-vertex inference result.
  typedef MuonDVInferenceResult_v1 MuonDVInferenceResult;
}

#include "xAODCore/CLASS_DEF.h"
CLASS_DEF(xAOD::MuonDVInferenceResult, 245900431, 1) // TODO : Change class id in the case of assigned/checked ATLAS conventions 

#endif
