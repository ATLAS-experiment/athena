/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonSelectorTools/MuonSelectionTool.h"

#ifndef XAOD_STANDALONE
#include "../MuonQualityUpdaterAlg.h"
#include "../MuonSelectionAlg.h"
#endif

// Project include(s).
#include "AsgTools/AsgComponentFactories.h"

DECLARE_COMPONENT(CP::MuonSelectionTool)

#ifndef XAOD_STANDALONE
DECLARE_COMPONENT(CP::MuonSelectionAlg)
DECLARE_COMPONENT(CP::MuonQualityUpdaterAlg)
#endif
