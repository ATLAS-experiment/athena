/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TILERECUTILS_TILERAWCHANNELBUILDERQIEFILTER_H
#define TILERECUTILS_TILERAWCHANNELBUILDERQIEFILTER_H

//////////////////////////////////////////////////////////////////////
//
//     Author:
//     Alexander.Paramonov@cern.ch
//
//     Created:
//    05 February 2016
//
//  	 File Name:
//     TileRawChannelBuilderQIEFilter.h
//
//    Description:
//		The code calculates time and energy/charge of QIE pulses
//
//////////////////////////////////////////////////////////////////////

// Tile includes
#include "TileRecUtils/TileRawChannelBuilder.h"
#include "TileConditions/ITileCondToolOfc.h"
#include "TileConditions/TileCondToolOfcCool.h"
#include "TileConditions/TileCondToolTiming.h"
#include "TileConditions/TileCondToolNoiseSample.h"

#include <vector>
#include <string>

/**
 *
 * @class TileRawChannelBuilderQIEFilter
 * @brief Reconstructs Tile digitized pulses (ie, computes amplitude and time) for the QIE front-end
 *
 */

class TileRawChannelBuilderQIEFilter: public TileRawChannelBuilder {
  public:

    TileRawChannelBuilderQIEFilter(const std::string& type, const std::string& name, const IInterface *parent); //!< Constructor
    ~TileRawChannelBuilderQIEFilter(); //!< Destructor

    // virtual methods
    virtual StatusCode initialize() override; //!< Initialize method

    // Inherited from TileRawChannelBuilder
    virtual TileRawChannel* rawChannel(const TileDigits* digits, const EventContext& ctx) override;

    /**
     * AlgTool InterfaceID
     */
    static const InterfaceID& interfaceID();

  private:

    //!< Applies OF algorithm
    float filter(int ros, int drawer, int channel, float &amplitude, float &time);
    int findMaxDigitPosition();  //!< Finds maximum digit position in the pulse

    //!< Computes A,time,ped using OF. If iterations are required, the Iterator method is used
    float compute(int ros, int drawer, int channel, float &amplitude, float &time);

    Gaudi::Property<int> m_pedestalMode{this, "PedestalMode", 17, "Pedestal mode to use"};  //!< pedestal mode to use
    int m_nSignal{}; //!< internal counters
    int m_nConst{};  //!< internal counters

    int m_nSamples{}; //!< number of samples in the data

    std::vector<float> m_digits;
};

#endif
