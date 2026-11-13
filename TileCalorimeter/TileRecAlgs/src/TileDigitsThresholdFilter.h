/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

//****************************************************************************
// Filename : TileDigitsThresholdFilter.h
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

#ifndef TILERECALGS_TILEDIGITSTHRESHOLDFILTER_H
#define TILERECALGS_TILEDIGITSTHRESHOLDFILTER_H

// Tile includes
#include "TileEvent/TileDigitsContainer.h"

// Atlas includes
#include "AthenaBaseComps/AthAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

// Gaudi includes
#include "GaudiKernel/ToolHandle.h"
#include "TileConditions/ITileCondToolDspThreshold.h"

class TileHWID;


#include <string>
#include <vector>

/** 
 @class TileDigitsThresholdFilter
 @brief This algorithm copies TileDigits from input container to output container
 */
class TileDigitsThresholdFilter: public AthAlgorithm {
  public:
    // Constructor
    TileDigitsThresholdFilter(const std::string& name, ISvcLocator* pSvcLocator);

    //Destructor 
    virtual ~TileDigitsThresholdFilter();

    //Gaudi Hooks
    StatusCode initialize(); //!< initialize method
    StatusCode execute(const EventContext& ctx);    //!< execute method
    StatusCode finalize();   //!< finalize method

  private:

    const TileHWID* m_tileHWID{nullptr};
    PublicToolHandle<ITileCondToolDspThreshold> m_tileDspThreshold{this, "TileCondToolDspThreshold", "TileCondToolDspThreshold"};

    SG::ReadHandleKey<TileDigitsContainer> m_inputDigitsContainerKey{this,"InputDigitsContainer",
                                                                       "TileDigitsCnt", "Input Tile digits container key"};

    SG::WriteHandleKey<TileDigitsContainer> m_outputDigitsContainerKey{this,"OutputDigitsContainer",
                                                                         "TileDigitsFiltered","Output Tile digits container key"};

};

#endif // TILERECALGS_TILEDIGITSTHRESHOLDFILTER_H
