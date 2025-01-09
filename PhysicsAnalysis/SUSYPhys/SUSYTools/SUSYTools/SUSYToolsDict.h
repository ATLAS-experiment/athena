/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef SUSYTOOLS_SUSYTOOLSDICT_H
#define SUSYTOOLS_SUSYTOOLSDICT_H

#include "SUSYTools/SUSYObjDef_xAOD.h"
#include "SUSYTools/SUSYCrossSection.h"

// AthAnalysis doesn't need dictionaries building for components (such as algorithms)
// So only do this overhead for AnalysisBase
#ifdef XAOD_STANDALONE
#include "SUSYTools/SUSYToolsAlg.h"
#endif

#endif // not SUSYTOOLS_SUSYTOOLSDICT_H
