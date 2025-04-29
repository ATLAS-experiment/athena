/*
    Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Oliver Majersky
/// @author Baptiste Ravina

#ifndef KLFITTERNANALYSISALGORITHMS_KLFITTERRESULTCONTAINER_H_
#define KLFITTERNANALYSISALGORITHMS_KLFITTERRESULTCONTAINER_H_

// EDM include(s).
#include "AthContainers/DataVector.h"
#include "xAODCore/CLASS_DEF.h"

// Local include(s).
#include "KLFitterAnalysisAlgorithms/KLFitterResult.h"

namespace xAOD {

/// Definition of the @c KLFitterResultContainer type
typedef DataVector<xAOD::KLFitterResult> KLFitterResultContainer;

}  // namespace xAOD

// Define a ClassID for the type.
CLASS_DEF(xAOD::KLFitterResultContainer, 1116647492, 1)

#endif
