/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONINFERENCE_XAODMUONINFERENCEDICT_H
#define XAODMUONINFERENCE_XAODMUONINFERENCEDICT_H

#include "xAODMuonInference/MuonDVInferenceResult.h"
#include "xAODMuonInference/MuonDVInferenceResultAuxContainer.h"
#include "xAODMuonInference/MuonDVInferenceResultContainer.h"
#include "xAODMuonInference/versions/MuonDVInferenceResult_v1.h"
#include "xAODMuonInference/versions/MuonDVInferenceResultAuxContainer_v1.h"
#include "xAODMuonInference/versions/MuonDVInferenceResultContainer_v1.h"

#include "xAODCore/tools/DictHelpers.h"

namespace {
  struct GCCXML_DUMMY_INSTANTIATION_XAODMUONINFERENCE {
    XAOD_INSTANTIATE_NS_CONTAINER_TYPES(xAOD, MuonDVInferenceResultContainer_v1);
    XAOD_INSTANTIATE_NS_OBJECT_TYPES(xAOD, MuonDVInferenceResult_v1);
  };
}

#endif
