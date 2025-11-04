/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "ZdcAnalysis/ZDCPulseAnalyzer.h"

#include "TFitResult.h"
#include "TFitResultPtr.h"
#include "TVirtualFitter.h"
#include "TList.h"
#include "TMinuit.h"

#include <algorithm>
#include <sstream>
#include <cmath>
#include <numeric>
#include <iomanip>
#include <stdexcept>

using JSON = ZDCJSONConfig::JSON;
//
// List of allowed JSON configuration parameters
//
//  For each parameter we have name, JSON value type, whether it can be set per channel, and whether it is required
//
//  if the type is -1, then there's no value, the presence of the parameter itself is a boolean -- i.e. enabling  
//
const ZDCJSONConfig::JSONParamList ZDCPulseAnalyzer::JSONConfigParams = {
  {"tag", {JSON::value_t::string, 1, true, true}},
  {"enabled", {JSON::value_t::boolean, 1, true, true}},
  {"LGMode", {JSON::value_t::number_unsigned, 1, false, true}},
  {"Nsample", {JSON::value_t::number_integer, 1, false, true}},
  {"FADCFreqMHz", {JSON::value_t::number_integer, 1, false, true}},
  {"preSampleIdx", {JSON::value_t::number_integer, 1, true, true}},
  {"nominalPedestal", {JSON::value_t::number_integer, 1, false, true}},
  {"fitFunction", {JSON::value_t::string, 1, true, true}}, 
  {"peakSample", {JSON::value_t::number_integer, 1, true, true}}, 
  {"peakTolerance", {JSON::value_t::number_integer, 1, true, false}}, 
  {"2ndDerivThreshHG", {JSON::value_t::number_integer, 1, true, true}},
  {"2ndDerivThreshLG", {JSON::value_t::number_integer, 1, true, true}},
  {"2ndDerivStep", {JSON::value_t::number_integer, 1, false, false}},
  {"HGOverflowADC", {JSON::value_t::number_integer, 1, true, true}}, 
  {"HGUnderflowADC", {JSON::value_t::number_integer, 1, true, true}}, 
  {"LGOverflowADC", {JSON::value_t::number_integer, 1, true, true}}, 
  {"nominalT0HG", {JSON::value_t::number_float, 1, true, true}},
  {"nominalT0LG", {JSON::value_t::number_float, 1, true, true}},
  {"nominalTau1", {JSON::value_t::number_float, 1, true, false}},
  {"nominalTau2", {JSON::value_t::number_float, 1, true, false}},
  {"fixTau1", {JSON::value_t::boolean, 1, true, false}},
  {"fixTau2", {JSON::value_t::boolean, 1, true, false}},
  {"T0CutsHG", {JSON::value_t::array, 2, true, true}},
  {"T0CutsLG", {JSON::value_t::array, 2, true, true}},
  {"chisqDivAmpCutHG", {JSON::value_t::number_float, 1, true, true}},
  {"chisqDivAmpCutLG", {JSON::value_t::number_float, 1, true, true}},
  {"gainFactorHG", {JSON::value_t::number_float, 1, true, true}},
  {"gainFactorLG", {JSON::value_t::number_float, 1, true, true}},
  {"noiseSigmaHG", {JSON::value_t::number_float, 1, true, true}},
  {"noiseSigmaLG", {JSON::value_t::number_float, 1, true, true}},
  {"enableRepass", {JSON::value_t::boolean, 1, false, false}},
  {"Repass2ndDerivThreshHG", {JSON::value_t::number_integer, 1, true, true}},
  {"Repass2ndDerivThreshLG", {JSON::value_t::number_integer, 1, true, true}},
  {"fitAmpMinMaxHG", {JSON::value_t::array, 2, true, false}},
  {"fitAmpMinMaxLG", {JSON::value_t::array, 2, true, false}},
  {"ampMinSignifHGLG", {JSON::value_t::array, 2, true, false}},
  {"enablePreExclusion", {JSON::value_t::array, 3, false, false}},
  {"enablePostExclusion", {JSON::value_t::array, 3, false, false}},
  {"enableTimingCorrection", {JSON::value_t::array, 2, false, false}},
  {"timeCorrCoeffHG", {JSON::value_t::array, 0, true, false}},
  {"timeCorrCoeffLG", {JSON::value_t::array, 0, true, false}},
  {"enableADCNLCorrection", {JSON::value_t::array, 3, false, false}},
  {"ADCNLCorrCoeffs", {JSON::value_t::array, 0, true, false}},
  {"useDelayed", {JSON::value_t::boolean, 1, false, false}},
  {"delayDeltaT", {JSON::value_t::number_float, 1, true, false}}
};
  

TH1* ZDCPulseAnalyzer::s_undelayedFitHist  = nullptr;
TH1* ZDCPulseAnalyzer::s_delayedFitHist    = nullptr;
TF1* ZDCPulseAnalyzer::s_combinedFitFunc   = nullptr;
float ZDCPulseAnalyzer::s_combinedFitTMax  = 1000;
float ZDCPulseAnalyzer::s_combinedFitTMin  = -0.5;   // add to allow switch to high gain by skipping early samples
std::vector<float> ZDCPulseAnalyzer::s_pullValues;

void ZDCPulseAnalyzer::CombinedPulsesFCN(int& /*numParam*/, double*, double& f, double* par, int flag)
{
  //  The first parameter is a correction factor to account for decrease in beam intensity between x
  //    and y scan. It is applied here and not passed to the actual fit function
  //
  int nSamples = s_undelayedFitHist->GetNbinsX();

  if (flag == 3) {
    s_pullValues.assign(nSamples * 2, 0);
  }

  double chiSquare = 0;

  float delayBaselineAdjust = par[0];

  // undelayed
  //
  for (int isample = 0; isample < nSamples; isample++) {
    double histValue = s_undelayedFitHist->GetBinContent(isample + 1);
    double histError = std::max(s_undelayedFitHist->GetBinError(isample + 1), 1.0);
    double t = s_undelayedFitHist->GetBinCenter(isample + 1);

    if (t > s_combinedFitTMax) break;
    if (t < s_combinedFitTMin) continue;

    double funcVal = s_combinedFitFunc->EvalPar(&t, &par[1]);

    double pull = (histValue - funcVal) / histError;

    if (flag == 3) s_pullValues[2 * isample] = pull;
    chiSquare += pull * pull;
  }

  // delayed
  //
  for (int isample = 0; isample < nSamples; isample++) {
    double histValue = s_delayedFitHist->GetBinContent(isample + 1);
    double histError = std::max(s_delayedFitHist->GetBinError(isample + 1), 1.0);
    double t = s_delayedFitHist->GetBinCenter(isample + 1);

    if (t > s_combinedFitTMax) break;
    if (t < s_combinedFitTMin) continue;

    double funcVal = s_combinedFitFunc->EvalPar(&t, &par[1]) + delayBaselineAdjust;
    double pull = (histValue - funcVal) / histError;

    if (flag == 3) s_pullValues[2 * isample + 1] = pull;
    chiSquare += pull * pull;
  }

  f = chiSquare;
}


ZDCPulseAnalyzer::ZDCPulseAnalyzer(ZDCMsg::MessageFunctionPtr msgFunc_p, const std::string& tag, int Nsample, float deltaTSample, size_t preSampleIdx, int pedestal,
                                   const std::string& fitFunction, int peak2ndDerivMinSample,
                                   float peak2ndDerivMinThreshHG, float peak2ndDerivMinThreshLG) :
  m_msgFunc_p(std::move(msgFunc_p)),
  m_tag(tag), m_Nsample(Nsample),
  m_preSampleIdx(preSampleIdx),
  m_deltaTSample(deltaTSample),
  m_pedestal(pedestal), m_fitFunction(fitFunction),
  m_peak2ndDerivMinSample(peak2ndDerivMinSample),
  m_peak2ndDerivMinThreshLG(peak2ndDerivMinThreshLG),
  m_peak2ndDerivMinThreshHG(peak2ndDerivMinThreshHG),
  m_ADCSamplesHGSub(Nsample, 0), m_ADCSamplesLGSub(Nsample, 0),
  m_ADCSSampSigHG(Nsample, 0), m_ADCSSampSigLG(Nsample, 0), 
  m_samplesSub(Nsample, 0)
{
  // Create the histogram used for fitting
  //
  m_tmin = -deltaTSample / 2;
  m_tmax = m_tmin + ((float) Nsample) * deltaTSample;
  m_defaultFitTMax = m_tmax;
  
  std::string histName = "ZDCFitHist" + tag;
  std::string histNameLGRefit = "ZDCFitHist" + tag + "_LGRefit";

  m_fitHist = std::make_unique<TH1F>(histName.c_str(), "", m_Nsample, m_tmin, m_tmax);
  m_fitHistLGRefit = std::make_unique<TH1F>(histNameLGRefit.c_str(), "", m_Nsample, m_tmin, m_tmax);

  m_fitHist->SetDirectory(0);
  m_fitHistLGRefit->SetDirectory(0);

  SetDefaults();
  Reset();
}

ZDCPulseAnalyzer::ZDCPulseAnalyzer(ZDCMsg::MessageFunctionPtr msgFunc_p, const JSON& configJSON) :
  m_msgFunc_p(std::move(msgFunc_p))
{
  SetDefaults();

  // auto [result, resultString] = ValidateJSONConfig(configJSON);
  // (*m_msgFunc_p)(ZDCMsg::Debug, "ValidateJSON produced result: " + resultString);

  auto [result2, resultString2] = ConfigFromJSON(configJSON);
  (*m_msgFunc_p)(ZDCMsg::Debug, "ConfigFromJSON produced result: "+ resultString2);

  dumpConfiguration();
  
  // Create the histogram used for fitting
  //
  if (m_freqMHz >1.e-6)  m_deltaTSample = 1000./m_freqMHz;
  m_tmin = -m_deltaTSample / 2;
  m_tmax = m_tmin + ((float)m_Nsample) * m_deltaTSample;
  m_defaultFitTMax = m_tmax;

  std::string histName = "ZDCFitHist" + m_tag;
  std::string histNameLGRefit = "ZDCFitHist" + m_tag + "_LGRefit";

  m_fitHist = std::make_unique<TH1F>(histName.c_str(), "", m_Nsample, m_tmin, m_tmax);
  m_fitHistLGRefit = std::make_unique<TH1F>(histNameLGRefit.c_str(), "", m_Nsample, m_tmin, m_tmax);

  m_fitHist->SetDirectory(0);
  m_fitHistLGRefit->SetDirectory(0);

  Reset();
}

void ZDCPulseAnalyzer::enableDelayed(float deltaT, float pedestalShift, bool fixedBaseline)
{
  m_useDelayed = true;
  m_useFixedBaseline = fixedBaseline;

  m_delayedDeltaT = deltaT;
  m_delayedPedestalDiff = pedestalShift;

  m_deltaTSample /= 2.;

  std::string delayedHGName = std::string(m_fitHist->GetName()) + "delayed";
  std::string delayedLGName = std::string(m_fitHistLGRefit->GetName()) + "delayed";

  m_delayedHist = std::make_unique<TH1F>(delayedHGName.c_str(), "", m_Nsample, m_tmin + m_delayedDeltaT, m_tmax + m_delayedDeltaT);
  m_delayedHist->SetDirectory(0);

  m_delayedHistLGRefit = std::make_unique<TH1F>(delayedLGName.c_str(), "", m_Nsample, m_tmin + m_delayedDeltaT, m_tmax + m_delayedDeltaT);
  m_delayedHistLGRefit->SetDirectory(0);

  m_ADCSamplesHGSub.assign(2 * m_Nsample, 0);
  m_ADCSamplesLGSub.assign(2 * m_Nsample, 0);
}

void ZDCPulseAnalyzer::enableRepass(float peak2ndDerivMinRepassHG, float peak2ndDerivMinRepassLG)
{
  m_enableRepass = true;
  m_peak2ndDerivMinRepassHG = peak2ndDerivMinRepassHG;
  m_peak2ndDerivMinRepassLG = peak2ndDerivMinRepassLG;
}

void ZDCPulseAnalyzer::SetDefaults()
{
  m_LGMode = LGModeNormal;
  
  m_nominalTau1 = 1.5;
  m_nominalTau2 = 5;

  m_fixTau1 = false;
  m_fixTau2 = false;

  m_HGOverflowADC  = 3500;
  m_HGUnderflowADC = 1;
  m_LGOverflowADC  = 3900;

  // Default values for the gain factors uswed to match low and high gain
  //
  m_gainFactorLG = 10;
  m_gainFactorHG = 1;

  m_2ndDerivStep = 1;

  m_noiseSigHG = 1;
  m_noiseSigLG = 1;

  m_timeCutMode = 0;
  m_chisqDivAmpCutLG = 100;
  m_chisqDivAmpCutHG = 100;

  m_LGT0CorrParams.assign(4, 0);
  m_HGT0CorrParams.assign(4, 0);

  // m_defaultFitTMax = m_tmax;
  // m_defaultFitTMin = m_tmin;
  
  m_fitAmpMinHG = 1;
  m_fitAmpMinLG = 1;

  m_fitAmpMaxHG = 1500;
  m_fitAmpMaxLG = 1500;

  m_haveSignifCuts = false;

  m_postPulse = false;
  m_prePulse = false;

  m_initialPrePulseT0  = -10;
  m_initialPrePulseAmp = 5;

  m_initialPostPulseT0  = 100;

  m_initialExpAmp = 0;
  m_fitPostT0lo   = 0;

  m_useDelayed = false;
  m_enablePreExcl = false;
  m_enablePostExcl = false;
  
  m_timingCorrMode = NoTimingCorr;
  m_haveNonlinCorr = false;
  m_quietFits = true;
  m_fitOptions = "s";
}

void ZDCPulseAnalyzer::Reset(bool repass)
{
  if (!repass) {
    m_haveData  = false;

    m_useLowGain = false;
    m_fail       = false;
    m_HGOverflow = false;

    m_HGUnderflow       = false;
    m_PSHGOverUnderflow = false;
    m_LGOverflow        = false;
    m_LGUnderflow       = false;

    m_ExcludeEarly = false;
    m_ExcludeLate  = false;

    m_adjTimeRangeEvent = false;
    m_backToHG_pre      = false;
    m_fixPrePulse       = false;

    m_minADCLG = -1;
    m_maxADCLG = -1;
    m_minADCHG = -1;
    m_maxADCHG = -1;
    
    m_minADCSampleHG = -1;
    m_maxADCSampleHG = -1;
    m_minADCSampleLG = -1;
    m_maxADCSampleLG = -1;

    m_ADCPeakHG = -1;
    m_ADCPeakLG = -1;
    
    int sampleVecSize = m_Nsample;
    if (m_useDelayed) sampleVecSize *= 2;

    m_ADCSamplesHG.clear();
    m_ADCSamplesLG.clear();
    
    m_ADCSamplesHGSub.clear();
    m_ADCSamplesLGSub.clear();

    m_ADCSSampSigHG.assign(sampleVecSize, m_noiseSigHG);
    m_ADCSSampSigLG.assign(sampleVecSize, m_noiseSigLG); 

    m_minSampleEvt = 0;
    m_maxSampleEvt = (m_useDelayed ? 2 * m_Nsample - 1 : m_Nsample - 1);

    m_usedPresampIdx = 0;

    m_fitTMax = m_defaultFitTMax;
    m_fitTMin = m_defaultFitTMin;

    m_lastHGOverFlowSample  = -999;
    m_firstHGOverFlowSample = 999;
  }

  
  m_defaultT0Max = m_deltaTSample * (m_peak2ndDerivMinSample + m_peak2ndDerivMinTolerance + 0.5);
  m_defaultT0Min = m_deltaTSample * (m_peak2ndDerivMinSample - m_peak2ndDerivMinTolerance - 0.5);

  if (m_initializedFits) {
    m_defaultFitWrapper ->SetT0Range(m_defaultT0Min, m_defaultT0Max);
    m_prePulseFitWrapper->SetT0Range(m_defaultT0Min, m_defaultT0Max);
    m_preExpFitWrapper->SetT0Range(m_defaultT0Min, m_defaultT0Max);
  }

  // -----------------------
  // Statuses
  //
  m_havePulse  = false;
  
  m_prePulse  = false;
  m_postPulse = false;
  m_fitFailed = false;
  m_badChisq  = false;

  m_badT0        = false;
  m_preExpTail   = false;
  m_repassPulse = false;

  m_fitMinAmp = false;
  m_evtLGRefit = false;
  m_failSigCut = false;

  // -----------------------

  m_delayedBaselineShift = 0;

  m_fitAmplitude = 0;
  m_ampNoNonLin = 0;
  m_fitTime      = -100;
  m_fitTimeSub   = -100;
  m_fitTimeCorr  = -100;
  m_fitTCorr2nd  = -100;

  m_fitPreT0   = -100;
  m_fitPreAmp  = -100;
  m_fitPostT0  = -100;
  m_fitPostAmp = -100;
  m_fitExpAmp  = -100;

  m_minDeriv2ndSig = -10;
  m_preExpSig = -10;
  m_prePulseSig = -10;

  m_fitChisq = 0;

  m_amplitude       = 0;
  m_ampError        = 0;
  m_preSampleAmp    = 0;
  m_preAmplitude    = 0;
  m_postAmplitude   = 0;
  m_expAmplitude    = 0;
  m_bkgdMaxFraction = 0;

  m_refitLGAmpl = 0;
  m_refitLGAmpError = 0;
  m_refitLGChisq = 0;
  m_refitLGTime = -100;
  m_refitLGTimeSub = -100;
    
  m_initialPrePulseT0  = -10;
  m_initialPrePulseAmp = 5;

  m_initialPostPulseT0  = 100;

  m_initialExpAmp = 0;
  m_fitPostT0lo   = 0;

  m_fitPulls.clear();
  
  m_sigMinLG = 0;
  m_sigMinHG = 0;
  
  m_samplesSub.clear();
  m_samplesDeriv2nd.clear();
}


void ZDCPulseAnalyzer::setMinimumSignificance(float sigMinHG, float sigMinLG)
{
  m_haveSignifCuts = true;
  m_sigMinHG = sigMinHG;
  m_sigMinLG = sigMinLG;
}

void ZDCPulseAnalyzer::SetGainFactorsHGLG(float gainFactorHG, float gainFactorLG)
{
  m_gainFactorHG = gainFactorHG;
  m_gainFactorLG = gainFactorLG;
}

void ZDCPulseAnalyzer::SetFitMinMaxAmp(float minAmpHG, float minAmpLG, float maxAmpHG, float maxAmpLG)
{
  m_fitAmpMinHG = minAmpHG;
  m_fitAmpMinLG = minAmpLG;

  m_fitAmpMaxHG = maxAmpHG;
  m_fitAmpMaxLG = maxAmpLG;
}

void ZDCPulseAnalyzer::SetTauT0Values(bool fixTau1, bool fixTau2, float tau1, float tau2, float t0HG, float t0LG)
{
  m_fixTau1     = fixTau1;
  m_fixTau2     = fixTau2;
  m_nominalTau1 = tau1;
  m_nominalTau2 = tau2;

  m_nominalT0HG = t0HG;
  m_nominalT0LG = t0LG;

  std::ostringstream ostrm;
  ostrm << "ZDCPulseAnalyzer::SetTauT0Values:: m_fixTau1=" << m_fixTau1 << "  m_fixTau2=" << m_fixTau2 << "  m_nominalTau1=" << m_nominalTau1 << "  m_nominalTau2=" << m_nominalTau2 << "  m_nominalT0HG=" << m_nominalT0HG << "  m_nominalT0LG=" << m_nominalT0LG;

  (*m_msgFunc_p)(ZDCMsg::Info, ostrm.str());

  m_initializedFits = false;
}

void ZDCPulseAnalyzer::SetFitTimeMax(float tmax)
{
  if (tmax < m_tmin) {
    (*m_msgFunc_p)(ZDCMsg::Error, ("ZDCPulseAnalyzer::SetFitTimeMax:: invalid FitTimeMax: " + std::to_string(tmax)));
    return;
  }

  m_defaultFitTMax = std::min(tmax, m_defaultFitTMax);

  if (m_initializedFits) SetupFitFunctions();
}

void ZDCPulseAnalyzer::SetADCOverUnderflowValues(int HGOverflowADC, int HGUnderflowADC, int LGOverflowADC)
{
  m_HGOverflowADC  = HGOverflowADC;
  m_LGOverflowADC  = LGOverflowADC;
  m_HGUnderflowADC = HGUnderflowADC;
}

void ZDCPulseAnalyzer::SetCutValues(float chisqDivAmpCutHG, float chisqDivAmpCutLG,
                                    float deltaT0MinHG, float deltaT0MaxHG,
                                    float deltaT0MinLG, float deltaT0MaxLG)
{
  m_chisqDivAmpCutHG = chisqDivAmpCutHG;
  m_chisqDivAmpCutLG = chisqDivAmpCutLG;

  m_T0CutLowHG = deltaT0MinHG;
  m_T0CutLowLG = deltaT0MinLG;

  m_T0CutHighHG = deltaT0MaxHG;
  m_T0CutHighLG = deltaT0MaxLG;
}

void ZDCPulseAnalyzer::enableTimeSigCut(bool AND, float sigCut, const std::string& TF1String,
					const std::vector<double>& parsHG, 
					const std::vector<double>& parsLG)
{
  m_timeCutMode = AND ? 2 : 1;
  m_t0CutSig = sigCut;

  // Make the TF1's that provide the resolution
  //
  std::string timeResHGName = "TimeResFuncHG_" + m_tag;
  std::string timeResLGName = "TimeResFuncLG_" + m_tag;

  TF1* funcHG_p = new TF1(timeResHGName.c_str(), TF1String.c_str(), 0, m_HGOverflowADC);
  TF1* funcLG_p = new TF1(timeResLGName.c_str(), TF1String.c_str(), 0, m_LGOverflowADC);

  if (parsHG.size() != static_cast<unsigned int>(funcHG_p->GetNpar()) ||
      parsLG.size() != static_cast<unsigned int>(funcLG_p->GetNpar())) {
    //
    // generate an error message
  }

  funcHG_p->SetParameters(&parsHG[0]);
  funcLG_p->SetParameters(&parsLG[0]);
  
  m_timeResFuncHG_p.reset(funcHG_p);
  m_timeResFuncLG_p.reset(funcLG_p);

}

void ZDCPulseAnalyzer::enableFADCCorrections(bool correctPerSample, std::unique_ptr<const TH1>& correHistHG, std::unique_ptr<const TH1>& correHistLG)
{
  m_haveFADCCorrections = true;
  m_FADCCorrPerSample = correctPerSample;
  
  // check for appropriate limits
  //
  auto getXmin=[](const TH1 * pH){
    return pH->GetXaxis()->GetXmin();
  };
  auto getXmax=[](const TH1 * pH){
    return pH->GetXaxis()->GetXmax();
  };
  auto xmin= getXmin(correHistHG.get());
  auto xmax= getXmax(correHistHG.get());
  if (std::abs(xmin+0.5) > 1e-3 || std::abs(xmax - 4095.5) > 1e-3) {
    (*m_msgFunc_p)(ZDCMsg::Error, "ZDCPulseAnalyzer::enableFADCCorrections:: invalid high gain correction histogram range: xmin, xmax = " +
				   std::to_string(xmin ) + ", " + std::to_string(xmax) );
  }
  else {
    m_FADCCorrHG = std::move(correHistHG);
  }
  xmin= getXmin(correHistLG.get());
  xmax= getXmax(correHistLG.get());
  if (std::abs(xmin+0.5) > 1e-3 ||
      std::abs(xmax - 4095.5) > 1e-3) {
    (*m_msgFunc_p)(ZDCMsg::Error, "ZDCPulseAnalyzer::enableFADCCorrections:: invalid low gain correction histogram range: xmin, xmax = " +
				   std::to_string(xmin) + ", " + std::to_string(xmax) );
  }
  else {
    m_FADCCorrLG = std::move(correHistLG);
  }
}

std::vector<float>  ZDCPulseAnalyzer::GetFitPulls(bool refitLG) const
{
  //
  // If there was no pulse for this event, return an empty vector (see reset() method)
  //
  if (!m_havePulse) {
    return m_fitPulls;
  }

  //
  // The combined (delayed+undelayed) pulse fitting fills out m_fitPulls directly
  //
  if (m_useDelayed) {
    return m_fitPulls;
  }
  else {
    //
    // When not using combined fitting, We don't have the pre-calculated pulls. Calculate them on the fly. 
    //
    std::vector<float> pulls(m_Nsample, -100);
    
    const TH1* dataHist_p = refitLG ? m_fitHistLGRefit.get() : m_fitHist.get();
    const TF1* fit_p = static_cast<const TF1*>(dataHist_p->GetListOfFunctions()->Last());
    
    for (size_t ibin = 0; ibin < m_Nsample ; ibin++) {
      float t = dataHist_p->GetBinCenter(ibin + 1);
      float fitVal = fit_p->Eval(t);
      float histVal = dataHist_p->GetBinContent(ibin + 1);
      float histErr = dataHist_p->GetBinError(ibin + 1);
      float pull = (histVal - fitVal)/histErr;
      pulls[ibin] = pull;
    }

    return pulls;
  }
}

double ZDCPulseAnalyzer::getAmplitudeCorrection(bool highGain)
{
  double amplCorrFactor  = 1;
  
  // If we have FADC correction and we aren't applying it per-sample, do so here 
  //
  if (m_haveFADCCorrections && !m_FADCCorrPerSample) {
    double fadcCorr = highGain ? m_FADCCorrHG->Interpolate(m_fitAmplitude) : m_FADCCorrLG->Interpolate(m_fitAmplitude);
    amplCorrFactor *= fadcCorr;
  }

  double amplCorr = m_fitAmplitude * amplCorrFactor;
  
  // If we have a non-linear correction, apply it here   
  //   We apply it as an inverse correction - i.e. we divide by a correction
  //     term tha is a sum of coefficients times the ADC minus a reference
  //     to a power. The lowest power is 1, the highest is deteremined by
  //     the number of provided coefficients 

  if (m_haveNonlinCorr) {
    float invNLCorr = 1.0;
    float nlPolyArg = (amplCorr - m_nonLinCorrRefADC) / m_nonLinCorrRefScale;
	
    if (highGain) {
      for (size_t power = 1; power <= m_nonLinCorrParamsHG.size(); power++) {
	      invNLCorr += m_nonLinCorrParamsHG[power - 1]*pow(nlPolyArg, power);
      }
    }
    else {
      for (size_t power = 1; power <= m_nonLinCorrParamsLG.size(); power++) {
	      invNLCorr += m_nonLinCorrParamsLG[power - 1]*pow(nlPolyArg, power);
      }
    }

    amplCorrFactor /= invNLCorr;  
  }
  
  return amplCorrFactor;
}

void ZDCPulseAnalyzer::SetupFitFunctions()
{
  float prePulseTMin = 0;
  float prePulseTMax = prePulseTMin + m_deltaTSample * (m_peak2ndDerivMinSample - m_peak2ndDerivMinTolerance);

  if (m_fitFunction == "FermiExp") {
    if (!m_fixTau1 || !m_fixTau2) {
      //
      // Use the variable tau version of the expFermiFit
      //
      m_defaultFitWrapper = std::unique_ptr<ZDCFitWrapper>(new ZDCFitExpFermiVariableTaus(m_tag, m_tmin, m_tmax, m_fixTau1, m_fixTau2, m_nominalTau1, m_nominalTau2));
    }
    else {
      m_defaultFitWrapper = std::unique_ptr<ZDCFitWrapper>(new ZDCFitExpFermiFixedTaus(m_tag, m_tmin, m_tmax, m_nominalTau1, m_nominalTau2));
    }

    m_preExpFitWrapper = std::unique_ptr<ZDCFitExpFermiPreExp>(new ZDCFitExpFermiPreExp(m_tag, m_tmin, m_tmax, m_nominalTau1, m_nominalTau2, m_nominalTau2, false));
    m_prePulseFitWrapper = std::unique_ptr<ZDCPrePulseFitWrapper>(new ZDCFitExpFermiPrePulse(m_tag, m_tmin, m_tmax, m_nominalTau1, m_nominalTau2));
  }
  else if (m_fitFunction == "FermiExpRun3") {
    if (!m_fixTau1 || !m_fixTau2) {
      //
      // Use the variable tau version of the expFermiFit
      //
      m_defaultFitWrapper = std::unique_ptr<ZDCFitWrapper>(new ZDCFitExpFermiVariableTausRun3(m_tag, m_tmin, m_tmax, m_fixTau1, m_fixTau2, m_nominalTau1, m_nominalTau2));
    }
    else {
      m_defaultFitWrapper = std::unique_ptr<ZDCFitWrapper>(new ZDCFitExpFermiFixedTaus(m_tag, m_tmin, m_tmax, m_nominalTau1, m_nominalTau2));
    }

    m_preExpFitWrapper = std::unique_ptr<ZDCFitExpFermiPreExp>(new ZDCFitExpFermiPreExp(m_tag, m_tmin, m_tmax, m_nominalTau1, m_nominalTau2, 6, false));
    m_prePulseFitWrapper = std::unique_ptr<ZDCPrePulseFitWrapper>(new ZDCFitExpFermiPrePulse(m_tag, m_tmin, m_tmax, m_nominalTau1, m_nominalTau2));
  }
  else if (m_fitFunction == "FermiExpLHCf") {
    //
    // Use the variable tau version of the expFermiFit
    //
    m_defaultFitWrapper = std::unique_ptr<ZDCFitWrapper>(new ZDCFitExpFermiVariableTausLHCf(m_tag, m_tmin, m_tmax, m_fixTau1, m_fixTau2,
											      m_nominalTau1, m_nominalTau2));

    m_preExpFitWrapper = std::unique_ptr<ZDCFitExpFermiLHCfPreExp>(new ZDCFitExpFermiLHCfPreExp(m_tag, m_tmin, m_tmax, m_nominalTau1, m_nominalTau2, 6, false));
    
    m_prePulseFitWrapper = std::unique_ptr<ZDCPrePulseFitWrapper>(new ZDCFitExpFermiLHCfPrePulse(m_tag, m_tmin, m_tmax, m_nominalTau1, m_nominalTau2));
  }
  else if (m_fitFunction == "FermiExpLinear") {
    if (!m_fixTau1 || !m_fixTau2) {
      //
      // Use the variable tau version of the expFermiFit
      //
      m_defaultFitWrapper = std::unique_ptr<ZDCFitWrapper>(new ZDCFitExpFermiVariableTaus(m_tag, m_tmin, m_tmax, m_fixTau1, m_fixTau2, m_nominalTau1, m_nominalTau2));
    }
    else {
      m_defaultFitWrapper = std::unique_ptr<ZDCFitWrapper>(new ZDCFitExpFermiLinearFixedTaus(m_tag, m_tmin, m_tmax, m_nominalTau1, m_nominalTau2));
    }

    m_preExpFitWrapper = std::unique_ptr<ZDCFitExpFermiPreExp>(new ZDCFitExpFermiPreExp(m_tag, m_tmin, m_tmax, m_nominalTau1, m_nominalTau2, 6, false));
    m_prePulseFitWrapper = std::unique_ptr<ZDCPrePulseFitWrapper>(new ZDCFitExpFermiLinearPrePulse(m_tag, m_tmin, m_tmax, m_nominalTau1, m_nominalTau2));
  }
  else if (m_fitFunction == "ComplexPrePulse") {
    if (!m_fixTau1 || !m_fixTau2) {
      //
      // Use the variable tau version of the expFermiFit
      //
      m_defaultFitWrapper = std::unique_ptr<ZDCFitWrapper>(new ZDCFitExpFermiVariableTaus(m_tag, m_tmin, m_tmax, m_fixTau1, m_fixTau2, m_nominalTau1, m_nominalTau2));
    }
    else {
      m_defaultFitWrapper = std::unique_ptr<ZDCFitWrapper>(new ZDCFitExpFermiLinearFixedTaus(m_tag, m_tmin, m_tmax, m_nominalTau1, m_nominalTau2));
    }

    m_prePulseFitWrapper  = std::unique_ptr<ZDCPrePulseFitWrapper>(new ZDCFitComplexPrePulse(m_tag, m_tmin, m_tmax, m_nominalTau1, m_nominalTau2));
  }
  else if (m_fitFunction == "GeneralPulse") {
    if (!m_fixTau1 || !m_fixTau2) {
      //
      // Use the variable tau version of the expFermiFit
      //
      m_defaultFitWrapper = std::unique_ptr<ZDCFitWrapper>(new ZDCFitExpFermiVariableTaus(m_tag, m_tmin, m_tmax, m_fixTau1, m_fixTau2, m_nominalTau1, m_nominalTau2));
    }
    else {
      m_defaultFitWrapper = std::unique_ptr<ZDCFitWrapper>(new ZDCFitExpFermiLinearFixedTaus(m_tag, m_tmin, m_tmax, m_nominalTau1, m_nominalTau2));
    }

    m_prePulseFitWrapper  = std::unique_ptr<ZDCPrePulseFitWrapper>(new ZDCFitGeneralPulse(m_tag, m_tmin, m_tmax, m_nominalTau1, m_nominalTau2));
  }
  else {
    (*m_msgFunc_p)(ZDCMsg::Fatal, "Wrong fit function type: " + m_fitFunction);
  }

  m_prePulseFitWrapper->SetPrePulseT0Range(prePulseTMin, prePulseTMax);
  
  m_initializedFits = true;
}

bool ZDCPulseAnalyzer::LoadAndAnalyzeData(const std::vector<float>& ADCSamplesHG, const std::vector<float>&  ADCSamplesLG)
{
  if (m_useDelayed) {
    (*m_msgFunc_p)(ZDCMsg::Fatal, "ZDCPulseAnalyzer::LoadAndAnalyzeData:: Wrong LoadAndAnalyzeData called -- expecting both delayed and undelayed samples");
  }

  if (!m_initializedFits) SetupFitFunctions();

  // Clear any transient data
  //
  Reset(false);

  // Make sure we have the right number of samples. Should never fail. necessry?
  //
  if (ADCSamplesHG.size() != m_Nsample || ADCSamplesLG.size() != m_Nsample) {
    m_fail = true;
    return false;
  }

  m_ADCSamplesHG = ADCSamplesHG;
  m_ADCSamplesLG = ADCSamplesLG;
  
  return DoAnalysis(false);
}

bool ZDCPulseAnalyzer::LoadAndAnalyzeData(const std::vector<float>& ADCSamplesHG, const std::vector<float>&  ADCSamplesLG,
					  const std::vector<float>& ADCSamplesHGDelayed, const std::vector<float>& ADCSamplesLGDelayed)
{
  if (!m_useDelayed) {
    (*m_msgFunc_p)(ZDCMsg::Fatal, "ZDCPulseAnalyzer::LoadAndAnalyzeData:: Wrong LoadAndAnalyzeData called -- expecting only undelayed samples");
  }

  if (!m_initializedFits) SetupFitFunctions();

  // Clear any transient data
  //
  Reset();

  // Make sure we have the right number of samples. Should never fail. necessry?
  //
  if (ADCSamplesHG.size() != m_Nsample || ADCSamplesLG.size() != m_Nsample ||
      ADCSamplesHGDelayed.size() != m_Nsample || ADCSamplesLGDelayed.size() != m_Nsample) {
    m_fail = true;
    return false;
  }

  m_ADCSamplesHG.reserve(m_Nsample*2);
  m_ADCSamplesLG.reserve(m_Nsample*2);
  
  // Now do pedestal subtraction and check for overflows
  //
  for (size_t isample = 0; isample < m_Nsample; isample++) {
    float ADCHG = ADCSamplesHG[isample];
    float ADCLG = ADCSamplesLG[isample];

    float ADCHGDelay = ADCSamplesHGDelayed[isample] - m_delayedPedestalDiff;
    float ADCLGDelay = ADCSamplesLGDelayed[isample] - m_delayedPedestalDiff;

    if (m_delayedDeltaT > 0) {
      m_ADCSamplesHG.push_back(ADCHG);
      m_ADCSamplesHG.push_back(ADCHGDelay);
      
      m_ADCSamplesLG.push_back(ADCLG);
      m_ADCSamplesLG.push_back(ADCLGDelay);
    }
    else {
      m_ADCSamplesHG.push_back(ADCHGDelay);
      m_ADCSamplesHG.push_back(ADCHG);
      
      m_ADCSamplesLG.push_back(ADCLGDelay);
      m_ADCSamplesLG.push_back(ADCLG);
    }
  }
    
  return DoAnalysis(false);
}

bool ZDCPulseAnalyzer::ReanalyzeData()
{
  Reset(true);

  bool result = DoAnalysis(true);
  if (result && havePulse()) {
    m_repassPulse = true;
  }

  return result;
}

bool ZDCPulseAnalyzer::ScanAndSubtractSamples()
{
  //
  // Dump samples to verbose output
  //
  bool doDump = (*m_msgFunc_p)(ZDCMsg::Verbose, "Dumping all samples before subtraction: ");
      
  m_NSamplesAna = m_ADCSamplesHG.size();

  m_ADCSamplesHGSub.assign(m_NSamplesAna, 0);
  m_ADCSamplesLGSub.assign(m_NSamplesAna, 0);

  m_useSampleHG.assign(m_NSamplesAna, true);
  m_useSampleLG.assign(m_NSamplesAna, true);

  // Now do pedestal subtraction and check for overflows
  //
  m_minADCHG = m_HGOverflowADC;
  m_minADCLG = m_LGOverflowADC;

  m_maxADCHG = 0;
  m_maxADCLG = 0;

  for (size_t isample = 0; isample < m_NSamplesAna; isample++) {
    float ADCHG = m_ADCSamplesHG[isample];
    float ADCLG = m_ADCSamplesLG[isample];

    //
    // We always pedestal subtract even if the sample isn't going to be used in analysis
    //  basically for diagnostics
    //
    m_ADCSamplesHGSub[isample] = ADCHG - m_pedestal;
    m_ADCSamplesLGSub[isample] = ADCLG - m_pedestal;

        
    // If we have the per-sample FADC correction, we apply it after pedestal correction
    //   since we analyze the FADC response after baseline (~ same as pedestal) subtraction
    //
    if (m_haveFADCCorrections && m_FADCCorrPerSample) {
      double fadcCorrHG = m_FADCCorrHG->Interpolate(m_ADCSamplesHGSub[isample]);
      double fadcCorrLG	= m_FADCCorrLG->Interpolate(m_ADCSamplesLGSub[isample]);

      m_ADCSamplesHGSub[isample] *= fadcCorrHG;
      m_ADCSamplesLGSub[isample] *= fadcCorrLG;

      if (doDump) {
	std::ostringstream dumpString;
	dumpString << "After FADC correction, sample " << isample << ", HG ADC = " << m_ADCSamplesHGSub[isample]
		   << ", LG ADC = " << m_ADCSamplesLGSub[isample] << std::endl;
	(*m_msgFunc_p)(ZDCMsg::Verbose, dumpString.str().c_str());
      }
    }

    if (ADCHG > m_maxADCHG) {
      m_maxADCHG = ADCHG;
      m_maxADCSampleHG = isample;
    }
    else if (ADCHG < m_minADCHG) {
      m_minADCHG = ADCHG;
      m_minADCSampleHG = isample;
    }
	     
    if (ADCLG > m_maxADCLG) {
      m_maxADCLG = ADCLG;
      m_maxADCSampleLG = isample;
    }
    else if (ADCLG < m_minADCLG) {
      m_minADCLG = ADCLG;
      m_minADCSampleLG = isample;
    }

    if (m_useSampleHG[isample]) {
      if (m_enablePreExcl) {
	if (ADCHG > m_preExclHGADCThresh && isample < m_maxSamplesPreExcl) {
	  m_useSampleHG[isample] = false;
	  continue;
	}
      }

      if (m_enablePostExcl) {
	if (ADCHG > m_postExclHGADCThresh && isample >= m_NSamplesAna - m_maxSamplesPostExcl) {
	  m_useSampleHG[isample] = false;
	  continue;
	}
      }

      // If we get here, we've passed the above filters on the sample
      //
      if (ADCHG > m_HGOverflowADC) {
	m_HGOverflow = true;
	
	if (isample == m_preSampleIdx) m_PSHGOverUnderflow = true;

	// Note: the implementation of the explicit pre- and post- sample exclusion should make this
	//   code obselete but before removing it we first need to validate the pre- and post-exclusion
	//
	if ((int) isample > m_lastHGOverFlowSample)  m_lastHGOverFlowSample  = isample;
	if ((int) isample < m_firstHGOverFlowSample) m_firstHGOverFlowSample = isample;
      }
      else if (ADCHG < m_HGUnderflowADC) {
	m_HGUnderflow = true;
	
	if (isample == m_preSampleIdx) m_PSHGOverUnderflow  = true;
      }
    }

    // Now the low gain sample
    //
    if (m_useSampleLG[isample]) {
      if (m_enablePreExcl) {
	if (ADCLG > m_preExclLGADCThresh && isample <= m_maxSamplesPreExcl) {
	  m_useSampleLG[isample] = false;
	  continue;
	}
      }
      
      if (m_enablePostExcl) {
	if (ADCLG > m_postExclLGADCThresh && isample >= m_NSamplesAna - m_maxSamplesPostExcl) {
	  m_useSampleLG[isample] = false;
	  m_ExcludeLate = true;
	  continue;
	}
      }

      if (ADCLG > m_LGOverflowADC) {
	m_LGOverflow = true;
	m_fail = true;
	m_amplitude = m_LGOverflowADC * m_gainFactorLG; // Give a vale here even though we know it's wrong because
	//   the user may not check the return value and we know that
	//   amplitude is bigger than this
      }
      
      if (ADCLG == 0) {
	m_LGUnderflow = true;
	m_fail = true;
      }
    }
  }

  // if (doDump) {
  //   (*m_msgFunc_p)(ZDCMsg::Verbose, "Dump of useSamples: ");
    
  //   std::ostringstream dumpStringUseHG;
  //   dumpStringUseHG << "HG: ";
  //   for (auto val : m_useSampleHG) {
  //     dumpStringUseHG  << val << " ";
  //   }
  //   (*m_msgFunc_p)(ZDCMsg::Verbose, dumpStringUseHG.str().c_str());
    
  //   std::ostringstream dumpStringUseLG;
  //   dumpStringUseLG << "LG: ";
  //   for (auto val : m_useSampleLG) {
  //     dumpStringUseLG  << val << " ";
  //   }
  //   (*m_msgFunc_p)(ZDCMsg::Verbose, dumpStringUseLG.str().c_str());
  // }
 
  // This ugly code should be obseleted by the introduction of the better, pre- and post-sample exclusion but
  //   that code still has to be fully validated.
  //
  // See if we can still use high gain even in the case of HG overflow if the overflow results from
  //   the first few samples or the last few samples
  //
  if (m_HGOverflow && !m_HGUnderflow) {
    if (m_lastHGOverFlowSample < 2 && m_lastHGOverFlowSample > -1) {
      m_HGOverflow = false;
      m_minSampleEvt = m_lastHGOverFlowSample + 1;
      m_adjTimeRangeEvent = true;
      m_backToHG_pre = true;
      m_ExcludeEarly = true;
    }
    else if (m_firstHGOverFlowSample < static_cast<int>(m_NSamplesAna) && m_firstHGOverFlowSample >= static_cast<int>(m_NSamplesAna - 2) ) {
      m_maxSampleEvt = m_firstHGOverFlowSample - 1;
      m_HGOverflow = false;
      m_adjTimeRangeEvent = true;
      m_ExcludeLate = true;
    }
  }

  //  (*m_msgFunc_p)(ZDCMsg::Verbose, "ZDCPulseAnalyzer:: " + m_tag + ": ScanAndSubtractSamples done");

  return true;
}

bool ZDCPulseAnalyzer::DoAnalysis(bool repass)
{
  float deriv2ndThreshHG = 0;
  float deriv2ndThreshLG = 0;

  if (!repass) {
    ScanAndSubtractSamples();
    
    deriv2ndThreshHG = m_peak2ndDerivMinThreshHG;
    deriv2ndThreshLG = m_peak2ndDerivMinThreshLG;
  }
  else {
    deriv2ndThreshHG = m_peak2ndDerivMinRepassHG;
    deriv2ndThreshLG = m_peak2ndDerivMinRepassLG;
  }

  m_useLowGain = m_HGUnderflow || m_HGOverflow || (m_LGMode == LGModeForceLG);
  if (m_useLowGain) {
    //    (*m_msgFunc_p)(ZDCMsg::Verbose, "ZDCPulseAnalyzer:: " + m_tag + " using low gain data ");

    bool result = AnalyzeData(m_NSamplesAna, m_preSampleIdx, m_ADCSamplesLGSub, m_useSampleLG,
			      deriv2ndThreshLG, m_noiseSigLG, m_LGT0CorrParams, 
                              m_chisqDivAmpCutLG, m_T0CutLowLG, m_T0CutHighLG);
    if (result) {
      //
      // +++BAC
      //
      // I have removed some code that attempted a refit in case of failure of the chi-square cut
      // Instead, we should implement an analysis of the failure with possible re-fit afterwards
      //  with possible change of the pre- or post-pulse condition
      //
      // --BAC
      //

      double amplCorrFactor = getAmplitudeCorrection(false);
      
      //
      // Multiply amplitude by gain factor
      //
      m_ampNoNonLin   = m_fitAmplitude * m_gainFactorLG;
      m_amplitude     = m_fitAmplitude * amplCorrFactor * m_gainFactorLG;
      m_ampError      = m_fitAmpError * amplCorrFactor * m_gainFactorLG;
      m_preSampleAmp  = m_preSample    * m_gainFactorLG;
      m_preAmplitude  = m_fitPreAmp    * m_gainFactorLG;
      m_postAmplitude = m_fitPostAmp   * m_gainFactorLG;
      m_expAmplitude  = m_fitExpAmp    * m_gainFactorLG;

      // BAC: also scale up the 2nd derivative by the gain factor so low and high gain can be treated on the same footing
      //
      m_minDeriv2nd *= m_gainFactorLG;
    }

    return result;
  }
  else {
    //    (*m_msgFunc_p)(ZDCMsg::Verbose, "ZDCPulseAnalyzer:: " + m_tag + " using high gain data ");

    bool result = AnalyzeData(m_NSamplesAna, m_preSampleIdx, m_ADCSamplesHGSub, m_useSampleHG,
			      deriv2ndThreshHG, m_noiseSigHG, m_HGT0CorrParams, 
                              m_chisqDivAmpCutHG, m_T0CutLowHG, m_T0CutHighHG);
    if (result) {
      // +++BAC
      //
      // I have removed some code that attempted a refit in case of failure of the chi-square cut
      // Instead, we should implement an analysis of the failure with possible re-fit afterwards
      //  with possible change of the pre- or post-pulse condition
      //
      // --BAC

      m_preSampleAmp = m_preSample  * m_gainFactorHG;
      m_amplitude = m_fitAmplitude  * m_gainFactorHG;
      m_ampNoNonLin   = m_amplitude;
      m_ampError = m_fitAmpError    * m_gainFactorHG;
      m_preAmplitude  = m_fitPreAmp * m_gainFactorHG;
      m_postAmplitude = m_fitPostAmp* m_gainFactorHG;
      m_expAmplitude  = m_fitExpAmp * m_gainFactorHG;

      m_minDeriv2nd *= m_gainFactorHG;

      //  If we have a non-linear correction, apply it here
      //
      //    We apply it as an inverse correction - i.e. we divide by a correction
      //      term tha is a sum of coefficients times the ADC minus a reference
      //      to a power. The lowest power is 1, the highest is deteremined by
      //      the number of provided coefficients 
      //
      double amplCorrFactor = getAmplitudeCorrection(true);

      m_amplitude *= amplCorrFactor;
      m_ampError *= amplCorrFactor;
    }

    // If LG refit has been requested, do it now
    //
    if (m_LGMode == LGModeRefitLG && m_havePulse) {
      prepareLGRefit(m_ADCSamplesLGSub, m_ADCSSampSigLG, m_useSampleLG);
      DoFit(true);

      double amplCorrFactor = getAmplitudeCorrection(false);
      m_refitLGAmplCorr = m_refitLGAmpl*amplCorrFactor;
    }
    
    return result;
  }
}

bool ZDCPulseAnalyzer::AnalyzeData(size_t nSamples, size_t preSampleIdx,
                                   const std::vector<float>& samples,        // The samples used for this event
                                   const std::vector<bool>& useSample,       // The samples used for this event
                                   float peak2ndDerivMinThresh,
				   float noiseSig,
                                   const std::vector<float>& t0CorrParams,   // The parameters used to correct the t0
                                   float maxChisqDivAmp,                     // The maximum chisq / amplitude ratio
                                   float minT0Corr, float maxT0Corr          // The minimum and maximum corrected T0 values
                                  )
{

  // We keep track of which sample we used to do the subtraction sepaate from m_minSampleEvt
  //  because the corresponding time is the reference time we provide to the fit function when
  //  we have pre-pulses and in case we change the m_minSampleEvt after doing the subtraction
  //
  //    e.g. our refit when the chisquare cuts fails
  //

  // Find the first used sample in the event
  //
  bool haveFirst = false;
  unsigned int lastUsed = 0;
  
  for (unsigned int sample = preSampleIdx; sample < nSamples; sample++) {
    if (useSample[sample]) {
      //
      // We're going to use this sample in the analysis, update bookeeping
      //
      if (!haveFirst) {
	if (sample > m_minSampleEvt) m_minSampleEvt = sample;
	haveFirst = true;
      }
      else {
	lastUsed = sample;
      }
    }
  }

  if (lastUsed < m_maxSampleEvt) m_maxSampleEvt = lastUsed;

  // Check to see whether we've changed the range of samples used in the analysis
  //
  // Should be obseleted with reworking of the fitting using Root::Fit package
  //
  if (m_minSampleEvt > preSampleIdx) {
     m_ExcludeEarly = true;
     m_adjTimeRangeEvent = true;
  }

  if (m_maxSampleEvt < nSamples - 1) {
    m_adjTimeRangeEvent = true;
    m_ExcludeLate = true;
  }

  // Prepare for subtraction
  //
  m_usedPresampIdx = m_minSampleEvt;
  m_preSample = samples[m_usedPresampIdx];

  // std::ostringstream pedMessage;
  // pedMessage << "Pedestal index = " << m_usedPresampIdx << ", value = " << m_preSample;
  // (*m_msgFunc_p)(ZDCMsg::Verbose, pedMessage.str().c_str());

  m_samplesSub = samples;
  m_samplesSig.assign(m_NSamplesAna, noiseSig);
      
  
  //
  // When we are combinig delayed and undelayed samples we have to deal with the fact that
  //   the two readouts can have different noise and thus different baselines. Which is a huge
  //   headache.
  //
  if (m_useDelayed) {
    if (m_useFixedBaseline) {
      m_baselineCorr = m_delayedPedestalDiff;
    }
    else {
      //
      //  Use much-improved method to match delayed and undelayed baselines
      //
      m_baselineCorr = obtainDelayedBaselineCorr(m_samplesSub);
      std::ostringstream baselineMsg;
      baselineMsg << "Delayed samples baseline correction = " << m_baselineCorr << std::endl;
      (*m_msgFunc_p)(ZDCMsg::Debug, baselineMsg.str().c_str());
    }

    // Now apply the baseline correction to align ADC values for delayed and undelayed samples
    //
    for (size_t isample = 0; isample < nSamples; isample++) {
      if (isample % 2) m_samplesSub[isample] -= m_baselineCorr;
    }

    // If we use one of the delayed samples for presample, we have to adjust the presample value as well
    //
    if (m_usedPresampIdx % 2) m_preSample -= m_baselineCorr;
  }

  // Do the presample subtraction
  //
  std::for_each(m_samplesSub.begin(), m_samplesSub.end(), [ = ] (float & adcUnsub) {return adcUnsub -= m_preSample;} );

  // Calculate the second derivatives using step size m_2ndDerivStep
  //
  m_samplesDeriv2nd = Calculate2ndDerivative(m_samplesSub, m_2ndDerivStep);

  // Find the sample which has the lowest 2nd derivative. We loop over the range defined by the
  //  tolerance on the position of the minimum second derivative. Note: the +1 in the upper iterator is because
  //  that's where the loop terminates, not the last element.
  //
  SampleCIter minDeriv2ndIter;

  int upperDelta = std::min(m_peak2ndDerivMinSample + m_peak2ndDerivMinTolerance + 1, nSamples);

  minDeriv2ndIter = std::min_element(m_samplesDeriv2nd.begin() + m_peak2ndDerivMinSample - m_peak2ndDerivMinTolerance, m_samplesDeriv2nd.begin() + upperDelta);

  m_minDeriv2nd = *minDeriv2ndIter;
  m_minDeriv2ndSig = -m_minDeriv2nd/(std::sqrt(6.0)*noiseSig);

  m_minDeriv2ndIndex = std::distance(m_samplesDeriv2nd.cbegin(), minDeriv2ndIter);

  // BAC 02-04-23 This check turned out to be problematic. Todo: figure out how to replace
  //
  // // Also check the ADC value for the "peak" sample to make sure it is significant (at least 3 sigma)
  // // The factor of sqrt(2) on the noise is because we have done a pre-sample subtraction
  // //
  if (std::abs(m_minDeriv2nd) >= peak2ndDerivMinThresh) {
    m_havePulse = true;
  }
  else {
    m_havePulse = false;
  }

  // save the low and high gain ADC values at the peak -- if we have a pulse, at m_minDeriv2ndIndex
  //   otherwise at m_peak2ndDerivMinSample
  //
  if (m_havePulse) {
    m_ADCPeakHG = m_ADCSamplesHG.at(m_minDeriv2ndIndex);
    m_ADCPeakLG = m_ADCSamplesLG.at(m_minDeriv2ndIndex);
  }
  else {
    m_ADCPeakHG = m_ADCSamplesHG.at(m_peak2ndDerivMinSample);
    m_ADCPeakLG = m_ADCSamplesLG.at(m_peak2ndDerivMinSample);
  }
  
  // Now decide whether we have a preceeding pulse or not. There are two possible kinds of preceeding pulses:
  //   1) exponential tail from a preceeding pulse
  //   2) peaked pulse before the main pulse
  //
  //   we can, of course, have both
  //

  // To check for exponential tail, test the slope determined by the minimum ADC value (and pre-sample)
  // **beware** this can cause trouble in 2015 data where the pulses had overshoot due to the transformers
  //

  // Implement a much simpler test for presence of an exponential tail from an OOT pulse
  //   namely if the slope evaluated at the presample is significantly negative, then 
  //   we have a preceeding pulse.
  //
  // Note that we don't have to subtract the FADC value corresponding to the presample because 
  //   that subtraction has already been done.
  //
  if (m_havePulse) {
    // If we've alreday excluded early samples, we have almost by construction have negative exponential tail
    //
    if (m_ExcludeEarly) m_preExpTail = true;

    //
    //  The subtracted ADC value at m_usedPresampIdx is, by construction, zero
    //    The next sample has had the pre-sample subtracted, so it represents the initial derivative
    //
    float derivPresampleSig = m_samplesSub[m_usedPresampIdx+1]/(std::sqrt(2.0)*noiseSig);
    if (derivPresampleSig < -5) {
      m_preExpTail = true;
      m_preExpSig = derivPresampleSig;
    }
    
    for (unsigned int isample = m_usedPresampIdx; isample < m_samplesSub.size(); isample++) {
      if (!useSample[isample]) continue;

      float sampleSig = -m_samplesSub[isample]/(std::sqrt(2.0)*noiseSig);

      // Compare the derivative significant to the 2nd derivative significance, 
      //   so we don't waste time dealing with small perturbations on large signals
      //
      if ((sampleSig > 5 && sampleSig > 0.02*m_minDeriv2ndSig) || sampleSig > 0.5*m_minDeriv2ndSig) {
	m_preExpTail = true;
	if (sampleSig > m_preExpSig) m_preExpSig = sampleSig;
      }
    }

    // Now we search for maxima before the main pulse
    //
    int loopLimit = (m_havePulse ? m_minDeriv2ndIndex - 2 : m_peak2ndDerivMinSample - 2);
    int loopStart = m_minSampleEvt == 0 ? 1 : m_minSampleEvt;
    
    float maxPrepulseSig = 0;
    unsigned int maxPrepulseSample = 0;
    
    for (int isample = loopStart; isample <= loopLimit; isample++) {
      if (!useSample[isample]) continue;
      
      //
      // If any of the second derivatives prior to the peak are significantly negative, we have a an extra pulse
      //   prior to the main one -- as opposed to just an expnential tail
      //
      float prePulseSig = -m_samplesDeriv2nd[isample]/(std::sqrt(6.0)*noiseSig);

      // std::cout << m_tag << ", for sample " << isample << ", 2nd derivative = " << m_samplesDeriv2nd[isample]
      // 		<< " 0.05 * m_minDeriv2nd) = " << 0.05 * m_minDeriv2nd
      // 		<< ", prePulseSig = " << prePulseSig << std::endl;
      
      if ((prePulseSig > 6 && m_samplesDeriv2nd[isample] < 0.05 * m_minDeriv2nd) ||
	  m_samplesDeriv2nd[isample]  < 0.5*m_minDeriv2nd)
      {
	 m_prePulse = true;
	if (prePulseSig > maxPrepulseSig) {
	  maxPrepulseSig = prePulseSig;
	  maxPrepulseSample = isample;
	}
      }
    }

    if (m_prePulse) {
      //      std::cout << m_tag << ": prepulse sigma = " << m_prePulseSig << ", pre exp sigma = " << m_preExpSig << std::endl;
      m_prePulseSig = maxPrepulseSig;
      
      if (m_preExpTail) {
	//
	// We have a prepulse. If we already indicated an negative exponential,
	//   if the prepulse has greater significance, we override the negative exponential
	//
	if (m_prePulseSig >  m_preExpSig) {
	  m_preExpTail = false;
	}
	else {
	  m_prePulse = false;
	}
      }

      m_initialPrePulseAmp = m_samplesSub[maxPrepulseSample];
      m_initialPrePulseT0 = m_deltaTSample * (maxPrepulseSample);
    }

    //    if (m_preExpTail) m_prePulse = true;    

    // -----------------------------------------------------
    // Post pulse detection
    //
    unsigned int postStartIdx = std::max(static_cast<unsigned int>(m_minDeriv2ndIndex + 2),
					 static_cast<unsigned int>(m_peak2ndDerivMinSample + m_peak2ndDerivMinTolerance + 1));

    
    for (int isample = postStartIdx; isample < (int) nSamples - 1; isample++) {
      if (!useSample.at(isample)) continue;
      
      // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      // BAC 12-01-2024
      //
      // The following code is commented out as a temporary measure to deal with apparent reflections
      //   associated with large out-of-time pulses that introduce a "kink" that triggers the derivative
      //   test. A work-around that doesn't introduce specific code for the 2023 Pb+Pb run but allows
      //   adaption for this specific issue is going to take some work. For now we leave the 2nd derivative
      //   test, but will also need to introduce some configurability of the cut -- which we need anyway
      //
      // Calculate the forward derivative. the pulse should never increase on the tail. If it
      //   does, we almost certainly have a post-pulse
      //
      // float deriv = m_samplesSub[isample + 1] - m_samplesSub[isample];
      // if (deriv/(std::sqrt(2)*noiseSig) > 6) {
      //   m_postPulse = true;
      //   m_maxSampleEvt = isample;
      //   m_adjTimeRangeEvent = true;
      //   break;
      // }
      // else {
      //----------------------------------------------------------------------------------------------
      //
      // Now we check the second derivative which might also indicate a post pulse
      //   even if the derivative is not sufficiently large
      //
      // add small 1e-3 in division to avoid floating overflow
      //

      float deriv = m_samplesSub.at(isample + 1) - m_samplesSub.at(isample);
      float derivSig = deriv/(std::sqrt(2)*noiseSig);
      float deriv2ndSig = -m_samplesDeriv2nd.at(isample) / (std::sqrt(6)*noiseSig);

      float deriv2ndTest = m_samplesDeriv2nd.at(isample) / (-m_minDeriv2nd + 1.0e-3);

      if (derivSig > 5) {
	//
	// Check the 2nd derivative -- we should be at a minimum(?)
	//	
	if (std::abs(deriv2ndTest) > 0.15) {
	  m_postPulse = true;
	  m_maxSampleEvt = std::min<int>(isample - (m_2ndDerivStep - 1), m_maxSampleEvt);

	  m_adjTimeRangeEvent = true;
	  break;
	}
      }
      	  
      // The place to apply the cut on samples depends on whether we have found a "minimum" or a "maximum"
      //   The -m_2ndDerivStep for the minimum accounts for the shift between 2nd derivative and the samples
      //   if we find a maximum we cut one sample lower
      //
      if (deriv2ndSig > 5 && deriv2ndTest < -0.1) {
	m_postPulse = true;
	m_maxSampleEvt = std::min<int>(isample - m_2ndDerivStep, m_maxSampleEvt);

	m_adjTimeRangeEvent = true;
	break;
      }
    }
  }

  if (m_postPulse) {
    std::ostringstream ostrm;
    ostrm << "Post pulse found, m_maxSampleEvt = " << m_maxSampleEvt;
    (*m_msgFunc_p)(ZDCMsg::Debug, ostrm.str());
  }

  
  // -----------------------------------------------------

  //  Stop now if we have no pulse or we've detected a failure
  //
  if (m_fail || !m_havePulse) return false;

  if (!m_useDelayed) DoFit();
  else DoFitCombined();
  
  if (fitFailed()) {
    m_fail = true;
  }
  else {
    std::ostringstream ostrm;
    // ostrm << "Pulse fit successful with chisquare = " << m_fitChisq;
    // (*m_msgFunc_p)(ZDCMsg::Debug, ostrm.str());

    m_fitTimeCorr = m_fitTimeSub;

    if (m_timingCorrMode != NoTimingCorr) {
      //
      // We correct relative to the m_timingCorrRefADC using m_timingCorrScale to scale
      //
      double t0CorrFact = (m_fitAmplitude - m_timingCorrRefADC) / m_timingCorrScale;
      if (m_timingCorrMode == TimingCorrLog) t0CorrFact = std::log(t0CorrFact);

      // Calculate the correction using a polynomial of power determined by the size of the vector.
      //   For historical reasons we include here a constant term, though it is degenerate with
      //   the nominal t0 and/or timing calibrations.
      //
      float correction = 0;

      for (unsigned int ipow = 0; ipow < t0CorrParams.size(); ipow++) {
	correction += t0CorrParams[ipow]*std::pow(t0CorrFact, double(ipow));
      }

      // The correction represents the offset of the timing value from zero so we subtract the result
      //
      m_fitTimeCorr -= correction; 
    }

    bool failFixedCut = m_fitTimeCorr < minT0Corr || m_fitTimeCorr > maxT0Corr;

    // Calculate the timing significance. Note this implementation breaks the model for
    //  how DoAnalysis is currently used by explicitly testing m_useLowGain, but that model
    //  needs adjustment anyway ... (one step at a time)
    //
    // Note that to avoid coupling non-linear corrections to the amplitude and the 
    //   timing significance we evaluate the time resolution using the fit amplitude
    //
    if (m_timeCutMode != 0) {
      double timeResolution = 0;
      if (m_useLowGain) timeResolution = m_timeResFuncLG_p->Eval(m_fitAmplitude);
      else timeResolution = m_timeResFuncHG_p->Eval(m_fitAmplitude);

      m_timeSig = m_fitTimeCorr/timeResolution;
      if (std::abs(m_timeSig) > m_t0CutSig) {
	//
	// We've failed the significance cut. In OR mode (1) we mark the time
	//  as bad if we've lso failed the fixed cut. In AND mode, we mark it
	//  as bad regardless of the fixed cut.
	//
	if (m_timeCutMode == 1) {
	  if (failFixedCut) m_badT0 = true;
	}
	else m_badT0 = true;
      }
      else if (m_timeCutMode == 2 && failFixedCut) m_badT0 = failFixedCut;
    }
    else {
      m_timeSig = -1;
      m_badT0 = failFixedCut;
    }
    
    // Now check for valid chisq and valid time
    //
    if (m_fitChisq/m_fitNDoF > 2 && m_fitChisq / (m_fitAmplitude + 1.0e-6) > maxChisqDivAmp) m_badChisq = true;
  }

  return !m_fitFailed;
}

void ZDCPulseAnalyzer::prepareLGRefit(const std::vector<float>& samplesLG, const std::vector<float>& samplesSig,
				      const std::vector<bool>& useSamples)
{
  m_samplesLGRefit.clear();
  m_samplesSigLGRefit.clear();
 
  float presampleLG = m_ADCSamplesLGSub[m_usedPresampIdx];
    
  // Do the presample subtraction
  //
  for (unsigned int idx = 0; idx < samplesLG.size(); idx++) {
    m_samplesLGRefit.push_back(samplesLG[idx] - presampleLG);

    if (useSamples[idx]) {
      m_samplesSigLGRefit.push_back(samplesSig[idx]);
    }
    else {
      m_samplesSigLGRefit.push_back(0);
    }
  }
}


void ZDCPulseAnalyzer::DoFit(bool refitLG)
{
  bool fitLG = m_useLowGain || refitLG;
  
  TH1* hist_p = nullptr;
  FillHistogram(refitLG);
    
  // Set the initial values
  //
  float ampInitial, fitAmpMin, fitAmpMax, t0Initial;

  if (fitLG) {
    fitAmpMin = m_fitAmpMinLG;
    fitAmpMax = m_fitAmpMaxLG;
    t0Initial = m_nominalT0LG;
    ampInitial = m_ADCPeakLG;
  }
  else {
    ampInitial = m_ADCPeakHG;
    fitAmpMin = m_fitAmpMinHG;
    fitAmpMax = m_fitAmpMaxHG;
    t0Initial = m_nominalT0HG;
 }

  if (refitLG) {
    hist_p = m_fitHistLGRefit.get();
  }
  else {
    hist_p = m_fitHist.get();
  }
  if (ampInitial < fitAmpMin) ampInitial = fitAmpMin * 1.5;


  ZDCFitWrapper* fitWrapper = m_defaultFitWrapper.get();
  if (preExpTail()) {
    fitWrapper = m_preExpFitWrapper.get();
    (static_cast<ZDCPreExpFitWrapper*>(m_preExpFitWrapper.get()))->SetInitialExpPulse(m_initialExpAmp);
  }
  else if (prePulse()) {
    fitWrapper = m_prePulseFitWrapper.get();
    (static_cast<ZDCPrePulseFitWrapper*>(fitWrapper))->SetInitialPrePulse(m_initialPrePulseAmp, m_initialPrePulseT0,0,25);
  }
  
  if (m_adjTimeRangeEvent) {
    m_fitTMin = std::max(m_fitTMin, m_deltaTSample * m_minSampleEvt - m_deltaTSample / 2);
    m_fitTMax = std::min(m_fitTMax, m_deltaTSample * m_maxSampleEvt + m_deltaTSample / 2);

    float fitTReference = m_deltaTSample * m_usedPresampIdx;

    fitWrapper->Initialize(ampInitial, t0Initial, fitAmpMin, fitAmpMax, m_fitTMin, m_fitTMax, fitTReference);
  }
  else {
    fitWrapper->Initialize(ampInitial, t0Initial, fitAmpMin, fitAmpMax);
  }

  // Now perform the fit
  //
  std::string options = m_fitOptions + "Ns";
  if (m_quietFits) {
    options += "Q";
  }

  bool fitFailed = false;

  //  dumpTF1(fitWrapper->GetWrapperTF1RawPtr());
  checkTF1Limits(fitWrapper->GetWrapperTF1RawPtr());
      
  //
  //  Fit the data with the function provided by the fit wrapper
  //
  TFitResultPtr result_ptr = hist_p->Fit(fitWrapper->GetWrapperTF1RawPtr(), options.c_str(), "", m_fitTMin, m_fitTMax);
  int fitStatus = result_ptr;
  
  //
  // If the first fit failed, also check the EDM. If sufficiently small, the failure is almost surely due
  //   to parameter limits and we just accept the result
  //
  if (fitStatus != 0 && result_ptr->Edm() > 0.001) 
    {
    //
    // We contstrain the fit and try again
    //
    fitWrapper->ConstrainFit();
 
    TFitResultPtr constrFitResult_ptr = hist_p->Fit(fitWrapper->GetWrapperTF1RawPtr(), options.c_str(), "", m_fitTMin, m_fitTMax);
    fitWrapper->UnconstrainFit();

    if ((int) constrFitResult_ptr != 0) {
      //
      // Even the constrained fit failed, so we quit.
      //
      fitFailed = true;
    }
    else {
      // Now we try the fit again with the constraint removed
      //
      TFitResultPtr unconstrFitResult_ptr = hist_p->Fit(fitWrapper->GetWrapperTF1RawPtr(), options.c_str(), "", m_fitTMin, m_fitTMax);
      if ((int) unconstrFitResult_ptr != 0) {
	//
	// The unconstrained fit failed again, so we redo the constrained fit
	//
	fitWrapper->ConstrainFit();
      
	TFitResultPtr constrFit2Result_ptr = hist_p->Fit(fitWrapper->GetWrapperTF1RawPtr(), options.c_str(), "", m_fitTMin, m_fitTMax);
	if ((int) constrFit2Result_ptr != 0) {
	  //
	  // Even the constrained fit failed the second time, so we quit.
	  //
	  fitFailed = true;
	}

	result_ptr = constrFit2Result_ptr;
	fitWrapper->UnconstrainFit();
      }
      else {
	result_ptr = unconstrFitResult_ptr;
      }
    }
  }

  if (!m_fitFailed && m_saveFitFunc) {
    hist_p->GetListOfFunctions()->Clear();

    TF1* func = fitWrapper->GetWrapperTF1RawPtr();
    std::string name = func->GetName();

    TF1* copyFunc = static_cast<TF1*>(func->Clone((name + "_copy").c_str()));
    hist_p->GetListOfFunctions()->Add(copyFunc);
  }

  if (!refitLG) {
    m_fitFailed = fitFailed;
    m_bkgdMaxFraction = fitWrapper->GetBkgdMaxFraction();
    m_fitAmplitude = fitWrapper->GetAmplitude();
    m_fitAmpError = fitWrapper->GetAmpError();

    if (preExpTail()) {
      m_fitExpAmp  = (static_cast<ZDCPreExpFitWrapper*>(m_preExpFitWrapper.get()))->GetExpAmp();
    }
    else {
      m_fitExpAmp = 0;
    }
    
    m_fitTime      = fitWrapper->GetTime();
    m_fitTimeSub = m_fitTime - t0Initial;

    m_fitChisq = result_ptr->Chi2();
    m_fitNDoF = result_ptr->Ndf();
    
    m_fitTau1 = fitWrapper->GetTau1();
    m_fitTau2 = fitWrapper->GetTau2();
    
    // Here we need to check if the fit amplitude is small (close) enough to fitAmpMin.
    // with "< 1+epsilon" where epsilon ~ 1%
    if (m_fitAmplitude < fitAmpMin * 1.01) {
      m_fitMinAmp = true;
    }

    float sigMinCut = fitLG ? m_sigMinLG : m_sigMinHG;
    if (m_fitAmpError > 1e-6) {
      if (m_fitAmplitude/m_fitAmpError < sigMinCut) m_failSigCut = true;
    }
  }
  else {
    m_evtLGRefit = true;
    m_refitLGFitAmpl = fitWrapper->GetAmplitude();
    m_refitLGAmpl = fitWrapper->GetAmplitude() * m_gainFactorLG;
    m_refitLGAmpError = fitWrapper->GetAmpError() * m_gainFactorLG;
    m_refitLGChisq = result_ptr->Chi2();
    m_refitLGTime = fitWrapper->GetTime();
    m_refitLGTimeSub = m_refitLGTime - t0Initial;
  }
}

void ZDCPulseAnalyzer::DoFitCombined(bool refitLG)
{
  bool fitLG = refitLG || m_useLowGain;
  TH1* hist_p = nullptr, *delayedHist_p = nullptr;

  FillHistogram(refitLG);
  if (refitLG) {
    hist_p = m_fitHistLGRefit.get();
    delayedHist_p = m_delayedHistLGRefit.get();
  }
  else {
    hist_p = m_fitHist.get();
    delayedHist_p = m_delayedHist.get();
  }
  
  float fitAmpMin, fitAmpMax, t0Initial, ampInitial;
  if (fitLG) {
    ampInitial = m_ADCPeakLG;
    fitAmpMin = m_fitAmpMinLG;
    fitAmpMax = m_fitAmpMaxLG;
    t0Initial = m_nominalT0LG;
  }
  else {
    ampInitial = m_ADCPeakHG;
    fitAmpMin = m_fitAmpMinHG;
    fitAmpMax = m_fitAmpMaxHG;
    t0Initial = m_nominalT0HG;
  }

  // Set the initial values
  //
  if (ampInitial < fitAmpMin) ampInitial = fitAmpMin * 1.5;

  ZDCFitWrapper* fitWrapper = m_defaultFitWrapper.get();
  //if (prePulse()) fitWrapper = fitWrapper = m_preExpFitWrapper.get();

  if (preExpTail()) {
    fitWrapper = m_preExpFitWrapper.get();
    (static_cast<ZDCPreExpFitWrapper*>(m_preExpFitWrapper.get()))->SetInitialExpPulse(m_initialExpAmp);
  }
  else if (prePulse()) {
    fitWrapper = m_prePulseFitWrapper.get();
    (static_cast<ZDCPrePulseFitWrapper*>(fitWrapper))->SetInitialPrePulse(m_initialPrePulseAmp, m_initialPrePulseT0,0,25);
  }

  // Initialize the fit wrapper for this event, specifying a
  //   per-event fit range if necessary
  //
  if (m_adjTimeRangeEvent) {
    m_fitTMin = std::max(m_fitTMin, m_deltaTSample * m_minSampleEvt - m_deltaTSample / 2);
    m_fitTMax = std::min(m_fitTMax, m_deltaTSample * m_maxSampleEvt + m_deltaTSample / 2);

    float fitTReference = m_deltaTSample * m_usedPresampIdx;

    fitWrapper->Initialize(ampInitial, t0Initial, fitAmpMin, fitAmpMax, m_fitTMin, m_fitTMax, fitTReference);
  }
  else {
    fitWrapper->Initialize(ampInitial, t0Initial, fitAmpMin, fitAmpMax);
  }

  // Set up the virtual fitter
  //
  TFitter* theFitter = nullptr;

  if (prePulse()) {
    m_prePulseCombinedFitter = MakeCombinedFitter(fitWrapper->GetWrapperTF1RawPtr());

    theFitter = m_prePulseCombinedFitter.get();
  }
  else {
    m_defaultCombinedFitter = MakeCombinedFitter(fitWrapper->GetWrapperTF1RawPtr());

    theFitter = m_defaultCombinedFitter.get();
  }

  //  dumpTF1(fitWrapper->GetWrapperTF1RawPtr());

  // Set the static pointers to histograms and function for use in FCN
  //
  s_undelayedFitHist = hist_p;
  s_delayedFitHist = delayedHist_p;
  s_combinedFitFunc = fitWrapper->GetWrapperTF1RawPtr();
  s_combinedFitTMax = m_fitTMax;
  s_combinedFitTMin = m_fitTMin;

  
  size_t numFitPar = theFitter->GetNumberTotalParameters();

  theFitter->GetMinuit()->fISW[4] = -1;

  // Now perform the fit
  //
  if (m_quietFits) {
    theFitter->GetMinuit()->fISW[4] = -1;

    int  ierr= 0; 
    theFitter->GetMinuit()->mnexcm("SET NOWarnings",nullptr,0,ierr);
  }
  else theFitter->GetMinuit()->fISW[4] = 0;

  // Only include baseline shift in fit for pre-pulses. Otherwise baseline matching should work
  //
  if (prePulse()) {
    theFitter->SetParameter(0, "delayBaselineAdjust", 0, 0.01, -100, 100);
    theFitter->ReleaseParameter(0);
  }
  else {
    theFitter->SetParameter(0, "delayBaselineAdjust", 0, 0.01, -100, 100);
    theFitter->FixParameter(0);
  }

  
  double arglist[100];
  arglist[0] = 5000; // number of function calls
  arglist[1] = 0.01; // tolerance
  int status = theFitter->ExecuteCommand("MIGRAD", arglist, 2);

  double fitAmp = theFitter->GetParameter(1);
  
  // Capture the chi-square etc.
  //
  double chi2, edm, errdef;
  int nvpar, nparx;
  
  theFitter->GetStats(chi2, edm, errdef, nvpar, nparx);

  // Here we need to check if fitAmp is small (close) enough to fitAmpMin.
  // with "< 1+epsilon" where epsilon ~ 1%
  //
  if (status || fitAmp < fitAmpMin * 1.01 || edm > 0.01){

    //
    // We first retry the fit with no baseline adjust
    //
    theFitter->SetParameter(0, "delayBaselineAdjust", 0, 0.01, -100, 100);
    theFitter->FixParameter(0);

    // Here we need to check if fitAmp is small (close) enough to fitAmpMin.
    // with "< 1+epsilon" where epsilon ~ 1%
    if (fitAmp < fitAmpMin * 1.01) {
      if (m_adjTimeRangeEvent) {
        float fitTReference = m_deltaTSample * m_usedPresampIdx;
        fitWrapper->Initialize(ampInitial, t0Initial, fitAmpMin, fitAmpMax, m_fitTMin, m_fitTMax, fitTReference);
      }
      else {
        fitWrapper->Initialize(ampInitial, t0Initial, fitAmpMin, fitAmpMax);
      }
    }

    status = theFitter->ExecuteCommand("MIGRAD", arglist, 2);
    if (status != 0) {
      //
      // Fit failed event with no baseline adust, so be it
      //
      theFitter->ReleaseParameter(0);
      m_fitFailed = true;
    }
    else {
      //
      // The fit succeeded with no baseline adjust, re-fit allowing for baseline adjust using
      //  the parameters from previous fit as the starting point
      //
      theFitter->ReleaseParameter(0);
      status = theFitter->ExecuteCommand("MIGRAD", arglist, 2);

      if (status) {
        // Since we know the fit can succeed without the baseline adjust, go back to it
        //
        theFitter->SetParameter(0, "delayBaselineAdjust", 0, 0.01, -100, 100);
        theFitter->FixParameter(0);
        status = theFitter->ExecuteCommand("MIGRAD", arglist, 2);
      }
    }
  }
  else m_fitFailed = false;

  // Check to see if the fit forced the amplitude to the minimum, if so set the corresponding status bit
  //
  fitAmp = theFitter->GetParameter(1);

  // Here we need to check if fitAmp is small (close) enough to fitAmpMin.
  // with "< 1+epsilon" where epsilon ~ 1%
  if (fitAmp < fitAmpMin * 1.01) {
    m_fitMinAmp = true;
  }

  // Check that the amplitude passes minimum significance requirement
  //
  float sigMinCut = fitLG ? m_sigMinLG : m_sigMinHG;
  if (m_fitAmpError > 1e-6) {
    if (m_fitAmplitude/m_fitAmpError < sigMinCut) m_failSigCut = true;
  }

  if (!m_quietFits) theFitter->GetMinuit()->fISW[4] = -1;

  std::vector<double> funcParams(numFitPar - 1);
  std::vector<double> funcParamErrs(numFitPar - 1);

  // Save the baseline shift between delayed and undelayed samples
  //
  m_delayedBaselineShift = theFitter->GetParameter(0);

  // Capture and store the fit function parameteds and errors
  //
  for (size_t ipar = 1; ipar < numFitPar; ipar++) {
    funcParams[ipar - 1] = theFitter->GetParameter(ipar);
    funcParamErrs[ipar - 1] = theFitter->GetParError(ipar);
  }

  s_combinedFitFunc->SetParameters(&funcParams[0]);
  s_combinedFitFunc->SetParErrors(&funcParamErrs[0]);

  // Capture the chi-square etc.
  //
  theFitter->GetStats(chi2, edm, errdef, nvpar, nparx);

  int ndf = 2 * m_Nsample - nvpar;

  s_combinedFitFunc->SetChisquare(chi2);
  s_combinedFitFunc->SetNDF(ndf);

  // add to list of functions
  if (m_saveFitFunc) {
    s_undelayedFitHist->GetListOfFunctions()->Clear();
    s_undelayedFitHist->GetListOfFunctions()->Add(s_combinedFitFunc);

    s_delayedFitHist->GetListOfFunctions()->Clear();
    s_delayedFitHist->GetListOfFunctions()->Add(s_combinedFitFunc);
  }

  if (!refitLG) {
    // Save the pull values from the last call to FCN
    //
    arglist[0] = 3; // number of function calls
    theFitter->ExecuteCommand("Cal1fcn", arglist, 1);
    m_fitPulls = s_pullValues;
    
    m_fitAmplitude = fitWrapper->GetAmplitude();
    m_fitTime      = fitWrapper->GetTime();
    if (prePulse()) {
      m_fitPreT0   = (static_cast<ZDCPrePulseFitWrapper*>(m_prePulseFitWrapper.get()))->GetPreT0();
      m_fitPreAmp  = (static_cast<ZDCPrePulseFitWrapper*>(m_prePulseFitWrapper.get()))->GetPreAmp();
      m_fitPostT0  = (static_cast<ZDCPrePulseFitWrapper*>(m_prePulseFitWrapper.get()))->GetPostT0();
      m_fitPostAmp = (static_cast<ZDCPrePulseFitWrapper*>(m_prePulseFitWrapper.get()))->GetPostAmp();
    }
    
    if (preExpTail()) {
      m_fitExpAmp  = (static_cast<ZDCPreExpFitWrapper*>(m_preExpFitWrapper.get()))->GetExpAmp();
    }
    else {
      m_fitExpAmp = 0;
    }
    
    m_fitTimeSub = m_fitTime - t0Initial;
    m_fitChisq = chi2;
    m_fitNDoF = ndf;
    
    m_fitTau1 = fitWrapper->GetTau1();
    m_fitTau2 = fitWrapper->GetTau2();
    
    m_fitAmpError = fitWrapper->GetAmpError();
    m_bkgdMaxFraction = fitWrapper->GetBkgdMaxFraction();
  }
  else {
    m_evtLGRefit = true;
    m_refitLGFitAmpl = fitWrapper->GetAmplitude();
    m_refitLGAmpl = fitWrapper->GetAmplitude() * m_gainFactorLG;
    m_refitLGAmpError = fitWrapper->GetAmpError() * m_gainFactorLG;
    m_refitLGChisq = chi2;
    m_refitLGTime = fitWrapper->GetTime();
    m_refitLGTimeSub = m_refitLGTime - t0Initial;
  }
}

void ZDCPulseAnalyzer::checkTF1Limits(TF1* func)
{
  for (int ipar = 0; ipar < func->GetNpar(); ipar++) {
    double parLimitLow, parLimitHigh;
    
    func->GetParLimits(ipar, parLimitLow, parLimitHigh);
    
    (*m_msgFunc_p)(ZDCMsg::Debug, (
				  "ZDCPulseAnalyzer name=" + std::string(func->GetName())
				  + " ipar=" + std::to_string(ipar)
				  + " parLimitLow=" + std::to_string(parLimitLow)
				  + " parLimitHigh="+ std::to_string(parLimitHigh)
				  )
    		   );
    
    //if (std::abs(parLimitHigh / parLimitLow - 1) > 1e-6) {
    if (std::abs(parLimitHigh - parLimitLow) > (1e-6)*std::abs(parLimitLow)) {
      double value = func->GetParameter(ipar);
      if (value >= parLimitHigh) {
	value = parLimitHigh * 0.9;
      }
      else if (value <= parLimitLow) {
	value = parLimitLow + 0.1*std::abs(parLimitLow);
      }
      func->SetParameter(ipar, value);
    }
  }
}

std::unique_ptr<TFitter> ZDCPulseAnalyzer::MakeCombinedFitter(TF1* func)
{
  TVirtualFitter::SetDefaultFitter("Minuit");

  size_t nFitParams = func->GetNpar() + 1;
  std::unique_ptr<TFitter> fitter = std::make_unique<TFitter>(nFitParams);

  fitter->GetMinuit()->fISW[4] = -1;
  fitter->SetParameter(0, "delayBaselineAdjust", 0, 0.01, -100, 100);

  for (size_t ipar = 0; ipar < nFitParams - 1; ipar++) {
    double parLimitLow, parLimitHigh;

    func->GetParLimits(ipar, parLimitLow, parLimitHigh);
    if (std::abs(parLimitHigh / parLimitLow - 1) < 1e-6) {
      double value   = func->GetParameter(ipar);
      double lowLim  = std::min(value * 0.99, value * 1.01);
      double highLim = std::max(value * 0.99, value * 1.01);

      fitter->SetParameter(ipar + 1, func->GetParName(ipar), func->GetParameter(ipar), 0.01, lowLim, highLim);
      fitter->FixParameter(ipar + 1);
    }
    else {
      double value = func->GetParameter(ipar);
      if (value >= parLimitHigh)     value = parLimitHigh * 0.99;
      else if (value <= parLimitLow) value = parLimitLow * 1.01;

      double step = std::min((parLimitHigh - parLimitLow)/100., (value - parLimitLow)/100.);
      
      fitter->SetParameter(ipar + 1, func->GetParName(ipar), value, step, parLimitLow, parLimitHigh);
    }
  }

  fitter->SetFCN(ZDCPulseAnalyzer::CombinedPulsesFCN);

  return fitter;
}

void ZDCPulseAnalyzer::UpdateFitterTimeLimits(TFitter* fitter, ZDCFitWrapper* wrapper, bool prePulse)
{
  double parLimitLow, parLimitHigh;

  auto func_p = wrapper->GetWrapperTF1();
  func_p->GetParLimits(1, parLimitLow, parLimitHigh);

  fitter->SetParameter(2, func_p->GetParName(1), func_p->GetParameter(1), 0.01, parLimitLow, parLimitHigh);

  if (prePulse) {
    unsigned int parIndex = static_cast<ZDCPrePulseFitWrapper*>(wrapper)->GetPreT0ParIndex();

    func_p->GetParLimits(parIndex, parLimitLow, parLimitHigh);
    fitter->SetParameter(parIndex + 1, func_p->GetParName(parIndex), func_p->GetParameter(parIndex), 0.01, parLimitLow, parLimitHigh);
  }
}

void ZDCPulseAnalyzer::dump() const
{
  (*m_msgFunc_p)(ZDCMsg::Info, ("ZDCPulseAnalyzer dump for tag = " + m_tag));

  (*m_msgFunc_p)(ZDCMsg::Info, ("Presample index, value = " + std::to_string(m_preSampleIdx) +  ", " + std::to_string(m_preSample)));

  if (m_useDelayed) {
    (*m_msgFunc_p)(ZDCMsg::Info, ("using delayed samples with delta T = " + std::to_string(m_delayedDeltaT) + ", and pedestalDiff == " +
                                  std::to_string(m_delayedPedestalDiff)));
  }

  std::ostringstream message1;
  message1 << "samplesSub ";
  for (size_t sample = 0; sample < m_samplesSub.size(); sample++) {
    message1 << ", [" << sample << "] = " << m_samplesSub[sample];
  }
  (*m_msgFunc_p)(ZDCMsg::Info, message1.str());

  std::ostringstream message3;
  message3 << "samplesDeriv2nd ";
  for (size_t sample = 0; sample < m_samplesDeriv2nd.size(); sample++) {
    message3 << ", [" << sample << "] = " << m_samplesDeriv2nd[sample];
  }
  (*m_msgFunc_p)(ZDCMsg::Info, message3.str());

  (*m_msgFunc_p)(ZDCMsg::Info, ("minimum 2nd deriv sample, value = " + std::to_string(m_minDeriv2ndIndex) + ", " + std::to_string(m_minDeriv2nd)));
}

void ZDCPulseAnalyzer::dumpTF1(const TF1* func) const
{
  std::string message = "Dump of TF1: " + std::string(func->GetName());
  bool continueDump = (*m_msgFunc_p)(ZDCMsg::Verbose, std::move(message));
  if (!continueDump) return;
  
  unsigned int npar = func->GetNpar();
  for (unsigned int ipar = 0; ipar < npar; ipar++) {
    std::ostringstream msgstr;

    double parMin = 0, parMax = 0;
    func->GetParLimits(ipar, parMin, parMax);
		       
    msgstr << "Parameter " << ipar << ", value = " << func->GetParameter(ipar) << ", error = "
	   << func->GetParError(ipar) << ", min = " << parMin << ", max = " << parMax;
    (*m_msgFunc_p)(ZDCMsg::Verbose, msgstr.str());
  }
}

void ZDCPulseAnalyzer::dumpConfiguration() const    // setting
{
  std::ostringstream ostrStream;
  
  (*m_msgFunc_p)(ZDCMsg::Info, ("ZDCPulserAnalyzer:: settings for instance: " + m_tag));

  ostrStream << "Nsample = " << m_Nsample << " at frequency " << m_freqMHz << " MHz, preSample index = "
	     << m_preSampleIdx <<  ", nominal pedestal = " << m_pedestal;
    
  (*m_msgFunc_p)(ZDCMsg::Info, ostrStream.str()); ostrStream.str(""); ostrStream.clear();

  ostrStream << "LG mode = " << m_LGMode << ", gainFactor HG = " << m_gainFactorHG << ", gainFactor LG = " << m_gainFactorLG << ", noise sigma HG = " <<  m_noiseSigHG << ", noiseSigLG = " << m_noiseSigLG;
  (*m_msgFunc_p)(ZDCMsg::Info, ostrStream.str()); ostrStream.str(""); ostrStream.clear();
  
  ostrStream << "peak sample = " << m_peak2ndDerivMinSample <<  ", tolerance = " << m_peak2ndDerivMinTolerance
	     << ", 2ndDerivThresh HG, LG = " << m_peak2ndDerivMinThreshHG <<  ", " << m_peak2ndDerivMinThreshLG
	     << ", 2nd deriv step = " << m_2ndDerivStep;
  (*m_msgFunc_p)(ZDCMsg::Info, ostrStream.str()); ostrStream.str(""); ostrStream.clear();

  if (m_useDelayed) {
    ostrStream << "using delayed samples with delta T = " << m_delayedDeltaT << ", and default pedestalDiff == "
	       << m_delayedPedestalDiff;
    (*m_msgFunc_p)(ZDCMsg::Info, ostrStream.str()); ostrStream.str(""); ostrStream.clear();
  }

  ostrStream << "m_fixTau1 = " << m_fixTau1 <<  ", m_fixTau2 = " << m_fixTau2 << ", m_nominalTau1 = " << m_nominalTau1
	     << ", m_nominalTau2 = " << m_nominalTau2 <<  ", m_nominalT0HG = " << m_nominalT0HG
	     << ",  m_nominalT0LG = " << m_nominalT0LG;
  (*m_msgFunc_p)(ZDCMsg::Info, ostrStream.str()); ostrStream.str(""); ostrStream.clear();

  ostrStream << "m_HGOverflowADC = " << m_HGOverflowADC << ",  m_HGUnderflowADC = " << m_HGUnderflowADC
	     << ",  m_LGOverflowADC = "<< m_LGOverflowADC;
  (*m_msgFunc_p)(ZDCMsg::Info, ostrStream.str()); ostrStream.str(""); ostrStream.clear();

  ostrStream << "m_chisqDivAmpCutLG = " << m_chisqDivAmpCutLG << ",  m_chisqDivAmpCutHG=" << m_chisqDivAmpCutHG
	     << ", t0Cuts HG = [" << m_T0CutLowHG << ", " << m_T0CutHighHG << "], t0Cuts LG = ["
	     << m_T0CutLowLG << ", " << m_T0CutHighLG << "]";
  (*m_msgFunc_p)(ZDCMsg::Info, ostrStream.str()); ostrStream.str(""); ostrStream.clear();

  if ( m_enableRepass) {
    ostrStream << "Repass enabled with peak2ndDerivMinRepassHG = " << m_peak2ndDerivMinRepassHG << ", peak2ndDerivMinRepassLG = " << m_peak2ndDerivMinRepassLG;
    (*m_msgFunc_p)(ZDCMsg::Info, ostrStream.str()); ostrStream.str(""); ostrStream.clear();
  }

  if (m_enablePreExcl) {
    ostrStream << "Pre-exclusion enabled for up to " << m_maxSamplesPreExcl << ", samples with ADC threshold HG = "
	       << m_preExclHGADCThresh << ", LG = " << m_preExclLGADCThresh;
    (*m_msgFunc_p)(ZDCMsg::Info, ostrStream.str()); ostrStream.str(""); ostrStream.clear();
  }
  if (m_enablePostExcl) {
    ostrStream << "Post-exclusion enabled for up to " << m_maxSamplesPostExcl << ", samples with ADC threshold HG = "
	       << m_postExclHGADCThresh << ", LG = " << m_postExclLGADCThresh;
    (*m_msgFunc_p)(ZDCMsg::Info, ostrStream.str()); ostrStream.str(""); ostrStream.clear();
  }

}

unsigned int ZDCPulseAnalyzer::GetStatusMask() const
{
  unsigned int statusMask = 0;

  if (havePulse())  statusMask |= 1 << PulseBit;
  if (useLowGain()) statusMask |= 1 << LowGainBit;
  if (failed())     statusMask |= 1 << FailBit;
  if (HGOverflow()) statusMask |= 1 << HGOverflowBit;

  if (HGUnderflow())       statusMask |= 1 << HGUnderflowBit;
  if (PSHGOverUnderflow()) statusMask |= 1 << PSHGOverUnderflowBit;
  if (LGOverflow())        statusMask |= 1 << LGOverflowBit;
  if (LGUnderflow())       statusMask |= 1 << LGUnderflowBit;

  if (prePulse())  statusMask |= 1 << PrePulseBit;
  if (postPulse()) statusMask |= 1 << PostPulseBit;
  if (fitFailed()) statusMask |= 1 << FitFailedBit;
  if (badChisq())  statusMask |= 1 << BadChisqBit;

  if (badT0())          statusMask |= 1 << BadT0Bit;
  if (excludeEarlyLG()) statusMask |= 1 << ExcludeEarlyLGBit;
  if (excludeLateLG())  statusMask |= 1 << ExcludeLateLGBit;
  if (preExpTail())     statusMask |= 1 << preExpTailBit;
  if (fitMinimumAmplitude()) statusMask |= 1 << FitMinAmpBit;
  if (repassPulse()) statusMask |= 1 << RepassPulseBit;
  if (armSumInclude()) statusMask |= 1 << ArmSumIncludeBit;
  if (failSigCut()) statusMask |= 1 << FailSigCutBit;

  return statusMask;
}

std::shared_ptr<TGraphErrors> ZDCPulseAnalyzer::GetCombinedGraph(bool LGRefit)
{
  //
  // We defer filling the histogram if we don't have a pulse until the histogram is requested
  //
  GetHistogramPtr(LGRefit);

  TH1* hist_p = nullptr, *delayedHist_p = nullptr;
  if (LGRefit) {
    hist_p = m_fitHistLGRefit.get();
    delayedHist_p = m_delayedHistLGRefit.get();
  }
  else {
    hist_p = m_fitHist.get();
    delayedHist_p = m_delayedHist.get();
  }
  
  std::shared_ptr<TGraphErrors> theGraph = std::make_shared<TGraphErrors>(TGraphErrors(2 * m_Nsample));
  size_t npts = 0;

  for (int ipt = 0; ipt < hist_p->GetNbinsX(); ipt++) {
    theGraph->SetPoint(npts, hist_p->GetBinCenter(ipt + 1), hist_p->GetBinContent(ipt + 1));
    theGraph->SetPointError(npts++, 0, hist_p->GetBinError(ipt + 1));
  }

  for (int iDelayPt = 0; iDelayPt < delayedHist_p->GetNbinsX(); iDelayPt++) {
    theGraph->SetPoint(npts, delayedHist_p->GetBinCenter(iDelayPt + 1), delayedHist_p->GetBinContent(iDelayPt + 1) - m_delayedBaselineShift);
    theGraph->SetPointError(npts++, 0, delayedHist_p->GetBinError(iDelayPt + 1));
  }
  if (m_havePulse) {
    TF1* func_p = static_cast<TF1*>(hist_p->GetListOfFunctions()->Last());
    if (func_p) {
      theGraph->GetListOfFunctions()->Add(new TF1(*func_p));
      hist_p->GetListOfFunctions()->SetOwner (false);
    }
  }
  theGraph->SetName(( std::string(hist_p->GetName()) + "combinaed").c_str());

  theGraph->SetMarkerStyle(20);
  theGraph->SetMarkerColor(1);

  return theGraph;
}


std::shared_ptr<TGraphErrors> ZDCPulseAnalyzer::GetGraph(bool forceLG)
{
  //
  // We defer filling the histogram if we don't have a pulse until the histogram is requested
  //
  const TH1* hist_p = GetHistogramPtr(forceLG);

  std::shared_ptr<TGraphErrors> theGraph = std::make_shared<TGraphErrors>(TGraphErrors(m_Nsample));
  size_t npts = 0;

  for (int ipt = 0; ipt < hist_p->GetNbinsX(); ipt++) {
    theGraph->SetPoint(npts, hist_p->GetBinCenter(ipt + 1), hist_p->GetBinContent(ipt + 1));
    theGraph->SetPointError(npts++, 0, hist_p->GetBinError(ipt + 1));
  }

  TF1* func_p = static_cast<TF1*>(hist_p->GetListOfFunctions()->Last());
  theGraph->GetListOfFunctions()->Add(func_p);
  theGraph->SetName(( std::string(hist_p->GetName()) + "not_combinaed").c_str());

  theGraph->SetMarkerStyle(20);
  theGraph->SetMarkerColor(1);

  return theGraph;
}


std::vector<float> ZDCPulseAnalyzer::CalculateDerivative(const std::vector <float>& inputData, unsigned int step)
{
  unsigned int nSamples = inputData.size();

  // So we pad at the beginning and end based on step (i.e. with step - 1 zeros). Fill out the fill vector with zeros initially
  //
  unsigned int vecSize = 2*(step - 1) + nSamples - step - 1;
  std::vector<float> results(vecSize, 0);
 
  // Now fill out the values
  //
  unsigned int fillIdx = step - 1;
  
  for (unsigned int sample = 0; sample < nSamples - step; sample++) {
    int deriv = inputData[sample + step] - inputData[sample];
    results.at(fillIdx++) = deriv;
  }

  return results;
}

std::vector<float> ZDCPulseAnalyzer::Calculate2ndDerivative(const std::vector <float>& inputData, unsigned int step)
{
  unsigned int nSamples = inputData.size();

  // We start with two zero entries for which we can't calculate the double-step derivative
  //   and would pad with two zero entries at the end. Start by initializing 
  //
  unsigned int vecSize = 2*step + nSamples - step - 1;
  std::vector<float> results(vecSize, 0);

  unsigned int fillIndex = step;
  for (unsigned int sample = step; sample < nSamples - step; sample++) {
    int deriv2nd = inputData[sample + step] + inputData[sample - step] - 2*inputData[sample];
    results.at(fillIndex++) = deriv2nd;
  }

  return results;
}

// Implement a more general method for the nasty problem (Runs 1 & 2 only) of matching
//   the baselines on the delayed and undelayed data. Instead of specifically looking
//   for early samples or late samples, find the region of the waveform with the smallest
//   combination of slope and second derivative in both sets of samples and match there.
//
// Depending on the second derivative 
//   we match using linear or (negative) exponential interpolation
//
// Note: the samples have already been combined so we distinguish by even or odd index
//  we use step = 2 for the derivative and 2nd derivative calculation to handle
//  the delayed and undelayed separately.
//
//
float ZDCPulseAnalyzer::obtainDelayedBaselineCorr(const std::vector<float>& samples)
{
  const unsigned int nsamples = samples.size();
  
  std::vector<float> derivVec = CalculateDerivative(samples, 2);
  std::vector<float> deriv2ndVec = Calculate2ndDerivative(samples, 2);

  // Now step through and check even and odd samples values for 2nd derivative and derivative 
  //  we start with index 2 since the 2nd derivative calculation has 2 initial zeros with nstep = 2
  //
  float minScore = 1.0e9;
  unsigned int minIndex = 0;
  
  for (unsigned int idx = 2; idx < nsamples - 1; idx++) {
    float deriv = derivVec[idx];
    float prevDeriv = derivVec[idx - 1];

    float derivDiff = deriv - prevDeriv;
    
    float deriv2nd = deriv2ndVec[idx];
    if (idx > nsamples - 2) deriv2nd = deriv2ndVec[idx - 1];
    
    // Calculate a score based on the actual derivatives (squared) and 2nd derivatives (squared)
    //   and the slope differences (squared). The relative weights are not adjustable for now
    //
    float score = (deriv*deriv + 2*derivDiff*derivDiff +
		   0.5*deriv2nd*deriv2nd);

    if (score < minScore) {
      minScore = score;
      minIndex = idx;
    }
  }

  // We use four samples, two each of "even" and "odd".
  // Because of the way the above analysis is done, we can always
  // Go back one even and one odd sample and forward one odd sample.
  //
  //if minIndex is < 2 or >samples.size() the result is undefined; prevent this:
  if (minIndex<2 or (minIndex+1) >=nsamples){
    throw std::out_of_range("minIndex out of range in ZDCPulseAnalyzer::obtainDelayedBaselineCorr");
  }
  float sample0 = samples[minIndex - 2];
  float sample1 = samples[minIndex - 1];
  float sample2 = samples[minIndex];
  float sample3 = samples[minIndex + 1];

  // Possibility -- implement logarithmic interpolation for large 2nd derivative?
  //
  float baselineCorr = (0.5 * (sample1 - sample0 + sample3 - sample2) -
			0.25 * (sample3 - sample1 + sample2 - sample0));

  if (minIndex % 2 != 0) baselineCorr =-baselineCorr;

  return baselineCorr;
}

std::pair<bool, std::string> ZDCPulseAnalyzer::ValidateJSONConfig(const JSON& config)
{
  bool result = true;
  std::string resultString = "success";

  for (auto [key, descr] : JSONConfigParams) {
    auto iter = config.find(key);
    if (iter != config.end()) {
      //
      // Check type consistency
      //
      auto jsonType = iter.value().type();
      auto paramType = std::get<0>(descr);
      if (jsonType != paramType) {
	result = false;
	resultString = "Bad type for parameter " + key + ", type in JSON = " + std::to_string((unsigned int) jsonType) ;
	break;
      }
      
      size_t paramSize = std::get<1>(descr);
      size_t jsonSize = iter.value().size();
      if (jsonSize != paramSize) {
	result = false;
	resultString = "Bad length for parameter " + key + ", length in JSON = " + std::to_string(jsonSize) ;
	break;
      }
    }
    else {
      bool required = std::get<2>(descr);
      if (required) {
	result = false;
	resultString = "Missing required parameter " + key;
	break;
      }
    }
  }

  if (result) {
    //
    // Now check that the parameters in the JSON object are in the master list of parameters 
    //
    for (auto [key, value] : config.items()) {
      //
      // Look for this key in the list of allowed parameters. Yes, it's a slow 
      //   search but we only do it once at configuration time.
      //
      // bool found = false;
      auto iter = JSONConfigParams.find(key);
      if (iter == JSONConfigParams.end()) {
	result = false;
	resultString = "Unknown parameter, key = " + key;
	break;
      }
    }
  }
  
  return {result, resultString};
}

std::pair<bool, std::string> ZDCPulseAnalyzer::ConfigFromJSON(const JSON& config)
{
  bool result = true;
  std::string resultString = "success";

  for (auto [key, value] : config.items()) {
    //
    // Big if statement to process configuration parameters
    //
    if (key == "Nsample") m_Nsample = value;
    else if (key == "tag") m_tag = value;
    else if (key == "LGMode") m_LGMode = value;
    else if (key == "Nsample") m_Nsample = value;
    else if (key == "useDelayed") m_useDelayed = value;
    else if (key == "preSampleIdx") m_preSampleIdx = value;
    else if (key == "FADCFreqMHz") m_freqMHz = value;
    else if (key == "nominalPedestal") m_pedestal = value;
    else if (key == "fitFunction") m_fitFunction = value;
    else if (key == "peakSample") m_peak2ndDerivMinSample = value;
    else if (key == "peakTolerance") m_peak2ndDerivMinTolerance = value;
    else if (key == "2ndDerivThreshHG") m_peak2ndDerivMinThreshHG = value;
    else if (key == "2ndDerivThreshLG") m_peak2ndDerivMinThreshLG = value;
    else if (key == "2ndDerivStep") m_2ndDerivStep = value;
    else if (key == "HGOverflowADC") m_HGOverflowADC = value;
    else if (key == "HGUnderflowADC") m_HGUnderflowADC = value;
    else if (key == "LGOverflowADC") m_LGOverflowADC = value;
    else if (key == "nominalT0HG") m_nominalT0HG = value;
    else if (key == "nominalT0LG") m_nominalT0LG = value;
    else if (key == "nominalTau1") m_nominalTau1 = value;
    else if (key == "nominalTau2") m_nominalTau2 = value;
    else if (key == "fixTau1") m_fixTau1 = value;
    else if (key == "fixTau2") m_fixTau2 = value;
    else if (key == "T0CutsHG") {
      m_T0CutLowHG = value[0];
      m_T0CutHighHG = value[1];
    }
    else if (key == "T0CutsLG") {
      m_T0CutLowLG = value[0];
      m_T0CutHighLG = value[1];
    }
    else if (key == "chisqDivAmpCutHG") m_chisqDivAmpCutHG = value;
    else if (key == "chisqDivAmpCutLG") m_chisqDivAmpCutLG = value;
    else if (key == "gainFactorHG") m_gainFactorHG = value;
    else if (key == "gainFactorLG") m_gainFactorLG = value;
    else if (key == "noiseSigmaHG") m_noiseSigHG = value;
    else if (key == "noiseSigmaLG") m_noiseSigLG = value;
    else if (key == "enableRepass") m_enableRepass = value;
    else if (key == "Repass2ndDerivThreshHG")  m_peak2ndDerivMinRepassHG = value;
    else if (key == "Repass2ndDerivThreshLG")  m_peak2ndDerivMinRepassLG = value;
    else if (key == "fitAmpMinMaxHG") {
      m_fitAmpMinHG = value[0];
      m_fitAmpMaxHG = value[1];
    }
    else if (key == "fitAmpMinMaxLG") {
      m_fitAmpMinLG = value[0];
      m_fitAmpMaxLG = value[1];
    }
    else if (key == "quietFits") {
      m_quietFits = value[0];
    }
    else if (key == "enablePreExclusion") {
      m_enablePreExcl = true;
      m_maxSamplesPreExcl  = value[0];
      m_preExclHGADCThresh = value[1];
      m_preExclLGADCThresh = value[2];
    }
    else if (key == "enablePostExclusion") {
      m_enablePostExcl = true;
      m_maxSamplesPostExcl  = value[0];
      m_postExclHGADCThresh = value[1];
      m_postExclLGADCThresh = value[2];
    }
    else if (key == "ampMinSignifHGLG") {
      m_haveSignifCuts = true;
      m_sigMinHG = value[0];
      m_sigMinLG = value[1];
    }
    else if (key == "enableFADCCorrections") {
      auto fileNameJson = value["filename"];
      auto doPerSampleCorrJson = value["doPerSampleCorr"];
      
      if (fileNameJson.is_null() || doPerSampleCorrJson.is_null()) {
	result = false;
	std::string resultString = "failure processing enableFADCCorrections object";
	break;
      }
      
      m_haveFADCCorrections = true;
      m_FADCCorrPerSample = doPerSampleCorrJson;
      m_fadcCorrFileName = fileNameJson;
    }
    else {
      result = false;
      std::string resultString = "unprocessed parameter";
      break;
    }
  }

  return {result, resultString};
}
