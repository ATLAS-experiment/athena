/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TILERECUTILS_TILERAWCHANNELBUILDEROPT2FILTER_H
#define TILERECUTILS_TILERAWCHANNELBUILDEROPT2FILTER_H

//////////////////////////////////////////////////////////////////////
//
//     Based on the code of Ximo Poveda@cern.ch. June 2007
//     Andrei.Artamonov@cern.ch, July 2008
//
//     TileRawChannelBuilderOpt2Filter.h
//
//     implementation of the Optimal Filtering based on Lagrange multipliers
//       for energy/time reconstruction in TileCal 
//
//////////////////////////////////////////////////////////////////////

// Tile includes
#include "TileRecUtils/TileRawChannelBuilder.h"
#include "TileConditions/ITileCondToolOfc.h"
#include "TileConditions/TileCondToolTiming.h"
#include "TileConditions/TileCondToolNoiseSample.h"

#include <vector>
#include <string>

/**
 *
 * @class TileRawChannelBuilderOpt2Filter
 * @brief Reconstructs Tile digitized pulses (ie, computes amplitude, time and pedestal) as a linear combination of the samples
 *
 * This class implements an energy reconstruction method known as Optimal
 * Filtering. It takes as input the digital samples from the digitizer boards
 * in the front-end electronics and outputs the amplitude, time and pedestal
 * of the pulse. Full details and fundaments of the method can be found in
 * the ATL-TILECAL-2005-001 note. Two different versions of the algorithms
 * are currently used: OF1 (with 2 parameters: amplitude and time) and OF2
 * (with 3 parameters (amplitude, time and pedestal).
 *
 * OFCs are calculated on-the-fly (using TileCondToolOfc) or are extracted
 * from COOL database (TileCondToolOfcCool). In case of non-iterative
 * procedure, optionally, the initial, "best phase", can be extracted
 * from COOL DB by means of TileCondToolTiming.
 */
class TileRawChannelBuilderOpt2Filter: public TileRawChannelBuilder {
  public:

    TileRawChannelBuilderOpt2Filter(const std::string& type, const std::string& name,
        const IInterface *parent); //!< Constructor
    ~TileRawChannelBuilderOpt2Filter(); //!< Destructor

    // virtual methods
    virtual StatusCode initialize(); //!< Initialize method
    //virtual StatusCode execute();
    virtual StatusCode finalize(); //!< Finalize method

    // Inherited from TileRawChannelBuilder
    virtual TileRawChannel* rawChannel(const TileDigits* digits, const EventContext& ctx);

    /**
     * AlgTool InterfaceID
     */
    static const InterfaceID& interfaceID();

  private:

    ToolHandle<ITileCondToolOfc> m_tileCondToolOfc{this,
        "TileCondToolOfc", "TileCondToolOfc", "Tile OFC tool"};

    ToolHandle<TileCondToolNoiseSample> m_tileToolNoiseSample{this,
        "TileCondToolNoiseSample", "TileCondToolNoiseSample", "Tile noise sample tool"};

    //!< Applies OF algorithm
    double filter(int ros, int drawer, int channel, int &gain, double &pedestal, double &amplitude, double &time, const EventContext &ctx);
    int findMaxDigitPosition();  //!< Finds maximum digit position in the pulse
    //!< Gets pedestal estimation for OF1
    float getPedestal(int ros, int drawer, int channel, int gain, const EventContext &ctx);
    //!< Apply the number of iterations needed for reconstruction by calling the Filter method
    int iterate(int ros, int drawer, int channel, int gain, double &pedestal, double &amplitude, double &time, double &chi2, const EventContext &ctx);
    //!< Computes A,time,ped using OF. If iterations are required, the Iterator method is used
    double compute(int ros, int drawer, int channel, int gain, double &pedestal, double &amplitude, double &time, double& phase, const EventContext &ctx);

    void ofc2int(int nDigits, double* w_off, short* w_int, short& scale); // convert weights to dsp short int format

    Gaudi::Property<int> m_maxIterations{this, "MaxIterations", 5, "Maximum number of iteration to perform"}; //!< maximum number of iteration to perform
    Gaudi::Property<int> m_pedestalMode{this, "PedestalMode", 17, "Pedestal mode to use"};  //!< pedestal mode to use
    Gaudi::Property<bool> m_confTB{this, "ConfTB", false, "Use testbeam configuration"};       //!< use testbeam configuration
    Gaudi::Property<double> m_timeForConvergence{this, "TimeForConvergence", 0.5,
        "Minimum time difference to quit iteration procedure"}; //!< minimum time difference to quit iteration procedure
    Gaudi::Property<bool> m_of2{this, "OF2", true, "OF method: true => OF2; false => OF1"};    //!< bool variable for OF method: true=> OF2;  false=> OF1
    Gaudi::Property<bool> m_minus1Iter{this, "Minus1Iteration", false,
        "Apply -1 iteration (initial phase guess)"};   //!< bool variable for whether to apply -1 iteration (initial phase guess)
    Gaudi::Property<bool> m_correctAmplitude{this, "AmplitudeCorrection", false,
        "Correct resulting amplitude when using weights for tau=0 without iteration"}; //!< If true, resulting amplitude is corrected when using weights for tau=0 without iteration
    Gaudi::Property<bool> m_correctTimeNI{this, "TimeCorrection", false,
        "Correct resulting time when using method without iteration"}; //!< If true, resulting time is corrected when using method  without iteration

    Gaudi::Property<bool> m_bestPhase{this, "BestPhase", false,
        "Use best phase from COOL DB in fixed phase mode (i.e., no iterations)"}; // if true, use best phase from COOL DB in "fixed phase" mode (i.e., no iterations)
    Gaudi::Property<bool> m_emulateDsp{this, "EmulateDSP", false, "Emulate DSP reconstruction algorithm"}; // if true, emulate DSP reconstruction algorithm
    int m_nSignal; //!< internal counters
    int m_nNegative;  //!< internal counters
    int m_nCenter; //!< internal counters
    int m_nConst;  //!< internal counters

    int m_nSamples; //!< number of samples in the data
    int m_t0SamplePosition;  //!< position of peak sample = (m_nSamples-1)/2
    Gaudi::Property<double> m_maxTime{this, "MaxTime", -1.0, "Max allowed time"}; //!< max allowed time = 25*(m_nSamples-1)/2
    Gaudi::Property<double> m_minTime{this, "MinTime", 0.0, "Min allowed time"}; //!< min allowed time = -25*(m_nSamples-1)/2


    std::vector<float> m_digits;

    Gaudi::Property<int> m_noiseThresholdHG{this, "NoiseThresholdHG", 5, "Noise threshold for high gain"};
    Gaudi::Property<int> m_noiseThresholdLG{this, "NoiseThresholdLG", 3, "Noise threshold for low gain"};
};

#endif
