/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

//****************************************************************************
// Filename : TileRawChannelVerify.h
// Author   : UC-ATLAS TileCal group
// Created  : May 2002
//
// DESCRIPTION
//    to verify the rawChannels reconstructed from different route or data 
//    source, for example, hit->rawchannel, hit->digits->rawchannel, etc.
//    Every time it can only compare the results from two routes.
//    Print out the different on the console.
// 
// Properties (JobOption Parameters):
//
//    TileRawChannelContainer1  string  Name of RawChannel Container created
//                                      by the first route for read
//    TileRawChannelContainer2  string  Name of RawChannel Container created
//                                      by the second route for read
//
// BUGS:
//  
// History:
//  
//  
//****************************************************************************

#ifndef TILERECUTILS_TILERAWCHANNELVERIFY_H
#define TILERECUTILS_TILERAWCHANNELVERIFY_H

// Tile includes
#include "TileEvent/TileRawChannelContainer.h"

// Atlas includes
#include "AthenaBaseComps/AthAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"

class TileHWID;

#include <string>

/**
 @class TileRawChannelVerify
 @brief This class compares two sets of TileRawChannels

 It is meant to check if both sets of TileRawChannels contain the same information, since the output is just one line confirming that. Optionally the differences are simply dumped to the screen. They TileRawChannels may have been obtained from different input data or from the same data reconstructed by different methods.
 */
class TileRawChannelVerify: public AthAlgorithm {
  public:
    
    TileRawChannelVerify(const std::string& name, ISvcLocator* pSvcLocator); //!< Constructor

    virtual ~TileRawChannelVerify();  //!< Destructor                         
    
    StatusCode initialize();  //!< initialize method 
    StatusCode execute(const EventContext& ctx);     //!< execute method 
    StatusCode finalize();    //!< finalize method

  private:

    const TileHWID* m_tileHWID; //!< Pointer to TileHWID  

    SG::ReadHandleKey<TileRawChannelContainer> m_rawChannelContainer1Key{this, "TileRawChannelContainer1", 
                                                                         "", "Input Tile raw channel container 1 key"};


    SG::ReadHandleKey<TileRawChannelContainer> m_rawChannelContainer2Key{this, "TileRawChannelContainer2", 
                                                                         "", "Input Tile raw channel container 2 key"};


    Gaudi::Property<double> m_precision{this, "Precision", 0,
        "Maximum difference between the amplitudes of the TileRawChannels to be compared"};

    Gaudi::Property<bool> m_dumpRawChannels{this, "DumpRawChannels", false,
        "If true => Differences found in the TileRawChannels are dumped on the screen"};
    Gaudi::Property<bool> m_sortFlag{this, "SortFlag", false,
        "If true => TileRawChannels are sorted by amplitude"};
};

#endif // not TILERECUTILS_TILERAWCHANNELVERIFY_H
