/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "IsolationSelection/IsolationCloseByCorrectionTool.h"
#include "IsolationSelection/IsolationLowPtPLVTool.h"
#include "IsolationSelection/IsolationSelectionTool.h"

#ifndef XAOD_STANDALONE
#include "../IsoCloseByCaloDecorAlg.h"
#include "../IsoCloseByCorrectionTrkSelAlg.h"
#include "../IsoCloseByCorrectionAlg.h"
#include "../TestIsolationAthenaAlg.h"
#include "../TestIsolationCloseByCorrAlg.h"
#endif

// Project include(s).
#include "AsgTools/AsgComponentFactories.h"

DECLARE_COMPONENT(CP::IsolationCloseByCorrectionTool)
DECLARE_COMPONENT(CP::IsolationLowPtPLVTool)
DECLARE_COMPONENT(CP::IsolationSelectionTool)

#ifndef XAOD_STANDALONE
DECLARE_COMPONENT(CP::TestIsolationAthenaAlg)
DECLARE_COMPONENT(CP::TestIsolationCloseByCorrAlg)
DECLARE_COMPONENT(CP::IsoCloseByCorrectionTrkSelAlg)
DECLARE_COMPONENT(CP::IsoCloseByCorrectionAlg)
DECLARE_COMPONENT(CP::IsoCloseByCaloDecorAlg)
#endif
