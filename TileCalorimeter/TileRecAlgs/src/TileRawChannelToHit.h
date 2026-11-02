/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

//****************************************************************************
// Filename : TileRawChannelToHit.h
// Author   : Alexander Solodkov
// Created  : Feb 2005
//
// DESCRIPTION
// 
//    Convert RawChannel amplitude to total energy (like in TileCell)
// but save it as TileHit 

// Properties (JobOption Parameters):
//
//    TileRawChannelContainer   string   Name of container with TileRawChannel to read
//    TileHitContainer          string   Name of TileHitContainer to write
//
// BUGS:
//  
// History:
//   15Feb05  Created from TileRawChannelToCell
//  
//****************************************************************************

#ifndef TILERECALGS_TILERAWCHANNELTOHIT_H
#define TILERECALGS_TILERAWCHANNELTOHIT_H

// Tile includes
#include "TileEvent/TileRawChannelContainer.h"
#include "TileSimEvent/TileHitVector.h"
#include "TileConditions/TileSamplingFraction.h"
#include "TileConditions/TileCondToolEmscale.h"

// Atlas includes
#include "AthenaBaseComps/AthAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/ReadCondHandleKey.h"

// Gaudi includes
#include "GaudiKernel/ToolHandle.h"

// C++ STL includes
#include <string>

// Avoid pushing dependencies into clients- just fwd declare the following:

class TileID;
class TileHWID;
class TileHit;
class TileCablingService;


class TileRawChannelToHit: public AthAlgorithm {
  public:
    // Constructor
    TileRawChannelToHit(const std::string& name, ISvcLocator* pSvcLocator);

    //Destructor 
    virtual ~TileRawChannelToHit();

    //Gaudi Hooks
    StatusCode initialize();
    StatusCode execute(const EventContext& ctx);
    StatusCode finalize();

  private:

    SG::ReadHandleKey<TileRawChannelContainer> m_rawChannelContainerKey{this,"TileRawChannelContainer",
                                                                        "TileRawChannelCnt", 
                                                                        "Input Tile raw channel container key"};

    SG::WriteHandleKey<TileHitVector> m_hitVectorKey{this,"TileHitContainer",
                                                      "TileHitVec","Output Tile hit container key"};


    /**
     * @brief Name of TileSamplingFraction in condition store
     */
    SG::ReadCondHandleKey<TileSamplingFraction> m_samplingFractionKey{this,
        "TileSamplingFraction", "TileSamplingFraction", "Input Tile sampling fraction"};

    std::string m_infoName;
    // if true energy in TileHit is the as it is coming from G4 simulation
    // and by default it is equal to final - TileCell energy
    Gaudi::Property<bool> m_useSamplFract{this, "UseSamplFract", false, "Take sampling fraction into account for TileHit energy"};

    const TileID* m_tileID{nullptr};
    const TileHWID* m_tileHWID{nullptr};

    ToolHandle<TileCondToolEmscale> m_tileToolEmscale{this, "TileCondToolEmscale",
        "TileCondToolEmscale", "Tile EM scale calibration tool"}; //!< main Tile Calibration tool
};

#endif // TILERECALGS_TILERAWCHANNELTOHIT_H
