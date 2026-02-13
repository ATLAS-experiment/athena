/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "../MdtReadoutGeomTool.h"
#include "../TgcReadoutGeomTool.h"
#include "../RpcReadoutGeomTool.h"
#include "../MmReadoutGeomTool.h"
#include "../MuonDetectorTool.h"
#include "../MuonGeoUtilityTool.h"
#include "../sTgcReadoutGeomTool.h"
#include "../ToroidDetectorTool.h"
#ifndef SIMULATIONBASE
#   include "../ChamberAssembleTool.h"
#endif

DECLARE_COMPONENT(MuonGMR4::MuonDetectorTool)
DECLARE_COMPONENT(MuonGMR4::ToroidDetectorTool)
DECLARE_COMPONENT(MuonGMR4::MdtReadoutGeomTool)
DECLARE_COMPONENT(MuonGMR4::TgcReadoutGeomTool)
DECLARE_COMPONENT(MuonGMR4::RpcReadoutGeomTool)
DECLARE_COMPONENT(MuonGMR4::MmReadoutGeomTool)
DECLARE_COMPONENT(MuonGMR4::MuonGeoUtilityTool)
DECLARE_COMPONENT(MuonGMR4::sTgcReadoutGeomTool)
#ifndef SIMULATIONBASE
DECLARE_COMPONENT(MuonGMR4::ChamberAssembleTool)
#endif
