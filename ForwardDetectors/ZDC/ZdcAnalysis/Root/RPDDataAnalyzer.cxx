#include "ZdcAnalysis/RPDDataAnalyzer.h"

#include "TLinearFitter.h"
#include "TMath.h"
#include <limits>

const auto zeroVector = [](std::vector<float>& v){ v.assign(v.size(), 0); };
const auto zeroVectorVector = [](std::vector<std::vector<float>>& vv){ for (std::vector<float>& v : vv) v.assign(v.size(), 0); };
const auto zeroPileupFunc = [](unsigned int){ return 0; };
const auto zeroPileupFuncVector = [](std::vector<std::function<float(unsigned int)>>& v){ v.assign(v.size(), zeroPileupFunc); };

RPDDataAnalyzer::RPDDataAnalyzer(
  ZDCMsg::MessageFunctionPtr messageFunc_p, const std::string& tag, const RPDConfig& config, std::vector<float> const& calibFactors
) :
  m_msgFunc_p(messageFunc_p),
  m_tag(tag),
  m_nRows(config.nRows),
  m_nColumns(config.nColumns),
  m_nChannels(m_nRows*m_nColumns),
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
  m_AdcOverflow(config.AdcOverflow),
  m_calibFactors(calibFactors),
  m_chFadcData(m_nChannels, std::vector<float>(m_nSamples, 0)),
  m_chCorrectedFadcData(m_nChannels, std::vector<float>(m_nSamples, 0)),
  m_chMaxSample(m_nChannels, 0),
  m_chSumAdc(m_nChannels, 0),
  m_chSumAdcCalib(m_nChannels, 0),
  m_chMaxAdc(m_nChannels, 0),
  m_chMaxAdcCalib(m_nChannels, 0),
  m_chPileupFrac(m_nChannels, 0),
  m_chBaseline(m_nChannels, 0),
  m_chPileupExpFitParams(m_nChannels, std::vector<float>(2, 0)),
  m_chPileupStretchedExpFitParams(m_nChannels, std::vector<float>(3, 0)),
  m_chPileupExpFitParamErrs(m_nChannels, std::vector<float>(2, 0)),
  m_chPileupStretchedExpFitParamErrs(m_nChannels, std::vector<float>(3, 0)),
  m_chPileupFuncType(m_nChannels, false),
  m_chExpPileupFuncs(m_nChannels, zeroPileupFunc),
  m_ch2ndOrderStretchedExpPileupFuncs(m_nChannels, zeroPileupFunc),
  m_chExpPileupMSE(m_nChannels, 0),
  m_ch2ndOrderStretchedExpPileupMSE(m_nChannels, 0),
  m_chStatus(m_nChannels)
{
  if (m_endSignalSample == 0) m_endSignalSample = m_nSamples; // sentinel value 0 -> go to end of waveform
  if (m_calibFactors.size() != m_nChannels) {
    (*m_msgFunc_p)(ZDCMsg::Fatal,
      "RPDDataAnalyzer::RPDDataAnalyzer: received incorrect number of channels in calibration factors ("
        + std::to_string(m_calibFactors.size()) + " != " + std::to_string(m_nChannels) + ")"
    );
  }
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
    (*m_msgFunc_p)(ZDCMsg::Fatal, "RPDDataAnalyzer::loadChannelData: received incorrect number of samples in FADC data");
  }
  for (unsigned int sample = 0; sample < m_nSamples; sample++) {
    m_chFadcData.at(channel).at(sample) = FadcData.at(sample);
  }
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

  zeroVectorVector(m_chFadcData);
  zeroVectorVector(m_chCorrectedFadcData);
  zeroVectorVector(m_chPileupExpFitParams);
  zeroVectorVector(m_chPileupStretchedExpFitParams);
  zeroVectorVector(m_chPileupExpFitParamErrs);
  zeroVectorVector(m_chPileupStretchedExpFitParamErrs);
  m_chPileupFuncType.assign(m_chPileupFuncType.size(), false);

  zeroPileupFuncVector(m_chExpPileupFuncs);
  zeroPileupFuncVector(m_ch2ndOrderStretchedExpPileupFuncs);

  zeroVector(m_chExpPileupMSE);
  zeroVector(m_ch2ndOrderStretchedExpPileupMSE);

  m_nChannelsLoaded = 0;
  zeroVector(m_chMaxSample);
  zeroVector(m_chSumAdc);
  zeroVector(m_chSumAdcCalib);
  zeroVector(m_chMaxAdc);
  zeroVector(m_chMaxAdcCalib);
  zeroVector(m_chPileupFrac);
  zeroVector(m_chBaseline);
}

/**
 * Check for overflow and set relevant status bit.
 * Returns true if there was NO overflow (c'est bon).
*/
bool RPDDataAnalyzer::checkOverflow(unsigned int channel)
{
  for (unsigned int sample = 0; sample < m_nSamples; sample++) {
    if (m_chFadcData.at(channel).at(sample) >= m_AdcOverflow) {
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
    float secondDiff = m_chFadcData.at(channel).at(sample + 1) - 2*m_chFadcData.at(channel).at(sample) + m_chFadcData.at(channel).at(sample - 1);
    if (secondDiff*m_calibFactors.at(channel) > m_pulse2ndDerivThresh) continue; // no pulse here
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

  return !hasPrePulse && !hasPostPulse && !hasNoPulse; // true only if there is a good pulse and no other pulse
}

/**
 * Calculate the mean squared error of the fit function in the baseline samples.
*/
float RPDDataAnalyzer::calculateBaselineSamplesMSE(unsigned int channel, std::function<float(unsigned int)> const& fit) const
{
  float MSE = 0;
  for (unsigned int sample = 0; sample < m_nBaselineSamples; sample++) {
    MSE += std::pow(m_chFadcData.at(channel).at(sample) - m_chBaseline.at(channel) - fit(sample), 2);
  }
  MSE /= m_nBaselineSamples;
  return MSE*std::pow(m_calibFactors.at(channel), 2);
}

/**
 * Perform an exponential fit in baseline-subtracted baseline samples and set relevant status bits.
 * Returns true if the fit and subtraction are good, false if there was a problem.
 */
bool RPDDataAnalyzer::doPileupExpFit(unsigned int channel)
{
  TLinearFitter fitter(1, "1 ++ x");
  double x, y;
  for (unsigned int sample = 0; sample < m_nBaselineSamples; sample++) {
    x = sample;
    y = m_chFadcData.at(channel).at(sample) - m_chBaseline.at(channel);
    if (y <= 0) continue;
    fitter.AddPoint(&x, std::log(y));
  }
  if (fitter.Eval()) {
    (*m_msgFunc_p)(ZDCMsg::Warn, "RPDDataAnalyzer::doPileupExpFit: there was an error while evaluating TLinearFitter!");
    m_chStatus.at(channel).set(PileupExpFitFailBit, true);
    return false;
  }
  m_chPileupExpFitParams.at(channel) = {static_cast<float>(fitter.GetParameter(0)), static_cast<float>(fitter.GetParameter(1))};
  m_chPileupExpFitParamErrs.at(channel) = {static_cast<float>(fitter.GetParError(0)), static_cast<float>(fitter.GetParError(1))};
  m_chExpPileupFuncs.at(channel) = [intercept = fitter.GetParameter(0), slope = fitter.GetParameter(1)](unsigned int sample) { return std::exp(intercept + slope*sample); };
  m_chExpPileupMSE.at(channel) = calculateBaselineSamplesMSE(channel, m_chExpPileupFuncs.at(channel));

  // check for exponential growth in parameters - we definitely don't want that for a function that describes pileup
  if (fitter.GetParameter(1) >= 0) {
    (*m_msgFunc_p)(ZDCMsg::Debug, "RPDDataAnalyzer::doPileupExpFit: p1 is " + std::to_string(fitter.GetParameter(1)) + " > 0 -> there is exponential growth in fit function!");
    m_chStatus.at(channel).set(PileupExpGrowthBit, true);
    return false;
  }
  return true; // all good
}

/**
 * Perform a stretched exponential fit in baseline-subtracted baseline samples and set relevant status bits.
 * Returns true if the fit and subtraction are good, false if there was a problem.
 */
bool RPDDataAnalyzer::doPileupStretchedExpFit(unsigned int channel)
{
  TLinearFitter fitter(1, "1 ++ (x + 4)**(0.5) ++ (x + 4)**(-0.5)");
  double x, y;
  for (unsigned int sample = 0; sample < m_nBaselineSamples; sample++) {
    x = sample;
    y = m_chFadcData.at(channel).at(sample) - m_chBaseline.at(channel);
    if (y <= 0) continue;
    fitter.AddPoint(&x, std::log(y));
  }
  if (fitter.Eval()) {
    (*m_msgFunc_p)(ZDCMsg::Warn, "RPDDataAnalyzer::doPileupStretchedExpFit: there was an error while evaluating TLinearFitter!");
    m_chStatus.at(channel).set(PileupStretchedExpFitFailBit, true);
    return false;
  }
  m_chPileupStretchedExpFitParams.at(channel) = {static_cast<float>(fitter.GetParameter(0)), static_cast<float>(fitter.GetParameter(1)), static_cast<float>(fitter.GetParameter(2))};
  m_chPileupStretchedExpFitParamErrs.at(channel) = {static_cast<float>(fitter.GetParError(0)), static_cast<float>(fitter.GetParError(1)), static_cast<float>(fitter.GetParError(2))};
  m_ch2ndOrderStretchedExpPileupFuncs.at(channel) = [p0 = fitter.GetParameter(0), p1 = fitter.GetParameter(1), p2 = fitter.GetParameter(2)](unsigned int sample) {
    return std::exp(p0 + p1*std::pow(sample + 4, 0.5) + p2*std::pow(sample + 4, -0.5));
  };
  m_ch2ndOrderStretchedExpPileupMSE.at(channel) = calculateBaselineSamplesMSE(channel, m_ch2ndOrderStretchedExpPileupFuncs.at(channel));

  // check for exponential growth in parameters - we definitely don't want that for a function that describes pileup
  if (fitter.GetParameter(1) >= 0) {
    (*m_msgFunc_p)(ZDCMsg::Debug, "RPDDataAnalyzer::doPileupStretchedExpFit: p1 is " + std::to_string(fitter.GetParameter(1)) + " > 0 -> there is exponential growth in fit function!");
    m_chStatus.at(channel).set(PileupStretchedExpGrowthBit, true);
    return false;
  }
  if (fitter.GetParameter(2)/fitter.GetParameter(1) - 4 > 0) {
    (*m_msgFunc_p)(ZDCMsg::Debug, "RPDDataAnalyzer::doPileupStretchedExpFit: 1st deriv max occurs at sample " + std::to_string(fitter.GetParameter(1)) + " > 0 -> fit probably looks like a pulse (and not like pileup)");
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
  float const& calibFactor = m_calibFactors.at(channel);

  float baselineSum = 0;
  unsigned int nFitPoints = 0;
  for (unsigned int sample = 0; sample < m_nBaselineSamples; sample++) {
    float &adc = m_chFadcData.at(channel).at(sample);
    baselineSum += adc;
    if (adc - m_nominalBaseline > 0) nFitPoints++;
  }
  float baselineStdDev = TMath::RMS(m_chFadcData.at(channel).begin(), std::next(m_chFadcData.at(channel).begin(), m_nBaselineSamples));

  if (baselineSum*calibFactor < m_pileupBaselineSumThresh || baselineStdDev*calibFactor < m_pileupBaselineStdDevThresh) {
    // there is NO pileup, we will trust the average of baseline samples as a good baseline estimate
    m_chBaseline.at(channel) = TMath::Mean(m_chFadcData.at(channel).begin(), std::next(m_chFadcData.at(channel).begin(), m_nBaselineSamples));
    // calculate fadc data with baseline subtracted
    for (unsigned int sample = 0; sample < m_nSamples; sample++) {
      m_chCorrectedFadcData.at(channel).at(sample) = m_chFadcData.at(channel).at(sample) - m_chBaseline.at(channel);
    }
    if (countSignalRangeNegatives(m_chCorrectedFadcData.at(channel)) > m_nNegativesAllowed) {
      m_chStatus.at(channel).set(BadAvgBaselineSubtrBit, true);
      return false;
    }
    return true; // all good
  }

  // we suspect that there is pileup - use nominal baseline
  m_chBaseline.at(channel) = m_nominalBaseline;

  if (nFitPoints < 2) {
    m_chStatus.at(channel).set(InsufficientPileupFitPointsBit, true);
    // there are not enough points to do fit, so just use nominal baseline and call it a day
    for (unsigned int sample = 0; sample < m_nSamples; sample++) {
      m_chCorrectedFadcData.at(channel).at(sample) = m_chFadcData.at(channel).at(sample) - m_chBaseline.at(channel);
    }
    return true; // all good
  }

  // there is OOT pileup in this channel => expect approx. negative exponential in baseline samples
  m_chStatus.at(channel).set(OutOfTimePileupBit, true);
  // fit (approximately) to exponential and stretched exponential in baseline samples
  bool expFitSuccess = doPileupExpFit(channel);
  bool stretchedExpFitSuccess = doPileupStretchedExpFit(channel);

  if (stretchedExpFitSuccess) {
    // calculate fadc data with baseline and pileup contribution subtracted
    for (unsigned int sample = 0; sample < m_nSamples; sample++) {
      m_chCorrectedFadcData.at(channel).at(sample) = m_chFadcData.at(channel).at(sample) - m_chBaseline.at(channel) - m_ch2ndOrderStretchedExpPileupFuncs.at(channel)(sample);
    }
    if (countSignalRangeNegatives(m_chCorrectedFadcData.at(channel)) > m_nNegativesAllowed) {
      m_chStatus.at(channel).set(PileupBadStretchedExpSubtrBit, true);
      // fallback to exponential fit
    } else {
      m_chPileupFuncType.at(channel) = true;
      return true; // all good
    }
  }

  if (expFitSuccess) {
    // calculate fadc data with baseline and pileup contribution subtracted
    for (unsigned int sample = 0; sample < m_nSamples; sample++) {
      m_chCorrectedFadcData.at(channel).at(sample) = m_chFadcData.at(channel).at(sample) - m_chBaseline.at(channel) - m_chExpPileupFuncs.at(channel)(sample);
    }
    if (countSignalRangeNegatives(m_chCorrectedFadcData.at(channel)) > m_nNegativesAllowed) {
      m_chStatus.at(channel).set(PileupBadExpSubtrBit, true);
      return false;
    }
    return true; // all good
  }

  return false; // both fits are bad...we have no measure of pileup!
}

/**
 * Calculate max ADC and max sample.
*/
void RPDDataAnalyzer::calculateMaxSampleMaxAdc(unsigned int channel)
{
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
  m_chMaxAdcCalib.at(channel) = maxAdc*m_calibFactors.at(channel);
  m_chMaxSample.at(channel) = maxSample;
}

/**
 * Calculate sum ADC and if there is pileup, calculate fractional pileup.
*/
void RPDDataAnalyzer::calculateSumAdc(unsigned int channel) {
  // sum range is after baseline until end of signal
  float signalRangeAdcSum = 0;
  for (unsigned int sample = m_nBaselineSamples; sample < m_endSignalSample; sample++) {
    signalRangeAdcSum += m_chCorrectedFadcData.at(channel).at(sample);
  }
  m_chSumAdc.at(channel) = signalRangeAdcSum;
  m_chSumAdcCalib.at(channel) = signalRangeAdcSum*m_calibFactors.at(channel);

  if (m_chStatus.at(channel)[OutOfTimePileupBit]) {
    // there is pileup in this channel, calculate fraction of baseline-subtracted raw signal
    // that is pileup (beginning of window until end of signal)
    std::function<float(unsigned int)> pileupFunc;
    if (m_chPileupFuncType.at(channel)) {
      // use stretched exponential
      pileupFunc = m_ch2ndOrderStretchedExpPileupFuncs.at(channel);
    } else {
      // use exponential
      pileupFunc = m_chExpPileupFuncs.at(channel);
    }

    float totalAdcSum = 0;
    float pileupTotalAdcSum = 0;
    for (unsigned int sample = 0; sample < m_endSignalSample; sample++) {
      totalAdcSum += m_chFadcData.at(channel).at(sample) - m_nominalBaseline;
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
  for (unsigned int channel = 0; channel < m_nChannels; channel++) {
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
  if (m_nChannelsLoaded != m_nChannels) {
    (*m_msgFunc_p)(ZDCMsg::Warn,
      "RPDDataAnalyzer::analyzeData: analyzing data with " + std::to_string(m_nChannelsLoaded) + " of "
      + std::to_string(m_nChannels) + " channels loaded"
    );
  }
  for (unsigned int channel = 0; channel < m_nChannels; channel++) {
    if (!checkOverflow(channel)) {
      // there was overflow, stop analysis
      m_chStatus.at(channel).set(ValidBit, false);
      continue;
    }
    if (!checkPulses(channel)) {
      // there was no pulse or bad pulse, stop analysis
      m_chStatus.at(channel).set(ValidBit, false);
      continue;
    }
    if (!doBaselinePileupSubtraction(channel)) {
      // there was a problem with baseline/pileup subtraction, stop analysis
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
 * Get sum of RPD data in signal range after baseline and pileup subtraction, with calibration factors applied.
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
 * Get max of RPD data in signal range after baseline and pileup subtraction, with calibration factors applied.
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
