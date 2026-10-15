/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonMomentumCorrections/MuonCalibTool.h"
#include "MuonMomentumCorrections/MuonCalibIntSagittaTool.h"
#include "MuonMomentumCorrections/MuonCalibIntHighpTSmearTool.h"
#include "MuonMomentumCorrections/MuonCalibIntScaleSmearTool.h"

#ifndef XAOD_STANDALONE
#include "../CalibratedMuonsProvider.h"
#include "../CalibratedTracksProvider.h"
#include "../TestMCASTTool.h"
#endif

// Project include(s).
#include "AsgTools/AsgComponentFactories.h"

DECLARE_COMPONENT(ADD_CMODE(CP)::MuonCalibTool)
DECLARE_COMPONENT(ADD_CMODE(CP)::MuonCalibIntSagittaTool)
DECLARE_COMPONENT(ADD_CMODE(CP)::MuonCalibIntHighpTSmearTool)
DECLARE_COMPONENT(ADD_CMODE(CP)::MuonCalibIntScaleSmearTool)

#ifndef XAOD_STANDALONE
DECLARE_COMPONENT(ADD_CMODE(CP)::TestMCASTTool)
DECLARE_COMPONENT(ADD_CMODE(CP)::CalibratedMuonsProvider)
DECLARE_COMPONENT(ADD_CMODE(CP)::CalibratedTracksProvider)
#endif
