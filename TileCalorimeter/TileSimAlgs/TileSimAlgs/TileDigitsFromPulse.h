/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

//****************************************************************************
// Filename : TileDigitsFromPulse.h
// Author   : Simon Molander <simon.molander@cern.ch>
// Created  : Feb 2013
//
// DESCRIPTION
// 
// Create TileDigits from pulse simulator
//
// Properties (JobOption Parameters):
//
//    OutputDigitsContainer        string   Name of container with TileDigits to write
//    ImperfectionMean             double   Mean value of pulse shape broadening
//    ImperfectionRMS              double   RMS of pulse shape broadening
//    InTimeAmp                    double   Amplitude of in-time pulse
//    OutOfTimeAmp                 double   Amplitude of out-of-time pulse
//    InTimeOffset                 double   In-time pulse offset from nominal time
//    OutOfTimeOffset              double   Out-of-time pulse offset from nominal time
//    UseGaussNoise                bool     Set to TRUE in order to create noise from double gaussian
//    GaussNoiseAmpOne             double   Amplitude of first gaussian of double gaussian noise.
//    GaussNoiseSigmaOne           double   Standard deviation of first gaussian of double gaussian noise.
//    GaussNoiseAmpTwo             double   Amplitude of second gaussian of double gaussian noise.
//    GaussNoiseSigmaTwo           double   Standard deviation of second gaussian of double gaussian noise.    
//    UseInTimeAmpDist             bool     Set to TRUE in order to use a distribution for the in-time amplitude instead of a const.
//    UseOutOfTimeAmpDist          bool     Set to TRUE in order to use a distribution for the out-of-time amplitude instead of a const.
//    InTimeAmpDistFileName        string   Filename of file to use for amplitude distribution of in-time pulses
//    OutOfTimeAmpDistFileName     string   Filename of file to use for amplitude distribution of out-of-time pulses
//    PileUpFraction               float    Probability that an out-of-time component will be added
//    GaussianC2CPhaseVariation    float    RMS for the in-time pulse offset (channel-to-channel phase variation)
//    ChannelSpecificPedestal      bool     Set to TRUE in order to use a channel specific value for the pedestal
//    ChannelSpecificNoise         bool     Set to TRUE in order to add channel specific noise
//    OutOfTimeOffsetHistogramFile string   Filename of file containing histogram of pile-up timing distribution
//    OutOfTimeOffsetHistogramName string   name of the histogram to use for pile-up timing distribution
//    AmpDistLowerLimit            int      Set all bins lower than this to zero. Default = 135.
//    InTimeAmpDistHistogramName   string   Name of the histogram to use for in-time amplitude distribution
//    OutOfTimeAmpDistHistogramName   string   Name of the histogram to use for out-of-time amplitude distribution
//
// BUGS:
//  
// History:
//  
//  
//****************************************************************************

#ifndef TILESIMALGS_TILEDIGITSFROMPULSE_H
#define TILESIMALGS_TILEDIGITSFROMPULSE_H

// Tile includes
#include "TileEvent/TileDigitsContainer.h"
#include "TileEvent/TileRawChannelContainer.h"
#include "TileConditions/TileCondToolNoiseSample.h"

// Athena includes
#include "AthenaBaseComps/AthAlgorithm.h"
#include "StoreGate/WriteHandleKey.h"

// Gaudi includes
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/ToolHandle.h"

#include "CLHEP/Random/RandomEngine.h"

//C++ STL includes
#include <vector>
#include <string>
#include "Gaudi/Property.h"

//Simulation includes
class TileSampleGenerator;
class TileSampleBuffer;
class TilePulseShape;

class TileHWID;
class TileInfo;
class IAthRNGSvc;

class TH1F;
class TFile;
class TRandom3;

/** 
@class TileDigitsFromPulse
@brief This algorithm creates TileDigits from pulses provided by pulse simulator
 */
class TileDigitsFromPulse : public AthAlgorithm {
public:
    // Constructor
    TileDigitsFromPulse(const std::string& name, ISvcLocator* pSvcLocator);

    //Destructor 
    virtual ~TileDigitsFromPulse();                         
    
    //Gaudi Hooks
    virtual StatusCode initialize() override; //!< initialize method
    virtual StatusCode execute(const EventContext& ctx) override;    //!< execute method
    virtual StatusCode finalize() override;   //!< finalize method

private:
    std::string m_outputContainer; //!< Name of the output TileDigitsContainer
    
    const TileHWID* m_tileHWID{};
    
    // TileInfo
    Gaudi::Property<std::string> m_infoName{this, "TileInfoName", "TileInfo"};
    const TileInfo* m_tileInfo{};
    int m_i_ADCmax{};
    
    TileRawChannelUnit::UNIT m_rChUnit{TileRawChannelUnit::ADCcounts}; //!< Units used for the TileRawChannels (ADC, pCb, etc.)(see TileInfo.h)
    TileFragHash::TYPE m_rChType{TileFragHash::Default}; //!< Type of TileRawChannels (Digitizar, OF1, OF2, Fit, etc.)(see TileFragHash.h)
    
    ToolHandle<TileCondToolNoiseSample> m_tileToolNoiseSample{this,
        "TileCondToolNoiseSample", "TileCondToolNoiseSample", "Tile sample noise tool"};

    SG::WriteHandleKey<TileDigitsContainer> m_digitsContainerKey{this,
        "TileDigitsContainer", "TileDigitsCnt", "Output Tile digits container key"};

    SG::WriteHandleKey<TileRawChannelContainer> m_rawChannelContainerKey{this,
        "TileRawChannelContainer", "TrueAmp", "Output Tile raw channel container key"};
    
    ServiceHandle <IAthRNGSvc> m_rndmSvc{this, "RndmSvc", "AthRNGSvc", ""};  //!< Random number service to use
    /// Random Stream Name
    Gaudi::Property<std::string> m_randomStreamName{this, "RandomStreamName", "Tile_DigitsMaker", ""};

    
    //Parameters
    Gaudi::Property<double> m_imperfectionMean{this, "ImperfectionMean", 1.01}; //!< Mean value of pulse shape broadening
    Gaudi::Property<double> m_imperfectionRms{this, "ImperfectionRms", 0.02}; //!< RMS of pulse shape broadening
    Gaudi::Property<double> m_inTimeAmp{this, "InTimeAmp", 300.}; //!< Amplitude of in-time pulse
    Gaudi::Property<double> m_ootAmp{this, "OutOfTimeAmp", 150.}; //!< Amplitude of out-of-time pulse
    Gaudi::Property<double> m_itOffset{this, "InTimeOffset", 0.}; //!< In-time pulse offset from nominal time
    Gaudi::Property<double> m_ootOffset{this, "OutOfTimeOffset", 50.}; //!< Out-of-time pulse offset from nominal time
    Gaudi::Property<double> m_ped_HG{this, "PedestalValueHG", 100}; //!< Pedestal value for HG if specific channel pedestal is not used
    Gaudi::Property<double> m_ped_LG{this, "PedestalValueLG", 100}; //!< Pedestal value for LG if specific channel pedestal is not used
    Gaudi::Property<bool> m_gaussNoise{this, "UseGaussNoise", false}; //!< Set to TRUE in order to create noise from double gaussian
    Gaudi::Property<double> m_GNAmpOne{this, "GaussNoiseAmpOne", 1 / 1.039}; //!< Amplitude of first gaussian of double gaussian noise.
    Gaudi::Property<double> m_GNSigmaOne{this, "GaussNoiseSigmaOne", 1.6}; //!< Standard deviation of first gaussian of double gaussian noise.
    Gaudi::Property<double> m_GNAmpTwo{this, "GaussNoiseAmpTwo", 0.039}; //!< Amplitude of second gaussian of double gaussian noise.
    Gaudi::Property<double> m_GNSigmaTwo{this, "GaussNoiseSigmaTwo", 3.6}; //!< Standard deviation of second gaussian of double gaussian noise.    
    Gaudi::Property<bool> m_useItADist{this, "UseInTimeAmpDist", false}; //!< Set to TRUE in order to use a distribution for the in-time amplitude instead of a constant value
    Gaudi::Property<double> m_itAPulseProb{this, "InTimeAmpPulseProb", 0}; //!< Probability to add an in-time pulse
    Gaudi::Property<bool> m_useOotADist{this, "UseOutOfTimeAmpDist", false}; //!< Set to TRUE in order to use a distribution for the out-of-time amplitude instead of a constant value
    Gaudi::Property<float> m_pileUpFraction{this, "PileUpFraction", 1}; //!< Probability that an out-of-time component will be added
    Gaudi::Property<float> m_gausC2C{this, "GaussianC2CPhaseVariation", 0}; //!< RMS for the in-time pulse offset (channel-to-channel phase variation)
    Gaudi::Property<std::string> m_itADistFileName{this, "InTimeAmpDistFileName", ""}; //!< Filename of file to use for amplitude distribution of in-time pulses
    Gaudi::Property<std::string> m_ootADistFileName{this, "OutOfTimeAmpDistFileName", ""}; //!< Filename of file to use for amplitude distribution of out-of-time pulses
    Gaudi::Property<bool> m_chanPed{this, "ChannelSpecificPedestal", false}; //!< Use channel specific pedestal value if true. Otherwise, same for all channels.
    Gaudi::Property<bool> m_chanNoise{this, "ChannelSpecificNoise", false}; //!< Add channel specific noise
    Gaudi::Property<std::string> m_ootOffsetFileName{this, "OutOfTimeOffsetHistogramFile", ""}; //!< File name for offset timing distribution histogram
    Gaudi::Property<std::string> m_ootOffsetHistName{this, "OutOfTimeOffsetHistogramName", ""}; //!< Name of the histogram for timing offset distribution
    Gaudi::Property<int> m_AmpDistLowLim{this, "AmpDistLowerLimit", 135}; //!< Set all bins to the left of this bin to 0 in the amplitude distribution histograms
    Gaudi::Property<std::string> m_itADistHistName{this, "InTimeAmpDistHistogramName", "h_Eopt_hi"}; //!< Name of histogram for in-time amplitude distribution
    Gaudi::Property<std::string> m_ootADistHistName{this, "OutOfTimeAmpDistHistogramName", "h_Eopt_hi"}; //!< Name of  histogram for out-of-time amplitude distribution
    
    Gaudi::Property<bool> m_simQIE{this, "SimulateQIE", false}; //!<Raw PMT pulses are generated if the option is set to true. The option is intended to simulate the QIE FEB.
    Gaudi::Property<bool> m_simPulseChain{this, "SimulatePulseChain", false}; //!< Simulate continous output of readout for HL-LHC paradigm

    Gaudi::Property<int> m_seed{this, "RandomSeed", 4357};
    Gaudi::Property<int> m_BunchSpacing{this, "BunchSpacing", 25}; //!< Time between pulses in ms 25, 50 or 75
    Gaudi::Property<int> m_nSamples{this, "NSamples", 7};  //!< number of read out samples
    Gaudi::Property<int> m_nPul{this, "NPulses", 21};  //!< number of pileup pulses 
    int m_nPul_eff{};  //Used for symetrization of PU in computation
    std::vector<std::vector<std::vector<std::vector<float>>>> m_PUAmp; // Used to store PU amplitudes
    Gaudi::Property<bool> m_PhaseII{this, "TilePhaseII", false}; //Use parameters of TilePhaseII if the option is set to true
    Gaudi::Property<bool> m_bigain{this, "Bigain", false}; //If true, save the two gains in the ntuples

    Gaudi::Property<bool> m_simPUwPoisson{this, "SimulatePileUpWithPoiss", false}; //If true, simulate PU overlapping amplitudes n times, with n being generated with a poissonian distribution 
    Gaudi::Property<int> m_avgMuForPU{this, "AvgMuForPileUpSimulation", 40}; //Mean of the poissonian distribution to generate the number of interactions per bunch crossing for PU simulation
    Gaudi::Property<std::string> m_pileupAmpDistFileName{this, "PileUpAmpDistFileName", ""}; //!< Filename for PU amplitude distribution histograms

    //Members for simulator
    TilePulseShape*      m_ps[2]; //!< Class for defining pulse. One element for lo gain and one for hi.
    TileSampleBuffer*    m_buf{}; //!< Buffer class to hold generated pulses
    TileSampleGenerator* m_tsg{}; //!< Pulse generating class
    
    //Histograms for distribution
    TH1F* m_ootDist; //!< Histogram to hold the distribution of out-of-time amplitudes.
    TH1F* m_itDist; //!< Histogram to hold the distribution of in-time amplitudes.
    TFile* m_itFile; //!< File that holds the distribution of in-time amplitudes.
    TFile* m_ootFile; //!< File that holds the distribution of out-of-time amplitudes.
    TH1F* m_ootOffsetDist; //!< Histogram to hold the distribution of out-of-time timing offsets
    TFile* m_ootOffsetFile; //!< File that holds the distribution of out-of-time timing offsets
    bool m_useOffsetHisto{false}; //!< Internally used to keep track of wether a histogram has been opened or not
    TFile* m_pileup_AmpDistFile; //!< File containing amplitude histograms for PU pulses 
    std::vector<std::vector<TH1F*>> m_pileup_AmpDists; //!< Matrix of PU amplitude distribution histograms (PU per partition and channel)

    //Internal methods
    bool makeDist(TFile*& file, TH1F*& hist, const std::string& fileName, const std::string& histName="h_Eopt_hi"); //!< Method to read distribution from file
    bool makeDist(TFile*& file, std::vector<std::vector<TH1F*>>& hists, const std::string& fileName);

    void addPileUp(double &n_inTimeAmp, double ootAmp, int gain, int ros, int drawer, int channel); //!< Fill vector with pile-up amplitudes
    void addPileUpSample(int gain, int ros, int drawer, int channel); //!< Fill only a BC with pile-up amplitude

    float m_sample_tru = 0;

    std::unique_ptr<TRandom3> m_random;
};

#endif // TILESIMALGS_TILEDIGITSFROMPULSE_H

