/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

//****************************************************************************
// Filename : TileDigitsToTTL1.h
// Author   : Pedro Amaral,.following TileHitToTTl1
// Created  : Feb 2005
//
// DESCRIPTION
// 
// Creates TileTTL1 Digits from TileDigits (data)
//
// Properties (JobOption Parameters):
//
//    TileDigitsContainer       string   Name of container with TileDigits to read
//    TileTTL1Container         string   Name of container with TileTTL1Digits to write
//    TileInfoName              string   Name of object in TDS with all parameters
//
// BUGS:
//  
// History:
//  
//  
//****************************************************************************

#ifndef TILERECALGS_TILEDIGITSTOTTL1_H
#define TILERECALGS_TILEDIGITSTOTTL1_H

// Tile includes
#include "TileIdentifier/TileFragHash.h"
#include "TileEvent/TileDigitsContainer.h"
#include "TileEvent/TileTTL1Container.h"

// Atlas includes
#include "AthenaBaseComps/AthAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

// Gaudi includes
#include "GaudiKernel/ToolHandle.h"

#include <string>

class TileID;
class TileHWID;
class TileInfo;
class CaloLVL1_ID;
class TileCablingService;
class TileCondToolEmscale;

class TileDigitsToTTL1: public AthAlgorithm {
  public:
    // Constructor
    TileDigitsToTTL1(const std::string& name, ISvcLocator* pSvcLocator);

    //Destructor 
    virtual ~TileDigitsToTTL1() = default;

    //Gaudi Hooks
    virtual StatusCode initialize() override;
    virtual StatusCode execute() override;
    virtual StatusCode finalize() override;

  private:

    SG::ReadHandleKey<TileDigitsContainer> m_digitsContainerKey{this,"TileDigitsContainer",
                                                                "TileDigitsCnt", "Input Tile digits container key"};

    SG::WriteHandleKey<TileTTL1Container> m_ttl1ContainerKey{this,"TileTTL1Container",
                                                            "TileTTL1Container","Output Tile TTL1 container key"};

    StringProperty m_infoName{this,"TileInfoName", "TileInfo","Name of TileInfo object in TES"}; 

    const TileID* m_tileID{};
    const TileHWID* m_tileHWID{};
    const TileInfo* m_tileInfo{};
    const CaloLVL1_ID* m_TT_ID{};

    ToolHandle<TileCondToolEmscale> m_tileToolEmscale{this,"TileCondToolEmscale","TileCondToolEmscale","Main Tile Calibration tool"};
};

#endif // TILERECALGS_TILEDIGITSTOTTL1_H
