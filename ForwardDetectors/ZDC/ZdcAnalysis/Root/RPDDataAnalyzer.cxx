/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "ZdcAnalysis/RPDDataAnalyzer.h"

#include "TLinearFitter.h"
#include "TMath.h"
#include <limits>

namespace ZDC {

float nullPileupFunc(unsigned int /* sample */) {
  return 0;
}

void helpResetFuncs(std::span<std::function<float(unsigned int)>> v) {
  std::function<float(unsigned int)> f (nullPileupFunc);
  std::fill(v.begin(), v.end(), f);
}

RPDDataAnalyzer::RPDDataAnalyzer(
  ZDCMsg::MessageFunctionPtr messageFunc_p,
  std::string tag,
  RPDConfig const& config,
  std::vector<float> const& calibFactors
) : m_msgFunc_p(std::move(messageFunc_p)),
    m_tag(std::move(tag)),
    m_nSamples(config.nSamples),
    m_nBaselineSamples(config.nBaselineSamples),
    m_endSignalSample(config.endSignalSample),
    m_pulse2ndDerivThresh(config.pulse2ndDerivThresh),
    m_postPulseFracThresh(config.postPulseFracThresh),
    m_goodPulseSampleStart(config.goodPulseSampleStart),
    m_goodPulseSampleStop(config.goodPulseSampleStop),
    m_nominalBaseline(config.nominalBaseline),
    m_pileupBaselineSumThresh(config.pileupBaselineSumThresh),
    m_pileupBaselineStdDevThresh(config.pileupBaselineStdDevThresh),
    m_nNegativesAllowed(config.nNegativesAllowed),
    m_AdcOverflow(config.ADCOverflow)
{
  if (m_endSignalSample == 0) m_endSignalSample = m_nSamples; // sentinel value 0 -> go to end of waveform
  if (m_outputCalibFactors.size() != s_nChannels) {
    (*m_msgFunc_p)(ZDCMsg::Fatal,
      "RPDDataAnalyzer::RPDDataAnalyzer: received incorrect number of channels in calibration factors ("
        + std::to_string(m_outputCalibFactors.size()) + " != " + std::to_string(s_nChannels) + ")"
    );
  }
  std::copy(calibFactors.begin(), calibFactors.end(), m_outputCalibFactors.data());
  m_chFADCData.fill(std::vector<uint16_t>(m_nSamples, 0));
  m_chCorrectedFadcData.fill(std::vector<float>(m_nSamples, 0));
  m_chPileupExpFitParams.fill(std::vector<float>(2, 0));
  m_chPileupStretchedExpFitParams.fill(std::vector<float>(3, 0));
  m_chPileupExpFitParamErrs.fill(std::vector<float>(2, 0));
  m_chPileupStretchedExpFitParamErrs.fill(std::vector<float>(3, 0));
  m_chPileupFuncType.fill(PileupFitFuncType::None);
  m_chExpPileupFuncs.fill(nullPileupFunc);
  m_ch2ndOrderStretchedExpPileupFuncs.fill(nullPileupFunc);

  m_sideStatus.reset();
  m_sideStatus.set(ValidBit, true);
  for (auto& status : m_chStatus) {
    status.reset();
    status.set(ValidBit, true);
  }
}

/**
 * Load a single channel's FADC data into member variable.
*/
void RPDDataAnalyzer::loadChannelData(unsigned int channel, const std::vector<uint16_t>& FadcData)
{
  if (FadcData.size() != m_nSamples) {
    (*m_msgFunc_p)(ZDCMsg::Fatal,
      "RPDDataAnalyzer::loadChannelData: received incorrect number of samples "
      "in FADC data (" + std::to_string(FadcData.size()) + ", expected " + std::to_string(m_nSamples) + ")"
    );
  }
  m_chFADCData.at(channel) = FadcData;
  m_nChannelsLoaded++;
}

/**
 * Reset all member variables to default values.
*/
void RPDDataAnalyzer::reset()
{
  // reset status bits; valid bit is true by default
  m_sideStatus.reset();
  m_sideStatus.set(ValidBit, true);
  for (auto& status : m_chStatus) {
    status.reset();
    status.set(ValidBit, true);
  }

  RPDUtils::helpZero(m_chFADCData);
  RPDUtils::helpZero(m_chCorrectedFadcData);
  RPDUtils::helpZero(m_chPileupExpFitParams);
  RPDUtils::helpZero(m_chPileupStretchedExpFitParams);
  RPDUtils::helpZero(m_chPileupExpFitParamErrs);
  RPDUtils::helpZero(m_chPileupStretchedExpFitParamErrs);
  m_chPileupFuncType.fill(PileupFitFuncType::None);

  helpResetFuncs(m_chExpPileupFuncs);
  helpResetFuncs(m_ch2ndOrderStretchedExpPileupFuncs);

  RPDUtils::helpZero(m_chExpPileupMSE);
  RPDUtils::helpZero(m_ch2ndOrderStretchedExpPileupMSE);

  m_nChannelsLoaded = 0;
  RPDUtils::helpZero(m_chMaxSample);
  RPDUtils::helpZero(m_chSumAdc);
  RPDUtils::helpZero(m_chSumAdcCalib);
  RPDUtils::helpZero(m_chMaxAdc);
  RPDUtils::helpZero(m_chMaxAdcCalib);
  RPDUtils::helpZero(m_chPileupFrac);
  RPDUtils::helpZero(m_chBaseline);
}

/**
 * Check for overflow and set relevant status bit.
 * Returns true if there was NO overflow (c'est bon).
*/
bool RPDDataAnalyzer::checkOverflow(unsigned int channel)
{
  for (unsigned int sample = 0; sample < m_nSamples; sample++) {
    if (m_chFADCData.at(channel).at(sample) >= m_AdcOverflow) {
      m_chStatus.at(channel).set(OverflowBit, true);
      return false; // overflow - not good
    }
  }
  return true; // all good
}

/**
 * Calculate 2nd difference, identify pulses, and set relevant status bits.
 * Returns true if there is a good pulse detected, false if there was a problem,
 * e.g., no pulse or a pulse outside of the expected range.
*/
bool RPDDataAnalyzer::checkPulses(unsigned int channel) {
  float prePulseSize = 0;
  unsigned int prePulseSample = 0;
  float goodPulseSize = 0;
  unsigned int goodPulseSample = 0;
  float postPulseSize = 0;
  unsigned int postPulseSample = 0;
  for (unsigned int sample = 1; sample < m_nSamples - 1; sample++) {
    float const secondDiff = m_chFADCData.at(channel).at(sample + 1) - 2*m_chFADCData.at(channel).at(sample) + m_chFADCData.at(channel).at(sample - 1);
    if (secondDiff > m_pulse2ndDerivThresh) continue; // no pulse here
    if (sample < m_goodPulseSampleStart && secondDiff < prePulseSize) {
      prePulseSize = secondDiff;
      prePulseSample = sample;
    } else if (sample >= m_goodPulseSampleStart && sample <= m_goodPulseSampleStop && secondDiff < goodPulseSize) {
      goodPulseSize = secondDiff;
      goodPulseSample = sample;
    } else if (sample > m_goodPulseSampleStop && secondDiff < postPulseSize) {
      postPulseSize = secondDiff;
      postPulseSample = sample;
    }
  }

  bool hasPrePulse = prePulseSample;
  bool hasGoodPulse = goodPulseSample;
  bool hasPostPulse = postPulseSample;

  // we need to accomodate side A, which has dips in second derivative after good pulse range
  // if post-pulses are sufficiently small, we ignore them
  // we expect these anomolies at ~ sample 11 or 12
  if (hasGoodPulse && hasPostPulse && postPulseSize/goodPulseSize <= m_postPulseFracThresh) hasPostPulse = false;

  bool hasNoPulse = !hasPrePulse && !hasGoodPulse && !hasPostPulse;

  if (hasPrePulse) m_chStatus.at(channel).set(PrePulseBit, true);
  if (hasPostPulse) m_chStatus.at(channel).set(PostPulseBit, true);
  if (hasNoPulse) m_chStatus.at(channel).set(NoPulseBit, true);

  return !hasPrePulse && !hasPostPulse; // true if there is a only good pulse or if there is no pulse
}

/**
 * Calculate the mean squared error of the fit function in the baseline samples.
*/
float RPDDataAnalyzer::calculateBaselineSamplesMSE(unsigned int channel, std::function<float(unsigned int)> const& fit) const
{
  float MSE = 0;
  for (unsigned int sample = 0; sample < m_nBaselineSamples; sample++) {
    MSE += std::pow(m_chFADCData.at(channel).at(sample) - m_chBaseline.at(channel) - fit(sample), 2);
  }
  MSE /= m_nBaselineSamples;
  return MSE;
}

/**
 * Perform an exponential fit in baseline-subtracted baseline samples and set relevant status bits.
 * Returns true if the fit and subtraction are good, false if there was a problem.
 */
bool RPDDataAnalyzer::doPileupExpFit(unsigned int channel, std::vector<std::pair<unsigned int, float>> const& pileupFitPoints)
{
  auto pFitter = std::make_unique<TLinearFitter>(1, "1 ++ x");
  double x {};
  for (auto const& [sample, y] : pileupFitPoints) {
    x = sample;
    pFitter->AddPoint(&x, std::log(y));
  }
  if (pFitter->Eval()) {
    (*m_msgFunc_p)(ZDCMsg::Warn, "RPDDataAnalyzer::doPileupExpFit: there was an error while evaluating TLinearFitter!");
    m_chStatus.at(channel).set(PileupExpFitFailBit, true);
    return false;
  }
  m_chPileupExpFitParams.at(channel) = {static_cast<float>(pFitter->GetParameter(0)), static_cast<float>(pFitter->GetParameter(1))};
  m_chPileupExpFitParamErrs.at(channel) = {static_cast<float>(pFitter->GetParError(0)), static_cast<float>(pFitter->GetParError(1))};
  m_chExpPileupFuncs.at(channel) = [intercept = pFitter->GetParameter(0), slope = pFitter->GetParameter(1)](unsigned int sample) { return std::exp(intercept + slope*sample); };
  m_chExpPileupMSE.at(channel) = calculateBaselineSamplesMSE(channel, m_chExpPileupFuncs.at(channel));

  // check for exponential growth in parameters - we definitely don't want that for a function that describes pileup
  if (pFitter->GetParameter(1) >= 0) {
    (*m_msgFunc_p)(ZDCMsg::Debug, "RPDDataAnalyzer::doPileupExpFit: p1 is " + std::to_string(pFitter->GetParameter(1)) + " > 0 -> there is exponential growth in fit function!");
    m_chStatus.at(channel).set(PileupExpGrowthBit, true);
    return false;
  }
  return true; // all good
}

/**
 * Perform a stretched exponential fit in baseline-subtracted baseline samples and set relevant status bits.
 * Returns true if the fit and subtraction are good, false if there was a problem.
 */
bool RPDDataAnalyzer::doPileupStretchedExpFit(unsigned int channel, std::vector<std::pair<unsigned int, float>> const& pileupFitPoints)
{
  auto pFitter = std::make_unique<TLinearFitter>(1, "1 ++ (x + 4)**(0.5) ++ (x + 4)**(-0.5)");
  double x {};
  for (auto const& [sample, y] : pileupFitPoints) {
    x = sample;
    pFitter->AddPoint(&x, std::log(y));
  }
  if (pFitter->Eval()) {
    (*m_msgFunc_p)(ZDCMsg::Warn, "RPDDataAnalyzer::doPileupStretchedExpFit: there was an error while evaluating TLinearFitter!");
    m_chStatus.at(channel).set(PileupStretchedExpFitFailBit, true);
    return false;
  }
  auto getFitParam = [&pFitter](int i){return pFitter->GetParameter(i);};
  auto fGetFitErr = [&pFitter](int i){return static_cast<float>(pFitter->GetParError(i));};
  auto fGetFitParam = [&pFitter](int i){return static_cast<float>(pFitter->GetParameter(i));};
  //
  m_chPileupStretchedExpFitParams.at(channel) = {fGetFitParam(0), fGetFitParam(1), fGetFitParam(2)};
  m_chPileupStretchedExpFitParamErrs.at(channel) = {fGetFitErr(0), fGetFitErr(1), fGetFitErr(2)};
  m_ch2ndOrderStretchedExpPileupFuncs.at(channel) = [p0 = getFitParam(0), p1 = getFitParam(1), p2 = getFitParam(2)](unsigned int sample) {
    return std::exp(p0 + p1*std::pow(sample + 4, 0.5) + p2*std::pow(sample + 4, -0.5));
  };
  m_ch2ndOrderStretchedExpPileupMSE.at(channel) = calculateBaselineSamplesMSE(channel, m_ch2ndOrderStretchedExpPileupFuncs.at(channel));

  // check for exponential growth in parameters - we definitely don't want that for a function that describes pileup
  if (getFitParam(1) >= 0) {
    (*m_msgFunc_p)(ZDCMsg::Debug, "RPDDataAnalyzer::doPileupStretchedExpFit: p1 is " + std::to_string(getFitParam(1)) + " > 0 -> there is exponential growth in fit function!");
    m_chStatus.at(channel).set(PileupStretchedExpGrowthBit, true);
    return false;
  }
  if (getFitParam(2)/getFitParam(1) - 4 > 0) {
    (*m_msgFunc_p)(ZDCMsg::Debug, "RPDDataAnalyzer::doPileupStretchedExpFit: 1st deriv max occurs at sample " + std::to_string(getFitParam(1)) + " > 0 -> fit probably looks like a pulse (and not like pileup)");
    m_chStatus.at(channel).set(PileupStretchedExpPulseLikeBit, true);
    // analysis remains valid (for now)
  }
  return true; // all good
}

/**
 * Calculate the number of negative values in signal range.
 */
unsigned int RPDDataAnalyzer::countSignalRangeNegatives(std::vector<float> const& values) const
{
  unsigned int nNegatives = 0;
  for (unsigned int sample = m_nBaselineSamples; sample < m_endSignalSample; sample++) {
    if (values.at(sample) < 0) nNegatives++;
  }
  return nNegatives;
}

/**
 * Determine if there is pileup, subtract baseline and pileup, and set relevant status bits.
 * Returns true if pileup subtraction succeeded, false if there was a problem.
*/
bool RPDDataAnalyzer::doBaselinePileupSubtraction(unsigned int channel) {
  float nominalBaselineSubtrSum = 0;
  /** points (sample, baseline-subtracted ADC) with ADC above baseline, to be used in fit in case of pileup */
  std::vector<std::pair<unsigned int, float>> pileupFitPoints;
  for (unsigned int sample = 0; sample < m_nBaselineSamples; sample++) {
    float const& adc = m_chFADCData.at(channel).at(sample);
    float const adcBaselineSubtr = adc - m_nominalBaseline;
    nominalBaselineSubtrSum += adcBaselineSubtr;
    if (adcBaselineSubtr > 0) {
      // this sample is a candidate for pileup fit
      pileupFitPoints.emplace_back(sample, adcBaselineSubtr);
    }
  }
  float baselineStdDev = TMath::StdDev(m_chFADCData.at(channel).begin(), std::next(m_chFADCData.at(channel).begin(), m_nBaselineSamples));

  if (nominalBaselineSubtrSum < m_pileupBaselineSumThresh || baselineStdDev < m_pileupBaselineStdDevThresh) {
    // there is NO pileup, we will trust the average of baseline samples as a good baseline estimate
    m_chBaseline.at(channel) = TMath::Mean(m_chFADCData.at(channel).begin(), std::next(m_chFADCData.at(channel).begin(), m_nBaselineSamples));
    // calculate fadc data with baseline subtracted
    for (unsigned int sample = 0; sample < m_nSamples; sample++) {
      m_chCorrectedFadcData.at(channel).at(sample) = m_chFADCData.at(channel).at(sample) - m_chBaseline.at(channel);
    }
    if (countSignalRangeNegatives(m_chCorrectedFadcData.at(channel)) > m_nNegativesAllowed) {
      m_chStatus.at(channel).set(BadAvgBaselineSubtrBit, true);
      return false;
    }
    return true; // all good
  }

  // we suspect that there is pileup - use nominal baseline
  m_chBaseline.at(channel) = m_nominalBaseline;

  if (pileupFitPoints.size() < s_minPileupFitPoints) {
    m_chStatus.at(channel).set(InsufficientPileupFitPointsBit, true);
    // there are not enough points to do fit, so just use nominal baseline and call it a day
    for (unsigned int sample = 0; sample < m_nSamples; sample++) {
      m_chCorrectedFadcData.at(channel).at(sample) = m_chFADCData.at(channel).at(sample) - m_chBaseline.at(channel);
    }
    return true; // all good
  }

  // there is OOT pileup in this channel => expect approx. negative exponential in baseline samples
  m_chStatus.at(channel).set(OutOfTimePileupBit, true);
  // fit (approximately) to exponential and stretched exponential in baseline samples
  bool expFitSuccess = doPileupExpFit(channel, pileupFitPoints);
  bool stretchedExpFitSuccess = doPileupStretchedExpFit(channel, pileupFitPoints);

  if (stretchedExpFitSuccess) {
    // calculate fadc data with baseline and pileup contribution subtracted
    for (unsigned int sample = 0; sample < m_nSamples; sample++) {
      m_chCorrectedFadcData.at(channel).at(sample) = m_chFADCData.at(channel).at(sample) - m_chBaseline.at(channel) - m_ch2ndOrderStretchedExpPileupFuncs.at(channel)(sample);
    }
    if (countSignalRangeNegatives(m_chCorrectedFadcData.at(channel)) > m_nNegativesAllowed) {
      m_chStatus.at(channel).set(PileupBadStretchedExpSubtrBit, true);
      // fallback to exponential fit
    } else {
      m_chPileupFuncType.at(channel) = PileupFitFuncType::SecondOrderStretchedExp;
      return true; // all good
    }
  }

  if (expFitSuccess) {
    // calculate fadc data with baseline and pileup contribution subtracted
    for (unsigned int sample = 0; sample < m_nSamples; sample++) {
      m_chCorrectedFadcData.at(channel).at(sample) = m_chFADCData.at(channel).at(sample) - m_chBaseline.at(channel) - m_chExpPileupFuncs.at(channel)(sample);
    }
    if (countSignalRangeNegatives(m_chCorrectedFadcData.at(channel)) > m_nNegativesAllowed) {
      m_chStatus.at(channel).set(PileupBadExpSubtrBit, true);
      return false;
    }
    m_chPileupFuncType.at(channel) = PileupFitFuncType::Exp;
    return true; // all good
  }

  return false; // both fits are bad...we have no measure of pileup!
}

/**
 * Calculate max ADC and max sample.
*/
void RPDDataAnalyzer::calculateMaxSampleMaxAdc(unsigned int channel)
{
  if (m_chStatus.at(channel)[NoPulseBit]) {
    m_chMaxAdc.at(channel) = 0;
    m_chMaxAdcCalib.at(channel) = 0;
    m_chMaxSample.at(channel) = -1;
    return;
  }
  float maxAdc = -std::numeric_limits<float>::infinity();
  unsigned int maxSample = 0;
  for (unsigned int sample = m_nBaselineSamples; sample < m_endSignalSample; sample++) {
    float adc = m_chCorrectedFadcData.at(channel).at(sample);
    if (adc > maxAdc) {
      maxAdc = adc;
      maxSample = sample;
    }
  }
  m_chMaxAdc.at(channel) = maxAdc;
  m_chMaxAdcCalib.at(channel) = maxAdc*m_outputCalibFactors.at(channel);
  m_chMaxSample.at(channel) = maxSample;
}

/**
 * Calculate sum ADC and if there is pileup, calculate fractional pileup.
*/
void RPDDataAnalyzer::calculateSumAdc(unsigned int channel) {
  if (m_chStatus.at(channel)[NoPulseBit]) {
    m_chSumAdc.at(channel) = 0;
    m_chSumAdcCalib.at(channel) = 0;
    return;
  }
  // sum range is after baseline until end of signal
  float signalRangeAdcSum = 0;
  for (unsigned int sample = m_nBaselineSamples; sample < m_endSignalSample; sample++) {
    signalRangeAdcSum += m_chCorrectedFadcData.at(channel).at(sample);
  }
  m_chSumAdc.at(channel) = signalRangeAdcSum;
  m_chSumAdcCalib.at(channel) = signalRangeAdcSum*m_outputCalibFactors.at(channel);

  if (m_chStatus.at(channel)[OutOfTimePileupBit]) {
    // there is pileup in this channel, calculate fraction of baseline-subtracted raw signal
    // that is pileup (beginning of window until end of signal)
    std::function<float(unsigned int)> pileupFunc;
    switch (m_chPileupFuncType.at(channel)) {
      case PileupFitFuncType::SecondOrderStretchedExp:
        pileupFunc = m_ch2ndOrderStretchedExpPileupFuncs.at(channel);
        break;
      case PileupFitFuncType::Exp:
        pileupFunc = m_chExpPileupFuncs.at(channel);
        break;
      default:
        break;
    }

    float totalAdcSum = 0;
    float pileupTotalAdcSum = 0;
    for (unsigned int sample = 0; sample < m_endSignalSample; sample++) {
      totalAdcSum += m_chFADCData.at(channel).at(sample) - m_nominalBaseline;
      pileupTotalAdcSum += pileupFunc(sample);
    }
    if (totalAdcSum > 0) {
      m_chPileupFrac.at(channel) = pileupTotalAdcSum/totalAdcSum;
    } else {
      // avoid dividing by zero or negative, return sentinel value of -1
      m_chPileupFrac.at(channel) = -1;
    }
  } // else fractional pileup is zero initialized, and this is what we want for no pileup
}

/**
 * Set side status bits according to channel status bits.
*/
void RPDDataAnalyzer::setSideStatusBits()
{
  for (unsigned int channel = 0; channel < s_nChannels; channel++) {
    if (!m_chStatus.at(channel)[ValidBit]) m_sideStatus.set(ValidBit, false);

    if (m_chStatus.at(channel)[OutOfTimePileupBit]) m_sideStatus.set(OutOfTimePileupBit, true);
    if (m_chStatus.at(channel)[OverflowBit]) m_sideStatus.set(OverflowBit, true);
    if (m_chStatus.at(channel)[PrePulseBit]) m_sideStatus.set(PrePulseBit, true);
    if (m_chStatus.at(channel)[PostPulseBit]) m_sideStatus.set(PostPulseBit, true);
    if (m_chStatus.at(channel)[NoPulseBit]) m_sideStatus.set(NoPulseBit, true);
    if (m_chStatus.at(channel)[BadAvgBaselineSubtrBit]) m_sideStatus.set(BadAvgBaselineSubtrBit, true);
    if (m_chStatus.at(channel)[InsufficientPileupFitPointsBit]) m_sideStatus.set(InsufficientPileupFitPointsBit, true);
    if (m_chStatus.at(channel)[PileupStretchedExpFitFailBit]) m_sideStatus.set(PileupStretchedExpFitFailBit, true);
    if (m_chStatus.at(channel)[PileupStretchedExpGrowthBit]) m_sideStatus.set(PileupStretchedExpGrowthBit, true);
    if (m_chStatus.at(channel)[PileupBadStretchedExpSubtrBit]) m_sideStatus.set(PileupBadStretchedExpSubtrBit, true);
    if (m_chStatus.at(channel)[PileupExpFitFailBit]) m_sideStatus.set(PileupExpFitFailBit, true);
    if (m_chStatus.at(channel)[PileupExpGrowthBit]) m_sideStatus.set(PileupExpGrowthBit, true);
    if (m_chStatus.at(channel)[PileupBadExpSubtrBit]) m_sideStatus.set(PileupBadExpSubtrBit, true);
    if (m_chStatus.at(channel)[PileupStretchedExpPulseLikeBit]) m_sideStatus.set(PileupStretchedExpPulseLikeBit, true);
  }
}

/**
 * Analyze RPD data.
*/
void RPDDataAnalyzer::analyzeData()
{
  if (m_nChannelsLoaded != s_nChannels) {
    (*m_msgFunc_p)(ZDCMsg::Warn,
      "RPDDataAnalyzer::analyzeData: analyzing data with " + std::to_string(m_nChannelsLoaded) + " of "
      + std::to_string(s_nChannels) + " channels loaded"
    );
  }
  for (unsigned int channel = 0; channel < s_nChannels; channel++) {
    if (!checkOverflow(channel)) {
      // there was overflow, stop analysis
      m_chStatus.at(channel).set(ValidBit, false);
      continue;
    }
    if (!checkPulses(channel)) {
      // there was a pre-pulse or post-pulse, stop analysis
      m_chStatus.at(channel).set(ValidBit, false);
      continue;
    }
    // there is either only a good pulse or no pulse
    // only do baseline and pileup subtraction if there is a pulse!
    if (!m_chStatus.at(channel)[NoPulseBit] /* there is a pulse -> */ && !doBaselinePileupSubtraction(channel)) {
      // there was a pulse and a problem with baseline/pileup subtraction, stop analysis
      m_chStatus.at(channel).set(ValidBit, false);
      continue;
    }
    calculateMaxSampleMaxAdc(channel);
    calculateSumAdc(channel);
  }
  setSideStatusBits();
}

/**
 * Get sample of max of RPD data in signal range after pileup subtraction.
*/
unsigned int RPDDataAnalyzer::getChMaxSample(unsigned int channel) const
{
  return m_chMaxSample.at(channel);
}

/**
 * Get sum of RPD data in signal range after baseline and pileup subtraction.
*/
float RPDDataAnalyzer::getChSumAdc(unsigned int channel) const
{
  return m_chSumAdc.at(channel);
}

/**
 * Get sum of RPD data in signal range after baseline and pileup subtraction, with output calibration factors applied.
*/
float RPDDataAnalyzer::getChSumAdcCalib(unsigned int channel) const
{
  return m_chSumAdcCalib.at(channel);
}

/**
 * Get max of RPD data in signal range after baseline and pileup subtraction.
*/
float RPDDataAnalyzer::getChMaxAdc(unsigned int channel) const
{
  return m_chMaxAdc.at(channel);
}

/**
 * Get max of RPD data in signal range after baseline and pileup subtraction, with output calibration factors applied.
*/
float RPDDataAnalyzer::getChMaxAdcCalib(unsigned int channel) const
{
  return m_chMaxAdcCalib.at(channel);
}

/**
 * Get OOT pileup sum as a fraction of non-pileup sum in entire window (0 if no OOT pileup, -1 if sum ADC <= 0).
*/
float RPDDataAnalyzer::getChPileupFrac(unsigned int channel) const
{
  return m_chPileupFrac.at(channel);
}

/**
 * Get baseline used in baseline subtraction.
 */
float RPDDataAnalyzer::getChBaseline(unsigned int channel) const {
  return m_chBaseline.at(channel);
}

/**
 * Get parameters for pileup exponential fit (if pileup was detected and fit did not fail): exp( [0] + [1]*sample ).
 */
const std::vector<float>& RPDDataAnalyzer::getChPileupExpFitParams(unsigned int channel) const {
  return m_chPileupExpFitParams.at(channel);
}

/**
 * Get parameters for pileup stretched exponential fit (if pileup was detected and fit did not fail): exp( [0] + [1]*(sample + 4)**(0.5) + [2]*(sample + 4)**(-0.5) ).
 */
const std::vector<float>& RPDDataAnalyzer::getChPileupStretchedExpFitParams(unsigned int channel) const {
  return m_chPileupStretchedExpFitParams.at(channel);
}

/**
 * Get parameter errors for pileup exponential fit (if pileup was detected and fit did not fail).
 */
const std::vector<float>& RPDDataAnalyzer::getChPileupExpFitParamErrs(unsigned int channel) const {
  return m_chPileupExpFitParamErrs.at(channel);
}

/**
 * Get parameter errors for pileup stretched exponential fit (if pileup was detected and fit did not fail).
 */
const std::vector<float>& RPDDataAnalyzer::getChPileupStretchedExpFitParamErrs(unsigned int channel) const {
  return m_chPileupStretchedExpFitParamErrs.at(channel);
}

/**
 * Get mean squared error of pileup exponential fit in baseline samples (if pileup was detected and fit did not fail).
 */
float RPDDataAnalyzer::getChPileupExpFitMSE(unsigned int channel) const {
  return m_chExpPileupMSE.at(channel);
}

/**
 * Get mean squared error of pileup stretched exponential fit in baseline samples (if pileup was detected and fit did not fail).
 */
float RPDDataAnalyzer::getChPileupStretchedExpFitMSE(unsigned int channel) const {
  return m_ch2ndOrderStretchedExpPileupMSE.at(channel);
}

/**
 * Get status word for channel.
 */
unsigned int RPDDataAnalyzer::getChStatus(unsigned int channel) const
{
  return static_cast<unsigned int>(m_chStatus.at(channel).to_ulong());
}

/**
 * Get status word for side.
 */
unsigned int RPDDataAnalyzer::getSideStatus() const
{
  return static_cast<unsigned int>(m_sideStatus.to_ulong());
}

} // namespace ZDC
