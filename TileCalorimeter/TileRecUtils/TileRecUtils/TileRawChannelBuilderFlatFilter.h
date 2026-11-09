/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TILERECUTILS_TILERAWCHANNELBUILDERFLATFILTER_H
#define TILERECUTILS_TILERAWCHANNELBUILDERFLATFILTER_H

/********************************************************************
 *
 * NAME:     TileRawChannelBuilderFlatFilter.h 
 * PACKAGE:  offline/TileCalorimeter/TileRecUtils
 *
 * AUTHOR :  K. Gellerstedt
 * CREATED:  Oct 17 2003
 *
 * PURPOSE:  Build TileRawChannels from digits using flat filter
 *
 *  Input: TileDigitsContainer
 *  Output: TileRawChannelContainer
 *  Parameters: TileRawChannelCont - Name of output container in SG
 *   PedStart
 *   PedLength
 *   PedOffset
 *   SignalStart
 *   SignalLength
 *   FilterLength
 *   FrameLength
 *   DeltaCutLo
 *   DeltaCutHi
 *   RMSCutLo
 *   RMSCutHi
 ********************************************************************/

// Tile includes
#include "TileRecUtils/TileRawChannelBuilder.h"

#include <inttypes.h>

class TileRawChannelBuilderFlatFilter: public TileRawChannelBuilder {
  public:

    // constructor
    TileRawChannelBuilderFlatFilter(const std::string& type, const std::string& name,
        const IInterface *parent);
    // destructor
    ~TileRawChannelBuilderFlatFilter();

    // virtual methods
    virtual StatusCode initialize();
    virtual StatusCode finalize();

    // Inherited from TileRawChannelBuilder
    virtual TileRawChannel * rawChannel(const TileDigits* digits, const EventContext& ctx);

    /**
     * Filter given digits using FlatFilter method
     * @param digits as vector of unsigned 32 bit integers
     * @param amplitude
     * @param time
     */
    void flatFilter(const std::vector<uint32_t> &digits, const int gain
                    , double& amplitude, double& time) const;

    /**
     * Filter given digits using FlatFilter method
     * @param digits as vector of double
     * @param amplitude
     * @param time
     */
    void flatFilter(const std::vector<float> &digits, const int gain
                    , double& amplitude, double& time) const;

    /**
     * AlgTool InterfaceID
     */
    static const InterfaceID& interfaceID();

  private:

    /**
     * Find min and max sample
     * Calculate mean and RMS of all samples.
     * Returns true i signal is found
     * @param digits Vector of digits(double)
     * @param delatCut Delta cut value
     * @param rmsCut RMS cut value
     * @param frameMax Storage for max sample
     * @param frameMin Storage for frame min
     * @param frameMean Storage for frame mean
     * @param frameRMS Storage for frame RMS
     * @return If signal was found
     */
    bool isSignalInFrame(const std::vector<float> &digits, double deltaCut, double rmsCut,
                          int &frameMax, int &frameMin, double &frameMean, double &frameRMS) const;
    /**
     * Calculate pedestal as mean of <pedLgt> samples
     * starting from <pedStart>
     * @param digits Vector of digits(double)
     * @param pedStart Start of pedestal
     * @param pedLgt Length of pedestal
     * @return Pedestal value
     */
    double getPedestal(const std::vector<float> &digits, int pedStart, int pedLgt) const;

    /**
     * Get position of maximal adder (sum of <filterLength> samples)
     * in whole frame and in signal window (<signalStart>-><signalStart>+<signalLength>
     * Return position and value of maximal adder in frame
     * @param digits Vector if digits(double)
     * @param filterLength
     * @param signalStart Start of signal window
     * @param signalLength Length of signal window
     * @param
     * return Position of max adder
     */
    int getMaxAdder(const std::vector<float> &digits, int filterLength, int signalStart,
                    int signalLength, int &tMax, int &tMaxFrame, int &adderFrame) const;

    int getMaxSample(const std::vector<float> &digits, int signalStart, int signalLength
                    , int &tMax, int &tMaxFrame, int &sampleFrame) const;

    double calculatePeak(const std::vector<float> &digits, int peakPos, double ped,
                         double &position) const;

    /**
     * Calculate energy using flat filter
     * Sum of <filterLength> samples starting at <filterStart>
     * Requires pedestal value and returns position of peak
     * @param digits Vector of didits(double>
     * @param filterStart Start position for filter
     * @param filterLength Number of samples
     * @param ped Pedestal value
     * @param position Storage place for position of peak
     * @return Energy
     */
    double calculateFlatFilter(const std::vector<float> &digits, int filterStart
                              , int filterLength, double ped, double &position) const;

    /**
     * Calculate the time of signal in window of <signalLength> starting at <signalStart>
     * Result is sum of time for every sample weighted with amplitude
     * and divided by sum of times
     * @param digits Vector of digits(double)
     * @param signalStart Start of signal
     * @param signalLength Length of signal in samples
     * @param ped Pedestal value
     * @return Time of signal
     */
    double getTime(const std::vector<float> &digits, int signalStart
                  , int signalLength, double ped) const;

    // parameters
    Gaudi::Property<int> m_pedStart{this, "PedStart", 0, "Start of pedestal window"};
    Gaudi::Property<int> m_pedLength{this, "PedLength", 1, "Length of pedestal window"};
    Gaudi::Property<int> m_pedOffset{this, "PedOffset", 0, "If > 0 ped can be taken after signal as well"};
    Gaudi::Property<int> m_signalStart{this, "SignalStart", 1, "Start of signal window"};
    Gaudi::Property<int> m_signalLength{this, "SignalLength", 8, "Length of signal window"};
    Gaudi::Property<int> m_filterLength{this, "FilterLength", 5, "Filter length"};
    Gaudi::Property<int> m_frameLength{this, "FrameLength", 9, "Frame length (number of samples)"};

    // Cut for signalInFrame, lo/hi gain
    Gaudi::Property<double> m_deltaCutLo{this, "DeltaCutLo", 4.5, "Cut for signalInFrame, low gain"};
    Gaudi::Property<double> m_deltaCutHi{this, "DeltaCutHi", 8.5, "Cut for signalInFrame, high gain"};
    // RMS cut for signalInFram, lo/hi gain
    Gaudi::Property<double> m_rmsCutLo{this, "RMSCutLo", 1.0, "RMS cut for signalInFrame, low gain"};
    Gaudi::Property<double> m_rmsCutHi{this, "RMSCutHi", 2.5, "RMS cut for signalInFrame, high gain"};

};

#endif
