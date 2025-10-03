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

DECLARE_COMPONENT(CP::MuonCalibTool)
DECLARE_COMPONENT(CP::MuonCalibIntSagittaTool)
DECLARE_COMPONENT(CP::MuonCalibIntHighpTSmearTool)
DECLARE_COMPONENT(CP::MuonCalibIntScaleSmearTool)

#ifndef XAOD_STANDALONE
DECLARE_COMPONENT(CP::TestMCASTTool)
DECLARE_COMPONENT(CP::CalibratedMuonsProvider)
DECLARE_COMPONENT(CP::CalibratedTracksProvider)
#endif
