/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

//****************************************************************************
// Filename : TileRawChannelToTTL1.h
// Author   : Pedro Amaral, based on TileHitToTTL1
// Created  : Feb 2005
//
// DESCRIPTION
// 
// Creates TileTTL1 Digits from TileRawChannel
//
// Properties (JobOption Parameters):
//
//    TileRawChannelContainer          string   Name of container with TileRawChannel to read
//    TileTTL1Container         string   Name of container with TileTTL1Digits to write
//    TileInfoName              string   Name of object in TDS with all parameters
//
// BUGS:
//  
// History:
//  
//  
//****************************************************************************

#ifndef TILERECALGS_TILERAWCHANNELTOTTL1_H
#define TILERECALGS_TILERAWCHANNELTOTTL1_H

// Tile includes
//#include "TileEvent/TileRawChannelContainer.h"
#include "TileEvent/TileTTL1Container.h"
#include "TileConditions/ITileBadChanTool.h"
#include "TileConditions/TileCondToolEmscale.h"

// Atlas includes
#include "AthenaBaseComps/AthAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

// Gaudi includes
#include "GaudiKernel/ToolHandle.h"


class TileID;
class TileHWID;
class TileInfo;
class CaloLVL1_ID;
class TileCablingService;
class TileRawChannelContainer;

#include <string>

class TileRawChannelToTTL1: public AthAlgorithm {
  public:
    // Constructor
    TileRawChannelToTTL1(const std::string& name, ISvcLocator* pSvcLocator);

    //Destructor 
    virtual ~TileRawChannelToTTL1();

    //Gaudi Hooks
    StatusCode initialize();
    StatusCode execute(const EventContext& ctx);
    StatusCode finalize();

  private:

    SG::ReadHandleKey<TileRawChannelContainer> m_rawChannelContainerKey{this,"TileRawChannelContainer",
                                                                        "TileRawChannelCnt", 
                                                                        "Input Tile raw channel container key"};

    SG::WriteHandleKey<TileTTL1Container> m_ttl1ContainerKey{this,"TileTTL1Container",
                                                             "TileTTL1Container","Output Tile TTL1 container key"};

    Gaudi::Property<std::string> m_infoName{this, "TileInfoName", "TileInfo", "Name of TileInfo object in TES"};
    // true by default, so only gets ttl1 shape once at initialization, with phase=0 for all towers.
    Gaudi::Property<bool> m_constantTTL1shape{this, "TileConstantTTL1Shape", true, "Use constant TTL1 shape (phase=0 for all towers)"};

    const TileID* m_tileID{nullptr};
    const TileHWID* m_tileHWID{nullptr};
    const TileInfo* m_tileInfo{nullptr};
    const CaloLVL1_ID* m_TT_ID{nullptr};

    std::vector<double> m_TTL1Shape;
    double m_phase{0};
    int m_nSamp{0};
    int m_iTrig{0};

    ToolHandle<ITileBadChanTool> m_tileBadChanTool{this, "TileBadChanTool",
        "TileBadChanTool", "Tile bad channel tool"}; //!< Tile Bad Channel tool
    ToolHandle<TileCondToolEmscale> m_tileToolEmscale{this, "TileCondToolEmscale",
        "TileCondToolEmscale", "Tile EM scale calibration tool"}; //!< main Tile Calibration tool
};

#endif // TILERECALGS_TILERAWCHANNELTOTTL1_H
