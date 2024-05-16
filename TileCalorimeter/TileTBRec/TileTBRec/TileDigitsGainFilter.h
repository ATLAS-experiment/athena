/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

//****************************************************************************
// Filename : TileDigitsGainFilter.h
//
// DESCRIPTION
// 
// Copy TileDigits from input container to output container
//
// Properties (JobOption Parameters):
//
//    InputDigitsContainer      string   Name of container with TileDigits to read
//    OutputDigitsContainer     string   Name of container with TileDigits to write
//
//
// BUGS:
//  
// History:
//  
//  
//****************************************************************************

#ifndef TILETBREC_TILEDIGITSGAINFILTER_H
#define TILETBREC_TILEDIGITSGAINFILTER_H

#include "TileEvent/TileDigitsContainer.h"

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

class TileHWID;

/**
 @class TileDigitsGainFilter
 @brief This algorithm copies TileDigits from input container to output container
 */
class TileDigitsGainFilter: public AthReentrantAlgorithm {
  public:
    // Constructor
    using AthReentrantAlgorithm::AthReentrantAlgorithm;

    //Destructor 
    virtual ~TileDigitsGainFilter() = default;

    //Gaudi Hooks
    StatusCode initialize() override; //!< initialize method
    StatusCode execute(const EventContext& ctx) const override;    //!< execute method
    StatusCode finalize() override;   //!< finalize method

  private:

    SG::ReadHandleKey<TileDigitsContainer> m_inputContainerKey{this,
        "InputDigitsContainer", "TileDigitsCnt", "Input Tile digits container key"};

    SG::WriteHandleKey<TileDigitsContainer> m_outputContainerKey{this,
        "OutputDigitsContainer", "TileDigitsFiltered", "Output Tile digits container key"};

    Gaudi::Property<int> m_threshold{this,
        "HighGainThreshold", 4095, "Threshold to check overflowes in high gain"};

    const TileHWID* m_tileHWID{nullptr};

};

#endif // TILETBREC_TILEDIGITSGAINFILTER_H
