/*
    Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "../eFEXDriver.h"
#include "../eFEXSysSim.h"
#include "../eFEXFillEDM.h"
#include "../eFEXFormTOBs.h"
#include "../eFEXSim.h"
#include "../eFEXFPGA.h"
#include "../eFEXtauAlgo.h"
#include "../eFEXtauBDTAlgo.h"
#include "L1CaloFEXSim/eFEXegAlgo.h"
#include "L1CaloFEXSim/eFEXTOBEtTool.h"
#include "../eFEXFPGATowerIdProvider.h"
#include "../eTowerMakerFromSuperCells.h"
#include "../eTowerMakerFromEfexTowers.h"
#include "L1CaloFEXSim/eFEXSuperCellTowerIdProvider.h"
#include "../eFakeTower.h"


#include "../jFEXDriver.h"
#include "../jFEXSysSim.h"
#include "../jFEXSim.h"
#include "../jFEXFPGA.h"
#include "../jFEXSmallRJetAlgo.h"
#include "../jFEXtauAlgo.h"
#include "../jFEXsumETAlgo.h"
#include "../jFEXmetAlgo.h"
#include "../jFEXLargeRJetAlgo.h"
#include "../jFEXForwardJetsAlgo.h"
#include "../jFEXForwardElecAlgo.h"
#include "../jFEXPileupAndNoise.h"
#include "../jFEXFormTOBs.h"
#include "../jTowerMakerFromJfexTowers.h"

#include "../gFEXDriver.h"
#include "../gFEXSysSim.h"
#include "../gFEXSim.h"
#include "../gFEXFPGA.h"
#include "../gFEXJetAlgo.h"
#include "../gFEXJwoJAlgo.h"
#include "../gFEXaltMetAlgo.h"
#include "../gTowerMakerFromGfexTowers.h"



using namespace LVL1;

DECLARE_COMPONENT(eFEXDriver)
DECLARE_COMPONENT(eFEXSysSim)
DECLARE_COMPONENT(eFEXFillEDM)
DECLARE_COMPONENT(eFEXFormTOBs)
DECLARE_COMPONENT(eFEXSim)
DECLARE_COMPONENT(eTowerBuilder)
DECLARE_COMPONENT(eSuperCellTowerMapper)
DECLARE_COMPONENT(eFEXFPGA)
DECLARE_COMPONENT(eFEXtauAlgo)
DECLARE_COMPONENT(eFEXtauBDTAlgo)
DECLARE_COMPONENT(eFEXegAlgo)
DECLARE_COMPONENT(eFEXTOBEtTool)
DECLARE_COMPONENT(eTowerMakerFromSuperCells)
DECLARE_COMPONENT(eTowerMakerFromEfexTowers)
DECLARE_COMPONENT(eFEXFPGATowerIdProvider)
DECLARE_COMPONENT(eFEXSuperCellTowerIdProvider)
DECLARE_COMPONENT(eFakeTower)

DECLARE_COMPONENT(jFEXDriver)
DECLARE_COMPONENT(jFEXSysSim)
DECLARE_COMPONENT(jFEXSim)
DECLARE_COMPONENT(jTowerBuilder)
DECLARE_COMPONENT(jSuperCellTowerMapper)
DECLARE_COMPONENT(jFEXFPGA)
DECLARE_COMPONENT(jFEXSmallRJetAlgo)
DECLARE_COMPONENT(jFEXtauAlgo)
DECLARE_COMPONENT(jFEXPileupAndNoise)
DECLARE_COMPONENT(jFEXsumETAlgo)
DECLARE_COMPONENT(jFEXmetAlgo)
DECLARE_COMPONENT(jFEXLargeRJetAlgo)
DECLARE_COMPONENT(jFEXForwardJetsAlgo)
DECLARE_COMPONENT(jFEXForwardElecAlgo)
DECLARE_COMPONENT(jFEXFormTOBs)
DECLARE_COMPONENT(jTowerMakerFromJfexTowers)

DECLARE_COMPONENT(gFEXDriver)
DECLARE_COMPONENT(gFEXSysSim)
DECLARE_COMPONENT(gFEXSim)
DECLARE_COMPONENT(gTowerBuilder)
DECLARE_COMPONENT(gSuperCellTowerMapper)
DECLARE_COMPONENT(gFEXFPGA)
DECLARE_COMPONENT(gFEXJetAlgo)
DECLARE_COMPONENT(gFEXJwoJAlgo)
DECLARE_COMPONENT(gFEXaltMetAlgo)
DECLARE_COMPONENT(gTowerMakerFromGfexTowers)



