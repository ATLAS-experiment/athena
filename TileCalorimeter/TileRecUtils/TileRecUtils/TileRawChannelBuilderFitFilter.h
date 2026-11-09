/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TILERECUTILS_TILERAWCHANNELBUILDERFITFILTER_H
#define TILERECUTILS_TILERAWCHANNELBUILDERFITFILTER_H

/********************************************************************
 *
 * NAME:     TileRawChannelBuilderFitFilter.h 
 * PACKAGE:  offline/TileCalorimeter/TileRecUtils
 *
 * AUTHOR :  K. Gellerstedt, Algorithm by Richard John Teuscher
 * CREATED:  Dec 03 2003
 *
 * PURPOSE:  Build TileRawChannels from digits using fitting
 *
 *  Input: TileDigitsContainer
 *  Output: TileRawChannelContainer
 *  Parameters: TileRawChannelCont - Name of output container in SG
 *    FrameLength
 *    MaxIterate
 *    RMSChannelNoise
 ********************************************************************/

// Tile includes
#include "TileRecUtils/TileRawChannelBuilder.h"
#include "TileConditions/TilePulseShapes.h"
#include "TileConditions/TileCondToolNoiseSample.h"


class TileRawChannelBuilderFitFilter: public TileRawChannelBuilder {
  public:

    // constructor
    TileRawChannelBuilderFitFilter(const std::string& type, const std::string& name,
        const IInterface *parent);
    // destructor
    ~TileRawChannelBuilderFitFilter();

    // virtual methods
    virtual StatusCode initialize() override;
    virtual StatusCode finalize() override;

    // Inherited from TileRawChannelBuilder
    virtual TileRawChannel * rawChannel(const TileDigits* digits, const EventContext& ctx) override;

    /**
     * AlgTool InterfaceID
     */
    static const InterfaceID& interfaceID();

  private:

    Gaudi::Property<bool> m_bestPhase{this, "BestPhase", false, "Use best phase from DB"};

    void pulseFit(const TileDigits *digit, double &amplitude, double &time, double &pedestal, double &chi2, const EventContext &ctx);

    // generic functions for pulse interpolation
    double pulse(double x, const std::vector<double> * xvec, const std::vector<double>* yvec, bool zeroOutside = false) const;

    double scaledPulse(double x, const std::vector<double> * xvec, const std::vector<double> * yvec) const {
      return m_fnParameters[1] + m_fnParameters[2] * pulse(x, xvec, yvec, false);
    }

    double derivative(double x, const std::vector<double> * xvec, const std::vector<double> * yvec) const {
      return pulse(x, xvec, yvec, true);
    }

    std::vector<double> m_dummy; // dummy vector which can be passed to the functions above

    // Pulse shape function variables
    double m_t0Fit;
    // Parameters for pulse shape functions
    // [0] - phase, [1] - pedestal, [2] - amplitude
    double m_fnParameters[3]{};
    int m_iPeak0;
    //double m_phasefrac;
    double m_minTime;
    double m_maxTime;
    double m_minTau;
    double m_maxTau;

    // Precalculated values for 1 iteration
    std::vector<double> m_gPhysLo;
    std::vector<double> m_dgPhysLo;
    std::vector<double> m_gPhysHi;
    std::vector<double> m_dgPhysHi;

    // Parameters
    Gaudi::Property<int> m_frameLength{this, "FrameLength", 9, "Frame length (number of samples)"};
    Gaudi::Property<int> m_channelNoiseRMS{this, "RMSChannelNoise", 3, "Which channel noise RMS to use"};
    Gaudi::Property<int> m_maxIterate{this, "MaxIterate", 9, "Maximum number of fit iterations"};
    Gaudi::Property<int> m_extraSamplesLeft{this, "ExtraSamplesLeft", 0, "Increase window on left side"};
    Gaudi::Property<int> m_extraSamplesRight{this, "ExtraSamplesRight", 0, "Increase window on right side"};
    // defines the saturated sample
    Gaudi::Property<double> m_saturatedSample{this, "SaturatedSample", -1.0, "Defines the saturated sample"};
    // which error in terms of RMS is assigned to the saturated sample
    Gaudi::Property<double> m_saturatedSampleError{this, "SaturatedSampleError", 6.0,
        "Error in terms of RMS assigned to the saturated sample"};
    // which error in terms of RMS is assigned to the zero sample (== 0)
    Gaudi::Property<double> m_zeroSampleError{this, "ZeroSampleError", 100.0,
        "Error in terms of RMS assigned to the zero sample (== 0)"};
    // for pedestal-like events only 2-parametric fit is applied
    Gaudi::Property<double> m_noiseThresholdRMS{this, "NoiseThresholdRMS", 3.0,
        "For pedestal-like events only 2-parametric fit is applied"};
    // max.time for fitting the pulse shapes. Samples with t>t0+m_MaxTimeFromPeak are not fitted.
    Gaudi::Property<double> m_maxTimeFromPeak{this, "MaxTimeFromPeak", 250.0,
        "Max time for fitting the pulse shapes. Samples with t>t0+MaxTimeFromPeak are not fitted"};

    // default low gain noise from TileInfo used in simulation
    Gaudi::Property<double> m_noiseLow{this, "NoiseLowGain", 0.6, "Default low gain noise"};
    // default high gain noise from TileInfo used in simulation
    Gaudi::Property<double> m_noiseHigh{this, "NoiseHighGain", 1.5, "Default high gain noise"};

    // Pulse shapes
    const TilePulseShapesStruct* m_pulseShapes;

    Gaudi::Property<bool> m_disableNegativeAmp{this, "DisableNegativeAmp", false, "Disable negative amplitude"};
    // if >=0 - pulse shape for Demo is stored in non-default structures
    Gaudi::Property<int> m_specialDemoShape{this, "SpecialDemoShape", -1,
        "If >=0 - pulse shape for Demo is stored in non-default structures"};

    ToolHandle<TileCondToolNoiseSample> m_tileToolNoiseSample{this,
        "TileCondToolNoiseSample", "TileCondToolNoiseSample", "Tile sample noise tool"};

};

#define DTIME 25.0  // 25 ns distance between subsequent samples
#endif
