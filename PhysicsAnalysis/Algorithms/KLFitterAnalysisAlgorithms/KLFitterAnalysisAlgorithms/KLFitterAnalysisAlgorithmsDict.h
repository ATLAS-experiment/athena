/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Oliver Majersky
/// @author Baptiste Ravina

#ifndef KLFITTERANALYSIS_ALGORITHMS_DICT_H
#define KLFITTERANALYSIS_ALGORITHMS_DICT_H

#include "KLFitterAnalysisAlgorithms/KLFitterFinalizeOutputAlg.h"
#include "KLFitterAnalysisAlgorithms/KLFitterResult.h"
#include "KLFitterAnalysisAlgorithms/KLFitterResultAuxContainer.h"
#include "KLFitterAnalysisAlgorithms/KLFitterResultContainer.h"
#include "KLFitterAnalysisAlgorithms/RunKLFitterAlg.h"

// EDM include(s).
#include "xAODCore/tools/DictHelpers.h"
// Instantiate all necessary types for the dictionary.
namespace {
struct GCCXML_DUMMY_INSTANTIATION_KLFITTERANALYSISALGORITHMS {
  // Local type(s).
  XAOD_INSTANTIATE_NS_CONTAINER_TYPES(xAOD, KLFitterResultContainer);
  XAOD_INSTANTIATE_NS_OBJECT_TYPES(xAOD, KLFitterResult);
};
}  // namespace

#endif
