/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ZDCANALYSIS_RPDDATAANALYZER_H
#define ZDCANALYSIS_RPDDATAANALYZER_H

#include <string>
#include <vector>
#include <functional>
#include <bitset>

#include "ZdcAnalysis/ZDCMsg.h"
#include "ZdcUtils/RPDUtils.h"


namespace ZDC {

struct RPDConfig {
  unsigned int nSamples;
  unsigned int nBaselineSamples;
  unsigned int endSignalSample;
  float pulse2ndDerivThresh;
  float postPulseFracThresh;
  unsigned int goodPulseSampleStart;
  unsigned int goodPulseSampleStop;
  float nominalBaseline;
  float pileupBaselineSumThresh;
  float pileupBaselineStdDevThresh;
  unsigned int nNegativesAllowed;
  unsigned int ADCOverflow;
};

class RPDDataAnalyzer {
 public:
  enum {
    ValidBit                        =  0, // analysis and output are valid
    OutOfTimePileupBit              =  1, // OOT detected, pileup subtraction attempted
    OverflowBit                     =  2, // overflow detected => invalid
    PrePulseBit                     =  3, // pulse detected before expected range => invalid
    PostPulseBit                    =  4, // pulse detected after expected range => invalid
    NoPulseBit                      =  5, // no pulse detected => invalid
    BadAvgBaselineSubtrBit          =  6, // subtraction of avg. of baseline samples yielded too many negatives => invalid
    InsufficientPileupFitPointsBit  =  7, // baseline samples indicate pileup, but there are not enough points to perform fit -> nominal baseline used without pileup subtraction
    PileupStretchedExpFitFailBit    =  8, // fit to stretched exponential failed -> fallback to exponential fit
    PileupStretchedExpGrowthBit     =  9, // fit to stretched exponential does not decay -> fallback to exponential fit
    PileupBadStretchedExpSubtrBit   = 10, // subtraction of stretched exponential fit yielded too many negatives -> fallback to exponential fit
    PileupExpFitFailBit             = 11, // fit to exponential failed => invalid IF stretched exponential fit is also bad
    PileupExpGrowthBit              = 12, // fit to exponential does not decay => invalid IF stretched exponential fit is also bad
    PileupBadExpSubtrBit            = 13, // subtraction of stretched exponential yielded too many negatives => invalid IF stretched exponential fit is also bad
    PileupStretchedExpPulseLikeBit  = 14, // fit to stretched exponential probably looks more like a pulse than pileup
    N_STATUS_BITS
  };
  enum class PileupFitFuncType {
    None, Exp, SecondOrderStretchedExp
  };

  RPDDataAnalyzer(ZDCMsg::MessageFunctionPtr messageFunc_p, std::string tag, RPDConfig const& config, std::vector<float> const& calibFactors);
  virtual ~RPDDataAnalyzer() = default;
  // this class is not intended to be copied or moved
  RPDDataAnalyzer(RPDDataAnalyzer const&) = delete;
  RPDDataAnalyzer& operator=(RPDDataAnalyzer const&) = delete;
  RPDDataAnalyzer(RPDDataAnalyzer &&) = delete;
  RPDDataAnalyzer& operator=(RPDDataAnalyzer &&) = delete;

  void loadChannelData(unsigned int channel, const std::vector<uint16_t>& FadcData);
  void analyzeData();

  unsigned int getChMaxSample(unsigned int channel) const;
  float getChSumAdc(unsigned int channel) const;
  float getChSumAdcCalib(unsigned int channel) const;
  float getChMaxAdc(unsigned int channel) const;
  float getChMaxAdcCalib(unsigned int channel) const;
  float getChPileupFrac(unsigned int channel) const;
  float getChBaseline(unsigned int channel) const;
  const std::vector<float>& getChPileupExpFitParams(unsigned int channel) const;
  const std::vector<float>& getChPileupStretchedExpFitParams(unsigned int channel) const;
  const std::vector<float>& getChPileupExpFitParamErrs(unsigned int channel) const;
  const std::vector<float>& getChPileupStretchedExpFitParamErrs(unsigned int channel) const;
  float getChPileupExpFitMSE(unsigned int channel) const;
  float getChPileupStretchedExpFitMSE(unsigned int channel) const;

  unsigned int getChStatus(unsigned int channel) const;
  unsigned int getSideStatus() const;

  void reset();

 private:
  bool checkOverflow(unsigned int channel);
  bool checkPulses(unsigned int channel);
  unsigned int countSignalRangeNegatives(std::vector<float> const& values) const;
  bool doBaselinePileupSubtraction(unsigned int channel);
  void calculateMaxSampleMaxAdc(unsigned int channel);
  void calculateSumAdc(unsigned int channel);

  void setSideStatusBits();

  bool doPileupExpFit(unsigned int channel, std::vector<std::pair<unsigned int, float>> const& pileupFitPoints);
  bool doPileupStretchedExpFit(unsigned int channel, std::vector<std::pair<unsigned int, float>> const& pileupFitPoints);
  float calculateBaselineSamplesMSE(unsigned int channel, std::function<float(unsigned int)> const& fit) const;

  ZDCMsg::MessageFunctionPtr m_msgFunc_p;
  std::string m_tag;

  static unsigned int constexpr s_nChannels = RPDUtils::nChannels; // for convenience

  unsigned int m_nChannelsLoaded = 0;
  unsigned int m_nSamples;
  unsigned int m_nBaselineSamples; /** Number of baseline samples; the sample equal to this number is the start of signal region */
  unsigned int m_endSignalSample; /** Samples before (not including) this sample are the signal region; nSamples goes to end of window */
  float m_pulse2ndDerivThresh; /** Second differences less than or equal to this number indicate a pulse */
  float m_postPulseFracThresh; /** If there is a good pulse and post-pulse and size of post-pulse as a fraction of good pulse is less than or equal to this number, ignore post-pulse */
  unsigned int m_goodPulseSampleStart; /** Pulses before this sample are considered pre-pulses */
  unsigned int m_goodPulseSampleStop; /** Pulses after this sample are considered post-pulses */
  float m_nominalBaseline; /** The global nominal baseline; used when pileup is detected */
  float m_pileupBaselineSumThresh; /** Baseline sums less than this number indicate there is not pileup */
  float m_pileupBaselineStdDevThresh; /** Baseline standard deviations less than this number indicate there is not pileup */
  unsigned int m_nNegativesAllowed; /** Maximum number of negative ADC values after baseline and pileup subtraction allowed in signal range */
  unsigned int m_AdcOverflow; /** ADC values greater than or equal to this number are considered overflow */
  std::array<float, s_nChannels> m_outputCalibFactors {}; /** multiplicative calibration factors to apply to output, e.g., max and sum ADC; per channel */

  std::array<std::vector<uint16_t>, s_nChannels> m_chFADCData; /** raw RPD data; index channel then sample */
  std::array<std::vector<float>, s_nChannels> m_chCorrectedFadcData; /** RPD data with baseline and pileup subtracted; index channel then sample */
  std::array<unsigned int, s_nChannels> m_chMaxSample {}; /** sample of max of RPD data in signal range after pileup subtraction; per channel */
  std::array<float, s_nChannels> m_chSumAdc {}; /** sum of RPD data in signal range after baseline and pileup subtraction; per channel */
  std::array<float, s_nChannels> m_chSumAdcCalib {}; /** sum of RPD data in signal range after baseline and pileup subtraction, with output calibration factors applied; per channel */
  std::array<float, s_nChannels> m_chMaxAdc {}; /** max of RPD data in signal range after baseline and pileup subtraction; per channel */
  std::array<float, s_nChannels> m_chMaxAdcCalib {}; /** max of RPD data in signal range after baseline and pileup subtraction, with output calibration factors applied; per channel */
  std::array<float, s_nChannels> m_chPileupFrac {}; /** OOT pileup sum as a fraction of non-pileup sum in entire window (0 if no OOT pileup, -1 if sum ADC <= 0); per channel */
  std::array<float, s_nChannels> m_chBaseline {}; /** baseline used in baseline subtraction; per channel */
  std::array<std::vector<float>, s_nChannels> m_chPileupExpFitParams; /** parameters for pileup exponential fit (if pileup was detected and fit did not fail): exp( [0] + [1]*sample ); per channel */
  std::array<std::vector<float>, s_nChannels> m_chPileupStretchedExpFitParams; /** parameters for pileup stretched exponential fit (if pileup was detected and fit did not fail): exp( [0] + [1]*(sample + 4)**(0.5) + [2]*(sample + 4)**(-0.5) ); per channel */
  std::array<std::vector<float>, s_nChannels> m_chPileupExpFitParamErrs; /** parameter errors for pileup exponential fit (if pileup was detected and fit did not fail); per channel */
  std::array<std::vector<float>, s_nChannels> m_chPileupStretchedExpFitParamErrs; /** parameter errors for pileup stretched exponential fit (if pileup was detected and fit did not fail); per channel */
  std::array<PileupFitFuncType, s_nChannels> m_chPileupFuncType {}; /** enum indicating type of pileup fit function; per channel */
  std::array<std::function<float(unsigned int)>, s_nChannels> m_chExpPileupFuncs; /** pileup exponential fit function (if pileup was detected and fit did not fail); per channel */
  std::array<std::function<float(unsigned int)>, s_nChannels> m_ch2ndOrderStretchedExpPileupFuncs; /** pileup stretched exponential fit function (if pileup was detected and fit did not fail); per channel */
  std::array<float, s_nChannels> m_chExpPileupMSE {}; /** mean squared error of pileup exponential fit in baseline samples (if pileup was detected and fit did not fail); per channel */
  std::array<float, s_nChannels> m_ch2ndOrderStretchedExpPileupMSE {}; /** mean squared error of pileup stretched exponential fit in baseline samples (if pileup was detected and fit did not fail); per channel */
  std::array<std::bitset<N_STATUS_BITS>, s_nChannels> m_chStatus; /** status bits per channel */
  std::bitset<N_STATUS_BITS> m_sideStatus; /** status bits for side */

  /**
   * in the case of pileup, the number of points (above baseline) in baseline samples required to perform fit.
   * this number must be at least the number of parameters in pileup fits, else inversion of Gram matrix in TLinearFitter
   * will fail and generate ROOT error that propagates to Athena.
   * if insufficient points, set InsufficientPileupFitPointsBit and abort pileup subtraction.
   */
  static unsigned int constexpr s_minPileupFitPoints = 3;
};

} // namespace ZDC

#endif
