/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TILERECUTILS_TILERAWCHANNELBUILDERMF_H
#define TILERECUTILS_TILERAWCHANNELBUILDERMF_H

/********************************************************************
 *
 * NAME:     TileRawChannelBuilderMF.h 
 * PACKAGE:  offline/TileCalorimeter/TileRecUtils
 *
 * AUTHOR :  B. Peralva
 * CREATED:  September 2012
 *
 * PURPOSE:  Build TileRawChannels from digits using Matched filter
 *
 *  Input: TileDigitsContainer
 *  Output: TileRawChannelContainer
 *  Parameters: TileRawChannelCont - Name of output container in SG
 ********************************************************************/

// Tile includes
#include "TileRecUtils/TileRawChannelBuilder.h" //base class
#include "TileConditions/ITileCondToolOfc.h" //ToolHandle template parameter
#include "TileConditions/TileCondToolNoiseSample.h"  //ToolHandle template parameter
#include <vector>
#include <string>

class TileHWID;
class TileDigits;

class TileRawChannelBuilderMF: public TileRawChannelBuilder {
  public:

    // constructor
    TileRawChannelBuilderMF(const std::string& type, const std::string& name,
        const IInterface *parent);
    // destructor
    ~TileRawChannelBuilderMF();

    // virtual methods
    virtual StatusCode initialize() override;
    virtual StatusCode finalize() override;

    // Inherited from TileRawChannelBuilder
    virtual TileRawChannel * rawChannel(const TileDigits* digits, const EventContext& ctx) override;

  private:

    ToolHandle<ITileCondToolOfc> m_tileCondToolOfc{this,
        "TileCondToolOfc", "TileCondToolOfc", "Tile OFC tool"};

    ToolHandle<ITileCondToolOfc> m_tileCondToolOfcOnFly{this,
        "TileCondToolOfcOnFly", "TileCondToolOfc", "Tile OFC (calculated on the fly) tool"};

    ToolHandle<TileCondToolNoiseSample> m_tileToolNoiseSample{this,
        "TileCondToolNoiseSample", "TileCondToolNoiseSample", "Tile noise sample tool"};

    bool are3FF(float &dmin, float &dmax); //!< Checks that all the samples are 0x3FF (as sent by the DSP when no data arrives)

    Gaudi::Property<int> m_maxIterations{this, "MaxIterations", 5, "Maximum number of iteration to perform"}; //!< maximum number of iteration to perform
    Gaudi::Property<bool> m_correctAmplitude{this, "AmplitudeCorrection", false,
        "Correct resulting amplitude when using weights for tau=0 without iteration"}; //!< If true, resulting amplitude is corrected when using weights for tau=0 without iteration
    Gaudi::Property<int> m_pedestalMode{this, "PedestalMode", 1,
        "Pedestal mode: -1 pedestal from conditions, 0 - fixed pedestal, 1 (default) pedestal from data"};	  // -1 pedestal from conditions, 0 - fixed pedestal, 1 (default) pedestal from data
    Gaudi::Property<double> m_defaultPedestal{this, "DefaultPedestal", 0.0, "Fixed pedestal value to use"};  // use a fixed pedestal value
    Gaudi::Property<int> m_MF{this, "MF", 0, "0 - COF, 1 - MF (created for muon receiver board simulation)"}; // 0 - COF, 1 - MF (created for muon receiver board simulation)
    int m_nSamples; //!< number of samples in the data
    int m_t0SamplePosition;  //!< position of peak sample = (m_nSamples-1)/2
    double m_maxTime; //!< max allowed time = 25*(m_nSamples-1)/2
    double m_minTime; //!< min allowed time = -25*(m_nSamples-1)/2

    Gaudi::Property<bool> m_bestPhase{this, "BestPhase", false,
        "Use best phase from COOL DB in fixed phase mode (i.e., no iterations)"}; // if true, use best phase from COOL DB in "fixed phase" mode (i.e., no iterations)
    Gaudi::Property<bool> m_timeFromCOF{this, "TimeFromCOF", false, "Take time estimated from second step of COF"}; // if true, take time estimated from second step of COF

    std::vector<float> m_digits;
    int m_chPedCounter[5][64][48][2]{};
    float m_chPed[5][64][48][2]{};

};

#endif
