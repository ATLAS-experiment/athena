/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonGeoModelR4/MdtReadoutGeomTool.h"
#include "MuonGeoModelR4/TgcReadoutGeomTool.h"
#include "MuonGeoModelR4/RpcReadoutGeomTool.h"
#include "MuonGeoModelR4/MmReadoutGeomTool.h"
#include "MuonGeoModelR4/MuonDetectorTool.h"
#include "MuonGeoModelR4/MuonGeoUtilityTool.h"
#include "MuonGeoModelR4/sTgcReadoutGeomTool.h"
#ifndef SIMULATIONBASE
#   include "MuonGeoModelR4/ChamberAssembleTool.h"
#endif

DECLARE_COMPONENT(MuonGMR4::MuonDetectorTool)
DECLARE_COMPONENT(MuonGMR4::MdtReadoutGeomTool)
DECLARE_COMPONENT(MuonGMR4::TgcReadoutGeomTool)
DECLARE_COMPONENT(MuonGMR4::RpcReadoutGeomTool)
DECLARE_COMPONENT(MuonGMR4::MmReadoutGeomTool)
DECLARE_COMPONENT(MuonGMR4::MuonGeoUtilityTool)
DECLARE_COMPONENT(MuonGMR4::sTgcReadoutGeomTool)
#ifndef SIMULATIONBASE
DECLARE_COMPONENT(MuonGMR4::ChamberAssembleTool)
#endif
