/*
 Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
 */
// Local include(s):
#include "MuonEfficiencyCorrections/MuonEfficiencyScaleFactors.h"
#include "MuonEfficiencyCorrections/MuonTriggerScaleFactors.h"

// MCP tester macros
#ifndef XAOD_STANDALONE
#include "../MuonScaleFactorTestAlg.h"
#include "../MuonCloseJetDecorationAlg.h"
#endif

// Project include(s).
#include "AsgTools/AsgComponentFactories.h"

DECLARE_COMPONENT( CP::MuonEfficiencyScaleFactors )
DECLARE_COMPONENT( CP::MuonTriggerScaleFactors )

#ifndef XAOD_STANDALONE
DECLARE_COMPONENT( CP::MuonScaleFactorTestAlg )
DECLARE_COMPONENT( CP::MuonCloseJetDecorationAlg )
#endif
