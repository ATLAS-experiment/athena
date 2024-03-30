/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODBTAGGINGEFFICIENCY_XAODBTAGGINGEFFICIENCYDICT_H
#define XAODBTAGGINGEFFICIENCY_XAODBTAGGINGEFFICIENCYDICT_H

// Special handling for Eigen vectorization (relevant because of inclusion of Jet.h)
#if defined(__GCCXML__) and not defined(EIGEN_DONT_VECTORIZE)
#define EIGEN_DONT_VECTORIZE
#endif

#include "xAODBTaggingEfficiency/BTaggingEfficiencyTool.h"
#include "xAODBTaggingEfficiency/BTaggingSelectionTool.h"
#include "xAODBTaggingEfficiency/BTaggingSelectionJsonTool.h"
#include "xAODBTaggingEfficiency/BTaggingTruthTaggingTool.h"
#include "xAODBTaggingEfficiency/BTaggingEigenVectorRecompositionTool.h"
#endif // XAODBTAGGINGEFFICIENCY_XAODBTAGGINGEFFICIENCYDICT_H
