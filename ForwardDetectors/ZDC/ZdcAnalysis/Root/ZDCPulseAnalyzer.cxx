/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "ZdcAnalysis/ZDCPulseAnalyzer.h"

#include "TFitResult.h"
#include "TFitResultPtr.h"
#include "TVirtualFitter.h"
#include "TFitter.h"
#include "TList.h"
#include "TMinuit.h"

#include <algorithm>
#include <sstream>
#include <cmath>
#include <memory>
#include <numeric>

#include "CxxUtils/checker_macros.h"
ATLAS_NO_CHECK_FILE_THREAD_SAFETY;

extern int gErrorIgnoreLevel;

bool ZDCPulseAnalyzer::s_quietFits         = true;
bool ZDCPulseAnalyzer::s_saveFitFunc       = false;
std::string ZDCPulseAnalyzer::s_fitOptions = "";
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
                                   float gainHG, const std::string& fitFunction, int peak2ndDerivMinSample,
                                   float peak2ndDerivMinThreshHG, float peak2ndDerivMinThreshLG) :
  m_msgFunc_p(msgFunc_p),
  m_tag(tag), m_Nsample(Nsample),
  m_preSampleIdx(preSampleIdx),
  m_deltaTSample(deltaTSample),
  m_pedestal(pedestal), m_gainHG(gainHG), m_forceLG(false), m_fitFunction(fitFunction),
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

  std::string histName = "ZDCFitHist" + tag;

  m_fitHist = std::make_unique<TH1F>(histName.c_str(), "", m_Nsample, m_tmin, m_tmax);
  m_fitHist->SetDirectory(0);

  SetDefaults();
  Reset();
}


void ZDCPulseAnalyzer::enableDelayed(float deltaT, float pedestalShift, bool fixedBaseline)
{
  m_useDelayed = true;
  m_useFixedBaseline = fixedBaseline;

  m_delayedDeltaT = deltaT;
  m_delayedPedestalDiff = pedestalShift;

  m_deltaTSample /= 2.;

  m_delayedHist = std::make_unique<TH1F>((std::string(m_fitHist->GetName()) + "delayed").c_str(), "", m_Nsample, m_tmin + m_delayedDeltaT, m_tmax + m_delayedDeltaT);
  m_delayedHist->SetDirectory(0);

  m_ADCSamplesHGSub.assign(2 * m_Nsample, 0);
}

void ZDCPulseAnalyzer::enableRepass(float peak2ndDerivMinRepassHG, float peak2ndDerivMinRepassLG)
{
  m_enableRepass = true;
  m_peak2ndDerivMinRepassHG = peak2ndDerivMinRepassHG;
  m_peak2ndDerivMinRepassLG = peak2ndDerivMinRepassLG;
}

void ZDCPulseAnalyzer::SetDefaults()
{
  m_nominalTau1 = 4;
  m_nominalTau2 = 21;

  m_fixTau1 = false;
  m_fixTau2 = false;

  m_HGOverflowADC  = 900;
  m_HGUnderflowADC = 20;
  m_LGOverflowADC  = 1000;

  // Default values for the gain factors uswed to match low and high gain
  //
  m_gainFactorLG = m_gainHG;
  m_gainFactorHG = 1;

  m_2ndDerivStep = 1;

  m_noiseSigHG = 1;
  m_noiseSigLG = 1;

  m_chisqDivAmpCutLG = 100;
  m_chisqDivAmpCutHG = 100;

  m_T0CutLowLG = m_tmin;
  m_T0CutLowHG = m_tmin;

  m_T0CutHighLG = m_tmax;
  m_T0CutHighHG = m_tmax;

  m_LGT0CorrParams.assign(4, 0);
  m_HGT0CorrParams.assign(4, 0);

  m_defaultFitTMax = m_tmax;
  m_defaultFitTMin = m_tmin;
  
  m_fitAmpMinHG = 5;
  m_fitAmpMinLG = 2;

  m_fitAmpMaxHG = 1500;
  m_fitAmpMaxLG = 1500;

  m_postPulse = false;
  m_prePulse = false;

  m_initialPrePulseT0  = -10;
  m_initialPrePulseAmp = 5;

  m_initialPostPulseT0  = 100;

  m_initialExpAmp = 0;
  m_fitPostT0lo   = 0;

  s_fitOptions = "s";
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

    int sampleVecSize = m_Nsample;
    if (m_useDelayed) sampleVecSize *= 2;

    m_ADCSamplesHGSub.assign(sampleVecSize, 0);
    m_ADCSamplesLGSub.assign(sampleVecSize, 0);

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

  // -----------------------

  m_delayedBaselineShift = 0;

  m_fitAmplitude = 0;
  m_fitTime      = -100;
  m_fitTimeSub   = -100;
  m_fitTimeCorr  = -100;
  m_fitTCorr2nd  = -100;

  m_fitPreT0   = -100;
  m_fitPreAmp  = -100;
  m_fitPostT0  = -100;
  m_fitPostAmp = -100;
  m_fitExpAmp  = -100;

  m_fitChisq = 0;

  m_amplitude       = 0;
  m_ampError        = 0;
  m_preSampleAmp    = 0;
  m_preAmplitude    = 0;
  m_postAmplitude   = 0;
  m_expAmplitude    = 0;
  m_bkgdMaxFraction = 0;


  m_initialPrePulseT0  = -10;
  m_initialPrePulseAmp = 5;

  m_initialPostPulseT0  = 100;

  m_initialExpAmp = 0;
  m_fitPostT0lo   = 0;

  m_fitPulls.clear();
  
  m_samplesSub.clear();
  m_samplesDeriv2nd.clear();
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

  (*m_msgFunc_p)(ZDCMsg::Verbose, ("Setting FitTMax to " + std::to_string(m_defaultFitTMax)));

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

std::vector<float>  ZDCPulseAnalyzer::GetFitPulls() const
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
    
    const TH1* dataHist_p = m_fitHist.get();
    const TF1* fit_p = (const TF1*) dataHist_p->GetListOfFunctions()->Last();
    
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

    m_prePulseFitWrapper = std::unique_ptr<ZDCPrePulseFitWrapper>(new ZDCFitExpFermiPrePulse(m_tag, m_tmin, m_tmax, m_nominalTau1, m_nominalTau2));
  }
  else if (m_fitFunction == "FermiExpLHCf") {
    //if (!m_fixTau1 || !m_fixTau2) {
      //
      // Use the variable tau version of the expFermiFit
      //
      m_defaultFitWrapper = std::unique_ptr<ZDCFitWrapper>(new ZDCFitExpFermiVariableTausLHCf(m_tag, m_tmin, m_tmax, m_fixTau1, m_fixTau2, m_nominalTau1, m_nominalTau2));
    // }
    // else {
    //   m_defaultFitWrapper = std::unique_ptr<ZDCFitWrapper>(new ZDCFitExpFermiFixedTaus(m_tag, m_tmin, m_tmax, m_nominalTau1, m_nominalTau2));
    // }

    m_prePulseFitWrapper = std::unique_ptr<ZDCPrePulseFitWrapper>(new ZDCFitExpFermiPrePulse(m_tag, m_tmin, m_tmax, m_nominalTau1, m_nominalTau2));
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


  // Now do pedestal subtraction and check for overflows
  //
  for (size_t isample = 0; isample < m_Nsample; isample++) {
    float ADCHG = ADCSamplesHG[isample];
    float ADCLG = ADCSamplesLG[isample];

    if (ADCHG > m_HGOverflowADC) {
      m_HGOverflow = true;

      if (isample == m_preSampleIdx) m_PSHGOverUnderflow = true;
      if ((int) isample > m_lastHGOverFlowSample)  m_lastHGOverFlowSample  = isample;
      if ((int) isample < m_firstHGOverFlowSample) m_firstHGOverFlowSample = isample;
    }
    else if (ADCHG < m_HGUnderflowADC) {
      m_HGUnderflow = true;

      if (isample == m_preSampleIdx) m_PSHGOverUnderflow  = true;
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

    m_ADCSamplesHGSub[isample] = ADCHG - m_pedestal;
    m_ADCSamplesLGSub[isample] = ADCLG - m_pedestal;
  }

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
    else if (m_firstHGOverFlowSample < static_cast<int>(m_Nsample) && m_firstHGOverFlowSample >= static_cast<int>(m_Nsample - 2) ) {
      m_maxSampleEvt = m_firstHGOverFlowSample - 1;
      m_HGOverflow = false;
      m_adjTimeRangeEvent = true;
      m_ExcludeLate = true;
    }
  }
  
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

  // Now do pedestal subtraction and check for overflows
  //
  for (size_t isample = 0; isample < m_Nsample; isample++) {
    float ADCHG = ADCSamplesHG[isample];
    float ADCLG = ADCSamplesLG[isample];

    float ADCHGDelay = ADCSamplesHGDelayed[isample];
    float ADCLGDelay = ADCSamplesLGDelayed[isample];

    int undelSampIdx = 2 * isample;
    int delSampIdx   = 2 * isample + 1;

    if (ADCHG > m_HGOverflowADC) {
      m_HGOverflow = true;

      if (isample == m_preSampleIdx) m_PSHGOverUnderflow = true;
      if (undelSampIdx > m_lastHGOverFlowSample)  m_lastHGOverFlowSample  = undelSampIdx;
      if (undelSampIdx < m_firstHGOverFlowSample) m_firstHGOverFlowSample = undelSampIdx;
    }
    else if (ADCHG < m_HGUnderflowADC) {
      m_HGUnderflow = true;

      if (isample == m_preSampleIdx) m_PSHGOverUnderflow  = true;
    }

    if (ADCHGDelay > m_HGOverflowADC) {
      if (delSampIdx > m_lastHGOverFlowSample)  m_lastHGOverFlowSample  = delSampIdx;
      if (delSampIdx < m_firstHGOverFlowSample) m_firstHGOverFlowSample = delSampIdx;

      m_HGOverflow = true;
    }
    else if (ADCHGDelay < m_HGUnderflowADC) {
      m_HGUnderflow = true;
    }

    if (ADCLG > m_LGOverflowADC) {
      m_LGOverflow = true;
      m_fail       = true;
      m_amplitude  = m_LGOverflowADC * m_gainFactorLG; // Give a value here even though we know it's wrong because
      //   the user may not check the return value and we know that
      //   amplitude is bigger than this
    }
    else if (ADCLG == 0) {
      m_LGUnderflow = true;
      m_fail        = true;
    }

    if (ADCLGDelay > m_LGOverflowADC) {
      m_LGOverflow = true;
      m_fail       = true;
      m_amplitude  = 1024 * m_gainFactorLG; // Give a value here even though we know it's wrong because
      //   the user may not check the return value and we know that
      //   amplitude is bigger than this
    }
    else if (ADCLGDelay == 0) {
      m_LGUnderflow = true;
      m_fail        = true;
    }

    m_ADCSamplesHGSub[isample * 2] = ADCHG - m_pedestal;
    m_ADCSamplesLGSub[isample * 2] = ADCLG - m_pedestal;

    m_ADCSamplesHGSub[isample * 2 + 1] = ADCHGDelay - m_pedestal - m_delayedPedestalDiff;
    m_ADCSamplesLGSub[isample * 2 + 1] = ADCLGDelay - m_pedestal - m_delayedPedestalDiff;
  }

  // ------------------------------------------------------
  // Determine whether we can avoid the use of low gain by excluding early or late samples
  //
  if (m_HGOverflow) {
    if (m_lastHGOverFlowSample < 2 && m_lastHGOverFlowSample > -1) {
      m_HGOverflow = false;
      m_minSampleEvt = m_lastHGOverFlowSample + 1;
      m_adjTimeRangeEvent = true;
      m_backToHG_pre = true;
      m_ExcludeEarly = true;
    }
    else {
      //
      // If there is a larger pre-pulse on low-gain, exclude the first two points from subsequent
      //   analysis as they appear to cause chisq cut failures
      //
      if (m_ADCSamplesLGSub[0] > 100) {
        m_minSampleEvt = 2;
        m_adjTimeRangeEvent = true;
        m_ExcludeEarly = true;
      }
    }
  }
  else {
    //
    // Test to see whether one or two samples just before we measure could have overflowed
    //
    float backExtrapolate = m_ADCSamplesHGSub[0] + m_pedestal + 2 * (m_ADCSamplesHGSub[0] - m_ADCSamplesHGSub[1]);
    if (backExtrapolate > m_HGOverflowADC) {
      m_minSampleEvt = 1;
      m_adjTimeRangeEvent = true;
      m_ExcludeEarly = true;

      m_fixPrePulse = true; // new 2020/01/27
    }
  }
  if (m_firstHGOverFlowSample < static_cast<int>(m_Nsample) && m_firstHGOverFlowSample >= static_cast<int>(2 * m_Nsample - 3 )   ) {
    m_maxSampleEvt = m_firstHGOverFlowSample - 1;
    m_HGOverflow = false;
    m_adjTimeRangeEvent = true;
    m_ExcludeLate = true;
  }
  // ------------------------------------------------------

  return DoAnalysis(false);
}

bool ZDCPulseAnalyzer::ReanalyzeData()
{
  Reset(true);

  bool result = DoAnalysis(true);
  if (result && HavePulse()) {
    m_repassPulse = true;
  }

  return result;
}

bool ZDCPulseAnalyzer::DoAnalysis(bool repass)
{
  int nSampleScale = m_useDelayed ? 2 : 1;

  float deriv2ndThreshHG = 0;
  float deriv2ndThreshLG = 0;

  if (!repass) {
    deriv2ndThreshHG = m_peak2ndDerivMinThreshHG;
    deriv2ndThreshLG = m_peak2ndDerivMinThreshLG;
  }
  else {
    deriv2ndThreshHG = m_peak2ndDerivMinRepassHG;
    deriv2ndThreshLG = m_peak2ndDerivMinRepassLG;
  }

  m_useLowGain = m_HGUnderflow || m_HGOverflow || m_forceLG;
  if (m_useLowGain) {
    bool result = AnalyzeData(m_Nsample * nSampleScale, m_preSampleIdx, m_ADCSamplesLGSub, m_noiseSigLG, m_LGT0CorrParams, m_chisqDivAmpCutLG,
                              m_T0CutLowLG, m_T0CutHighLG, deriv2ndThreshLG);
    if (result) {
      if (BadChisq()) {
        if (!m_ExcludeEarly && m_fitPreAmp > 2 * m_fitAmplitude) {
          m_adjTimeRangeEvent = true;
          m_ExcludeEarly = true;
          m_minSampleEvt = 1;

          Reset(true);
          result = AnalyzeData(m_Nsample * nSampleScale, m_preSampleIdx, m_ADCSamplesLGSub, m_noiseSigLG, m_LGT0CorrParams,
                               m_chisqDivAmpCutLG, m_T0CutLowLG, m_T0CutHighLG, deriv2ndThreshLG);
        }
      }

      //  If we have a non-linear correction, apply it here
      //
      //    We apply it as an inverse correction - i.e. we divide by a correction
      //      term tha is a sum of coefficients times the ADC minus a reference
      //      to a power. The lowest power is 1, the highest is deteremined by
      //      the number of provided coefficients 
      //
      if (m_haveNonlinCorr) {
        float ampCorrFact = (m_fitAmplitude - m_nonLinCorrRefADC) / 1000. ;
	
	float invNLCorr = 1.0;
	for (size_t power = 1; power <= m_nonLinCorrParams.size(); power++) {
	  invNLCorr += m_nonLinCorrParams[power - 1]*pow(ampCorrFact, power);
	}

        m_fitAmplitude /= invNLCorr;
        m_fitAmpError /= invNLCorr;
      }
      
      //
      // Multiply amplitude by gain factor
      //
      m_amplitude     = m_fitAmplitude * m_gainFactorLG;
      m_ampError      = m_fitAmpError  * m_gainFactorLG;
      m_preSampleAmp  = m_preSample    * m_gainFactorLG;
      m_preAmplitude  = m_fitPreAmp    * m_gainFactorLG;
      m_postAmplitude = m_fitPostAmp   * m_gainFactorLG;
      m_expAmplitude  = m_fitExpAmp    * m_gainFactorLG;

      // BAC: also scale up the 2nd derivative so low and high gain can be treated on the same footing
      //
      m_minDeriv2nd *= m_gainFactorLG;
    }

    return result;
  }
  else {
    bool result = AnalyzeData(m_Nsample * nSampleScale, m_preSampleIdx, m_ADCSamplesHGSub, m_noiseSigHG, m_HGT0CorrParams, m_chisqDivAmpCutHG,
                              m_T0CutLowHG, m_T0CutHighHG, deriv2ndThreshHG);
    if (result) {
      if (BadChisq()) {
        if (!m_ExcludeEarly && m_fitPreAmp > 2 * m_fitAmplitude) {
          m_adjTimeRangeEvent = true;
          m_ExcludeEarly = true;
          m_minSampleEvt = 1;

          Reset(true);
          result = AnalyzeData(m_Nsample * nSampleScale, m_preSampleIdx, m_ADCSamplesHGSub, m_noiseSigHG, m_HGT0CorrParams, m_chisqDivAmpCutHG,
                               m_T0CutLowHG, m_T0CutHighHG, deriv2ndThreshHG);
        }
      }

      m_preSampleAmp = m_preSample  * m_gainFactorHG;
      m_amplitude = m_fitAmplitude  * m_gainFactorHG;
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
      if (m_haveNonlinCorr) {
        float ampCorrFact = (m_amplitude - m_nonLinCorrRefADC) / 1000. ;
	
	float invNLCorr = 1.0;
	for (size_t power = 1; power <= m_nonLinCorrParams.size(); power++) {
	  invNLCorr += m_nonLinCorrParams[power - 1]*pow(ampCorrFact, power);
	}

        m_amplitude /= invNLCorr;
        m_ampError /= invNLCorr;
      }
    }

    return result;
  }
}

bool ZDCPulseAnalyzer::AnalyzeData(size_t nSamples, size_t preSampleIdx,
                                   const std::vector<float>& samples,        // The samples used for this event
				   float noiseSig,
                                   const std::vector<float>& t0CorrParams,   // The parameters used to correct the t0
                                   float maxChisqDivAmp,                     // The maximum chisq / amplitude ratio
                                   float minT0Corr, float maxT0Corr,         // The minimum and maximum corrected T0 values
                                   float peak2ndDerivMinThresh
                                  )
{

  // We keep track of which sample we used to do the subtraction sepaate from m_minSampleEvt
  //  because the corresponding time is the reference time we provide to the fit function when
  //  we have pre-pulses and in case we change the m_minSampleEvt after doing the subtraction
  //
  //    e.g. our refit when the chisquare cuts fails
  //
  m_usedPresampIdx = preSampleIdx;

  if (m_adjTimeRangeEvent) {
    m_usedPresampIdx = m_minSampleEvt; 
  }

  m_preSample = samples[m_usedPresampIdx];

  m_samplesSub = samples;

  if (m_useDelayed) {
    if (m_useFixedBaseline) {
      m_baselineCorr = m_delayedPedestalDiff;
    }
    else {
      //  Attempt to address up front cases where we have significant offsets between the delayed and undelayed
      //

      // Check the slope in the first two samples form delayed and from undelayed
      //
      float slope1 = m_samplesSub[2] - m_samplesSub[0] + 1e-3;
      float slope2 = m_samplesSub[3] - m_samplesSub[1] + 1e-3;
      float slope12Ratio = slope1 / slope2;
      bool badEarly = ((std::abs(slope1) > 5 || std::abs(slope2) > 5) && (slope12Ratio < 0 || std::abs(slope12Ratio - 1) > 1)) ||
                      (std::abs(slope1) > 40 || std::abs(slope2) > 40) ;

      size_t n = m_samplesSub.size();
      float slope3 = m_samplesSub[n - 3] - m_samplesSub[n - 1] + 1e-3;
      float slope4 = m_samplesSub[n - 4] - m_samplesSub[n - 2] + 1e-3;
      float slope34Ratio = slope3 / slope4;
      bool badLate = ((std::abs(slope3) > 5 || std::abs(slope4) > 5) && ( slope34Ratio < 0 || std::abs(slope34Ratio - 1) > 1)) ||
                     (std::abs(slope3) > 20 || std::abs(slope4) > 20);

      int baselineFlag = 0; // default to use nominal pedestal difference

      if (!badEarly && std::abs(slope1 / slope3) < 2) baselineFlag = -1; // use early
      else if (!badLate && (badEarly || std::abs(slope1 / slope3) > 2)) baselineFlag = 1; // use late
      else if (!badEarly) baselineFlag = -1; // use early

      if (baselineFlag < 0) {
        //
        // If we have enough samples to do a proper interpolation do so
        //
        if (m_peak2ndDerivMinSample > 4) {
          if (m_backToHG_pre || m_preSample > 100) {
            m_baselineCorr =  m_samplesSub[3] - exp((log(m_samplesSub[2]) + log(m_samplesSub[4])) * 0.5);
          }
          else {
            m_baselineCorr = (0.5 * (m_samplesSub[1] - m_samplesSub[0] + m_samplesSub[3] - m_samplesSub[2]) - 0.25 * (m_samplesSub[3] - m_samplesSub[1] + m_samplesSub[2] - m_samplesSub[0]));
          }
        }
        else {
          //
          // Otherwise do the simplest thing possible
          //
          m_baselineCorr = m_samplesSub[1] - m_samplesSub[0];
        }
      }
      else if (baselineFlag > 0) {
        //
        // If the slope is large, negative, and none of the baseline-subtracted samples are negative, do exponential interpolation
        //
        if (slope3 < -10 && !(m_samplesSub[n - 3] <= 0 || m_samplesSub[n - 2] <= 0 || m_samplesSub[n - 1] <= 0)) {
          m_baselineCorr =  -m_samplesSub[n - 2] + std::exp((std::log(m_samplesSub[n - 3]) + std::log(m_samplesSub[n - 1])) * 0.5);
        }
        else {
          // Otherwise do linear interpolation
          //
          m_baselineCorr = (0.5 * (m_samplesSub[n - 3] - m_samplesSub[n - 4] + m_samplesSub[n - 1] - m_samplesSub[n - 2]) - 0.25 * (m_samplesSub[n - 1] - m_samplesSub[n - 3] + m_samplesSub[n - 2] - m_samplesSub[n - 4]));
        }
      }
      else {
        m_baselineCorr = m_delayedPedestalDiff;
      }
    }

    // Now apply the baseline correction to align ADC values for delayed and undelayed samples
    //
    for (size_t isample = 0; isample < nSamples; isample++) {
      if (isample % 2) m_samplesSub[isample] -= m_baselineCorr;
    }
  }

  std::for_each(m_samplesSub.begin(), m_samplesSub.end(), [ = ] (float & adcUnsub) {return adcUnsub -= m_preSample;} );

  // Find maximum and minimum values
  //
  int nSkippedSample = 0;
  if (m_useDelayed) nSkippedSample = 4;
  std::pair<SampleCIter, SampleCIter> minMaxIters = std::minmax_element(m_samplesSub.begin() + nSkippedSample, m_samplesSub.end() - nSkippedSample);
  SampleCIter minIter = minMaxIters.first;
  SampleCIter maxIter = minMaxIters.second;

  m_maxADCValue = *maxIter;
  m_minADCValue = *minIter;

  m_maxSampl = std::distance(m_samplesSub.cbegin(), maxIter);
  m_minSampl = std::distance(m_samplesSub.cbegin(), minIter);

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
  m_minDeriv2ndIndex = std::distance(m_samplesDeriv2nd.cbegin(), minDeriv2ndIter);

  // BAC 02-04-23 This check turned out to be problematic. Todo: figure out how to replace
  //
  // // Also check the ADC value for the "peak" sample to make sure it is significant (at least 3 sigma)
  // // The factor of sqrt(2) on the noise is because we have done a pre-sample subtraction
  // //
  if (m_minDeriv2nd <= peak2ndDerivMinThresh) {
    m_havePulse = true;
  }
  else {
    m_havePulse = false;
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
    //
    //  The subtracted ADC value at m_usedPresampIdx is, by construction, zero
    //    The next sample has had the pre-sample subtracted, so it represents the initial derivative
    //
    float derivPresampleSig = m_samplesSub[m_usedPresampIdx+1]/(std::sqrt(2)*noiseSig);
    if (derivPresampleSig < -5) m_preExpTail = true;

    // Separately, if the last sample is significantly below zero after subtraction, it is likely that
    //   the pulse of interest is on the tail of a preceeding pulse
    //
    if (m_samplesSub.back()/(std::sqrt(2)*noiseSig) < -5) m_preExpTail = true;
    
    int loopLimit = (m_havePulse ? m_minDeriv2ndIndex - 2 : m_peak2ndDerivMinSample - 2);
    
    for (int isample = m_usedPresampIdx + 1; isample <= loopLimit; isample++) {
      //
      // If any of the second derivatives prior to the peak are significantly negative, we have a an extra pulse
      //   prior to the main one -- as opposed to just an expnential tail
      //
      if (m_samplesDeriv2nd[isample] < 0.15 * m_minDeriv2nd && m_samplesDeriv2nd[isample]/(std::sqrt(6)*noiseSig) < -4) {
	m_prePulse = true;
	if (m_samplesSub[isample] > m_initialPrePulseAmp) {
	  m_initialPrePulseAmp = m_samplesSub[isample];
	  m_initialPrePulseT0 = m_deltaTSample * (isample);
	}
      }
    }

    if (m_fitFunction == "ComplexPrePulse" || m_fitFunction == "GeneralPulse") {
      if (!m_prePulse) m_fixPrePulse = true;
    }
    
    if (m_preExpTail) {
      m_initialPrePulseT0  = -20;
      m_initialPrePulseAmp = m_samplesSub[m_minSampleEvt];
      
      if (m_fitFunction == "ComplexPrePulse" || m_fitFunction == "GeneralPulse") {
	m_initialPrePulseT0  = m_fitTMin + 5;
	SampleCIter expMinIter = std::min_element(m_samplesSub.begin(), m_samplesSub.begin() + m_peak2ndDerivMinSample);
	m_initialExpAmp = std::abs(*expMinIter - m_samplesSub[0]);
	m_initialPrePulseAmp = 0.1;
      }
    }
  

    if (m_preExpTail) m_prePulse = true;
    
    
    // -----------------------------------------------------
    // Post pulse detection
    //
    if (m_fitFunction == "GeneralPulse") {
      
      for (int isampl = m_minDeriv2ndIndex + 2; isampl < (int) m_samplesDeriv2nd.size(); isampl++) {

	float deriv2ndTest = 0;
	// The place to apply the cut on samples depends on whether we have found a minimum or a maximum
	//   The +1 for the minimum accounts for the shift between 2nd derivative and the samples
	//   if we find a maximum we cut one sample lower
	//
	if (m_samplesDeriv2nd[isampl] > 0 && std::abs(deriv2ndTest) > 1.5) {  // the start of the post pulse, +3 to get at least 3 points into the fit
	  m_postPulse = true;
	  m_maxSampleEvt = std::min(isampl + 1, m_maxSampleEvt);
	  m_fitTMax = m_deltaTSample * (isampl + 3) + m_deltaTSample / 2;
	  m_adjTimeRangeEvent = true;
	  m_initialPostPulseT0 = m_deltaTSample * (isampl + 2);
	  break;
	}
	else if (m_samplesDeriv2nd[isampl] < 0 && std::abs(deriv2ndTest) > 0.5) { // the middle of the post pulse, +2 to get at least 3 points into the fit
	  m_postPulse = true;
	  m_maxSampleEvt = std::min(isampl, m_maxSampleEvt);
	  m_fitTMax = m_deltaTSample * (isampl + 2) + m_deltaTSample / 2;
	  m_adjTimeRangeEvent = true;
	  m_initialPostPulseT0 = m_deltaTSample * (isampl + 1);
	  break;
	}
      }
      
      // Prevent the upper limit of fit range is set to be too low.
      //
      m_fitTMax = std::max((float) 105, m_fitTMax);
      
      // Then make sure it's below default TMax that was given at the beginning
      //
      m_fitTMax = std::min(m_defaultFitTMax, m_fitTMax);
      
      m_fitPostT0lo = m_fitTMax - 2 * m_deltaTSample;
      if (m_fitPostT0lo <= m_deltaTSample * (m_minDeriv2ndIndex + 1)) m_fitPostT0lo = m_deltaTSample * (m_minDeriv2ndIndex + 1) + m_deltaTSample / 2;
    }
    else {
      for (int isampl = m_minDeriv2ndIndex + 2; isampl < (int) m_samplesDeriv2nd.size() - 1; isampl++) {

	// Calculate the forward derivative. the pulse should never increase on the tail. If it
	//   does, we almost certainly have a post-pulse
	//
	float deriv = m_samplesSub[isampl + 1] - m_samplesSub[isampl];
	if (deriv/(std::sqrt(2)*noiseSig) > 6) {
	  m_postPulse = true;
	  m_maxSampleEvt = isampl;
	  m_adjTimeRangeEvent = true;
	  break;
	}
	else {
	  //
	  // Now we check the second derivative which might also indicate a post pulse
	  //   even if the derivative is not sufficiently large
	  //
	  // add small 1e-3 in division to avoid floating overflow
	  //
	  float deriv2ndTest = m_samplesDeriv2nd[isampl] / (m_minDeriv2nd + 1.0e-3);
	  
	  // The place to apply the cut on samples depends on whether we have found a "minimum" or a "maximum"
	  //   The +1 for the minimum accounts for the shift between 2nd derivative and the samples
	  //   if we find a maximum we cut one sample lower
	  //
	  if (m_samplesDeriv2nd[isampl] > 0 && std::abs(deriv2ndTest) > 1.5) {
	    m_postPulse = true;
	    m_maxSampleEvt = std::min(isampl, m_maxSampleEvt);
	    m_adjTimeRangeEvent = true;
	    break;
	  }
	  else if ((m_samplesDeriv2nd[isampl] < 0 && std::abs(deriv2ndTest) > 0.2)) {
	    m_postPulse = true;
	    m_maxSampleEvt = std::min(isampl - 1, m_maxSampleEvt);
	    m_adjTimeRangeEvent = true;
	    break;
	  }
	}
      }
    }
  }

  if (m_postPulse) {
    std::ostringstream ostrm;
    ostrm << "Post pulse found, m_maxSampleEvt = " << m_maxSampleEvt;
    (*m_msgFunc_p)(ZDCMsg::Debug, ostrm.str());
  }

  
  // -----------------------------------------------------

  FillHistogram(m_samplesSub, noiseSig);

  //  Stop now if we have no pulse or we've detected a failure
  //
  if (m_fail || !m_havePulse) return false;

  if (!m_useDelayed) DoFit(maxChisqDivAmp);
  else DoFitCombined(maxChisqDivAmp);
  
  if (FitFailed()) {
    m_fail = true;
  }
  else {
    std::ostringstream ostrm;
    ostrm << "Pulse fit successful with chisquare = " << m_fitChisq;
    (*m_msgFunc_p)(ZDCMsg::Debug, ostrm.str());

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

    // Now check for valid chisq and valid time
    //
    if (m_fitChisq/m_fitNDoF > 2 && m_fitChisq / (m_fitAmplitude + 1.0e-6) > maxChisqDivAmp) m_badChisq = true;
    if (m_fitTimeCorr < minT0Corr || m_fitTimeCorr > maxT0Corr) m_badT0 = true;
  }

  return !m_fitFailed;
}

void ZDCPulseAnalyzer::DoFit(double maxChisqDivAmp)
{
  float fitAmpMin = (m_useLowGain ? m_fitAmpMinLG : m_fitAmpMinHG);
  float fitAmpMax = (m_useLowGain ? m_fitAmpMaxLG : m_fitAmpMaxHG);

  // Set the initial values
  //
  float ampInitial = m_maxADCValue - m_minADCValue;   // ???   sometime it is smaller than 5???? why
  float t0Initial = (m_useLowGain ? m_nominalT0LG : m_nominalT0HG);

  if (ampInitial < fitAmpMin) ampInitial = fitAmpMin * 1.5;

  ZDCFitWrapper* fitWrapper = m_defaultFitWrapper.get();
  if (PrePulse()) fitWrapper = m_prePulseFitWrapper.get();

  if (m_adjTimeRangeEvent) {
    m_fitTMin = std::max(m_fitTMin, m_deltaTSample * m_minSampleEvt - m_deltaTSample / 2);
    m_fitTMax = std::min(m_fitTMax, m_deltaTSample * m_maxSampleEvt + m_deltaTSample / 2);

    float fitTReference = m_deltaTSample * m_usedPresampIdx;

    fitWrapper->Initialize(ampInitial, t0Initial, fitAmpMin, fitAmpMax, m_fitTMin, m_fitTMax, fitTReference);
  }
  else {
    fitWrapper->Initialize(ampInitial, t0Initial, fitAmpMin, fitAmpMax);
  }

  if (PrePulse()) {
    //
    //
    (static_cast<ZDCPrePulseFitWrapper*>(m_prePulseFitWrapper.get()))->SetInitialPrePulse(m_initialPrePulseAmp, m_initialPrePulseT0, m_initialExpAmp, m_fixPrePulse);

    if (m_initialPrePulseT0 < 0) {
      (static_cast<ZDCPrePulseFitWrapper*>(m_prePulseFitWrapper.get()))->SetPrePulseT0Range(-25, 0);
    }
    else {
      (static_cast<ZDCPrePulseFitWrapper*>(m_prePulseFitWrapper.get()))->SetPrePulseT0Range(-m_deltaTSample / 2, (m_peak2ndDerivMinSample - m_peak2ndDerivMinTolerance)*m_deltaTSample);
    }
  }

  // Now perform the fit
  //
  std::string options = s_fitOptions + "N";
  if (QuietFits()) {
    options += "Q";
  }

  m_fitFailed = false;
  //
  //  Fit the data with the function provided by the fit wrapper
  //
  TFitResultPtr result_ptr = m_fitHist->Fit(fitWrapper->GetWrapperTF1RawPtr(), options.c_str(), "", m_fitTMin, m_fitTMax);
  int fitStatus = result_ptr;
  
  double chisqTest = result_ptr->Chi2()/fitWrapper->GetAmplitude();

  //
  // If the first fit failed, also check the EDM. If sufficiently small, the failure is almost surely due
  //   to parameter limits and we just accept the result
  //
  if ((fitStatus != 0 && result_ptr->Edm() > 0.001) || chisqTest > maxChisqDivAmp) // soon: pass the maxChisqDivAmp in so not hard-coded
    {
    //
    // We contstrain the fit and try again
    //
    fitWrapper->ConstrainFit();

    TFitResultPtr constrFitResult_ptr = m_fitHist->Fit(fitWrapper->GetWrapperTF1RawPtr(), options.c_str(), "", m_fitTMin, m_fitTMax);
    fitWrapper->UnconstrainFit();

    if ((int) constrFitResult_ptr != 0) {
      //
      // Even the constrained fit failed, so we quit.
      //
      m_fitFailed = true;
    }
    else {
      // Now we try the fit again with the constraint removed
      //
      TFitResultPtr unconstrFitResult_ptr = m_fitHist->Fit(fitWrapper->GetWrapperTF1RawPtr(), options.c_str(), "", m_fitTMin, m_fitTMax);
      if ((int) unconstrFitResult_ptr != 0) {
	//
	// The unconstrained fit failed again, so we redo the constrained fit
	//
	fitWrapper->ConstrainFit();
      
	TFitResultPtr constrFit2Result_ptr = m_fitHist->Fit(fitWrapper->GetWrapperTF1RawPtr(), options.c_str(), "", m_fitTMin, m_fitTMax);
	if ((int) constrFit2Result_ptr != 0) {
	  //
	  // Even the constrained fit failed the second time, so we quit.
	  //
	  m_fitFailed = true;
	}

	result_ptr = constrFit2Result_ptr;
	fitWrapper->UnconstrainFit();
      }
      else {
	result_ptr = unconstrFitResult_ptr;
      }
    }
  }

  if (!m_fitFailed && s_saveFitFunc) {
    m_fitHist->GetListOfFunctions()->Clear();
    m_fitHist->GetListOfFunctions()->Add(fitWrapper->GetWrapperTF1RawPtr());
  }

  m_bkgdMaxFraction = fitWrapper->GetBkgdMaxFraction();
  m_fitAmplitude = fitWrapper->GetAmplitude();
  m_fitTime      = fitWrapper->GetTime();

  m_fitTimeSub = m_fitTime - t0Initial;

  m_fitChisq = result_ptr->Chi2();
  m_fitNDoF = result_ptr->Ndf();

  m_fitTau1 = fitWrapper->GetTau1();
  m_fitTau2 = fitWrapper->GetTau2();

  m_fitAmpError = fitWrapper->GetAmpError();
}

void ZDCPulseAnalyzer::DoFitCombined(double)
{
  float fitAmpMin = (m_useLowGain ? m_fitAmpMinLG : m_fitAmpMinHG);
  float fitAmpMax = (m_useLowGain ? m_fitAmpMaxLG : m_fitAmpMaxHG);

  // Set the initial values
  //
  float ampInitial = m_maxADCValue - m_minADCValue;
  float t0Initial = (m_useLowGain ? m_nominalT0LG : m_nominalT0HG);

  if (ampInitial < fitAmpMin) ampInitial = fitAmpMin * 1.5;

  ZDCFitWrapper* fitWrapper = m_defaultFitWrapper.get();
  if (PrePulse() || PostPulse()) fitWrapper = m_prePulseFitWrapper.get();

  // Initialize the fit wrapper for this eventm, specifying a
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

  if (PrePulse() || PostPulse()) {
    //
    //
    (static_cast<ZDCPrePulseFitWrapper*>(m_prePulseFitWrapper.get()))->SetInitialPrePulse(m_initialPrePulseAmp, m_initialPrePulseT0, m_initialExpAmp, m_fixPrePulse);

    if (m_fitFunction == "ComplexPrePulse" || m_fitFunction == "GeneralPulse") {
      m_fitTMin = std::max(m_fitTMin, (float) - 0.5);
      (static_cast<ZDCPrePulseFitWrapper*>(m_prePulseFitWrapper.get()))->SetPrePulseT0Range(m_fitTMin, (m_peak2ndDerivMinSample - m_peak2ndDerivMinTolerance)*m_deltaTSample);
      (static_cast<ZDCPrePulseFitWrapper*>(m_prePulseFitWrapper.get()))->SetPostPulseT0Range(m_fitPostT0lo, m_fitTMax, m_initialPostPulseT0);
    }
    else {
      if (m_initialPrePulseT0 < 0) {
        (static_cast<ZDCPrePulseFitWrapper*>(m_prePulseFitWrapper.get()))->SetPrePulseT0Range(-25, 0);
      }
      else {
        (static_cast<ZDCPrePulseFitWrapper*>(m_prePulseFitWrapper.get()))->SetPrePulseT0Range(-m_deltaTSample / 2, (m_peak2ndDerivMinSample - m_peak2ndDerivMinTolerance)*m_deltaTSample);
      }
    }
  }

  // Set up the virtual fitter
  //
  TFitter* theFitter = nullptr;

  if (PrePulse()) {
    m_prePulseCombinedFitter = MakeCombinedFitter(fitWrapper->GetWrapperTF1RawPtr());

    theFitter = m_prePulseCombinedFitter.get();
  }
  else {
    m_defaultCombinedFitter = MakeCombinedFitter(fitWrapper->GetWrapperTF1RawPtr());

    theFitter = m_defaultCombinedFitter.get();
  }

  // Set the static pointers to histograms and function for use in FCN
  //
  s_undelayedFitHist = m_fitHist.get();
  s_delayedFitHist = m_delayedHist.get();
  s_combinedFitFunc = fitWrapper->GetWrapperTF1RawPtr();
  s_combinedFitTMax = m_fitTMax;
  s_combinedFitTMin = m_fitTMin;

  size_t numFitPar = theFitter->GetNumberTotalParameters();

  theFitter->GetMinuit()->fISW[4] = -1;

  // Now perform the fit
  //
  if (s_quietFits) {
    theFitter->GetMinuit()->fISW[4] = -1;

    int  ierr= 0; 
    theFitter->GetMinuit()->mnexcm("SET NOWarnings",nullptr,0,ierr);
  }
  else theFitter->GetMinuit()->fISW[4] = 0;

  // Only include baseline shift in fit for pre-pulses. Otherwise baseline matching should work
  //
  if (PrePulse()) {
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

  // Here we need to check if fitAmp is small (close) enough to fitAmpMin.
  // with "< 1+epsilon" where epsilon ~ 1%
  if (status || fitAmp < fitAmpMin * 1.01) {

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

  if (!s_quietFits) theFitter->GetMinuit()->fISW[4] = -1;

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
  double chi2, edm, errdef;
  int nvpar, nparx;

  theFitter->GetStats(chi2, edm, errdef, nvpar, nparx);

  int ndf = 2 * m_Nsample - nvpar;

  s_combinedFitFunc->SetChisquare(chi2);
  s_combinedFitFunc->SetNDF(ndf);

  // add to list of functions
  if (s_saveFitFunc) {
    s_undelayedFitHist->GetListOfFunctions()->Clear();
    s_undelayedFitHist->GetListOfFunctions()->Add(s_combinedFitFunc);

    s_delayedFitHist->GetListOfFunctions()->Clear();
    s_delayedFitHist->GetListOfFunctions()->Add(s_combinedFitFunc);
  }

  // Save the pull values from the last call to FCN
  //
  arglist[0] = 3; // number of function calls
  theFitter->ExecuteCommand("Cal1fcn", arglist, 1);
  m_fitPulls = s_pullValues;
  
  m_fitAmplitude = fitWrapper->GetAmplitude();
  m_fitTime      = fitWrapper->GetTime();
  if (PrePulse()) {
    m_fitPreT0   = (static_cast<ZDCPrePulseFitWrapper*>(m_prePulseFitWrapper.get()))->GetPreT0();
    m_fitPreAmp  = (static_cast<ZDCPrePulseFitWrapper*>(m_prePulseFitWrapper.get()))->GetPreAmp();
    m_fitPostT0  = (static_cast<ZDCPrePulseFitWrapper*>(m_prePulseFitWrapper.get()))->GetPostT0();
    m_fitPostAmp = (static_cast<ZDCPrePulseFitWrapper*>(m_prePulseFitWrapper.get()))->GetPostAmp();
    m_fitExpAmp  = (static_cast<ZDCPrePulseFitWrapper*>(m_prePulseFitWrapper.get()))->GetExpAmp();
  }

  m_fitTimeSub = m_fitTime - t0Initial;
  m_fitChisq = chi2;
  m_fitNDoF = ndf;

  m_fitTau1 = fitWrapper->GetTau1();
  m_fitTau2 = fitWrapper->GetTau2();

  m_fitAmpError = fitWrapper->GetAmpError();
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

      fitter->SetParameter(ipar + 1, func->GetParName(ipar), value, 0.01, parLimitLow, parLimitHigh);
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

void ZDCPulseAnalyzer::Dump() const
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

void ZDCPulseAnalyzer::Dump_setting() const    // setting
{
  if (m_useDelayed) {
    (*m_msgFunc_p)(ZDCMsg::Info, ("using delayed samples with delta T = " + std::to_string(m_delayedDeltaT) + ", and pedestalDiff == " + std::to_string(m_delayedPedestalDiff)));
  }

  (*m_msgFunc_p)(ZDCMsg::Info, ("m_fixTau1 = " + std::to_string(m_fixTau1) + "  m_fixTau2=" + std::to_string(m_fixTau2) + "  m_nominalTau1=" + std::to_string(m_nominalTau1) + "  m_nominalTau2=" + std::to_string(m_nominalTau2) + "  m_nominalT0HG=" + std::to_string(m_nominalT0HG) + "  m_nominalT0LG=" + std::to_string(m_nominalT0LG)));

  (*m_msgFunc_p)(ZDCMsg::Info, ("m_defaultFitTMax = " + std::to_string(m_defaultFitTMax)));

  (*m_msgFunc_p)(ZDCMsg::Info, ("m_HGOverflowADC = " + std::to_string(m_HGOverflowADC) + "  m_HGUnderflowADC=" + std::to_string(m_HGUnderflowADC) + "  m_LGOverflowADC=" + std::to_string(m_LGOverflowADC)));

  (*m_msgFunc_p)(ZDCMsg::Info, ("m_chisqDivAmpCutLG = " + std::to_string(m_chisqDivAmpCutLG) + "  m_chisqDivAmpCutHG=" + std::to_string(m_chisqDivAmpCutHG)));

  (*m_msgFunc_p)(ZDCMsg::Info, ("m_T0CutLowLG = " + std::to_string(m_T0CutLowLG) + "  m_T0CutHighLG=" + std::to_string(m_T0CutHighLG) + "  m_T0CutLowHG=" + std::to_string(m_T0CutLowHG) + "  m_T0CutHighHG=" + std::to_string(m_T0CutHighHG)));
}

unsigned int ZDCPulseAnalyzer::GetStatusMask() const
{
  unsigned int statusMask = 0;

  if (HavePulse())  statusMask |= 1 << PulseBit;
  if (UseLowGain()) statusMask |= 1 << LowGainBit;
  if (Failed())     statusMask |= 1 << FailBit;
  if (HGOverflow()) statusMask |= 1 << HGOverflowBit;

  if (HGUnderflow())       statusMask |= 1 << HGUnderflowBit;
  if (PSHGOverUnderflow()) statusMask |= 1 << PSHGOverUnderflowBit;
  if (LGOverflow())        statusMask |= 1 << LGOverflowBit;
  if (LGUnderflow())       statusMask |= 1 << LGUnderflowBit;

  if (PrePulse())  statusMask |= 1 << PrePulseBit;
  if (PostPulse()) statusMask |= 1 << PostPulseBit;
  if (FitFailed()) statusMask |= 1 << FitFailedBit;
  if (BadChisq())  statusMask |= 1 << BadChisqBit;

  if (BadT0())          statusMask |= 1 << BadT0Bit;
  if (ExcludeEarlyLG()) statusMask |= 1 << ExcludeEarlyLGBit;
  if (ExcludeLateLG())  statusMask |= 1 << ExcludeLateLGBit;
  if (preExpTail())     statusMask |= 1 << preExpTailBit;
  if (fitMinimumAmplitude()) statusMask |= 1 << FitMinAmpBit;
  if (repassPulse()) statusMask |= 1 << RepassPulseBit;

  return statusMask;
}

std::shared_ptr<TGraphErrors> ZDCPulseAnalyzer::GetCombinedGraph() const {
  //
  // We defer filling the histogram if we don't have a pulse until the histogram is requested
  //
  GetHistogramPtr();

  std::shared_ptr<TGraphErrors> theGraph = std::make_shared<TGraphErrors>(TGraphErrors(2 * m_Nsample));
  size_t npts = 0;

  for (int ipt = 0; ipt < m_fitHist->GetNbinsX(); ipt++) {
    theGraph->SetPoint(npts, m_fitHist->GetBinCenter(ipt + 1), m_fitHist->GetBinContent(ipt + 1));
    theGraph->SetPointError(npts++, 0, m_fitHist->GetBinError(ipt + 1));
  }

  for (int iDelayPt = 0; iDelayPt < m_delayedHist->GetNbinsX(); iDelayPt++) {
    theGraph->SetPoint(npts, m_delayedHist->GetBinCenter(iDelayPt + 1), m_delayedHist->GetBinContent(iDelayPt + 1) - m_delayedBaselineShift);
    theGraph->SetPointError(npts++, 0, m_delayedHist->GetBinError(iDelayPt + 1));
  }
  if (m_havePulse) {
    TF1* func_p = (TF1*) m_fitHist->GetListOfFunctions()->Last();
    if (func_p) {
      theGraph->GetListOfFunctions()->Add(new TF1(*func_p));
      m_fitHist->GetListOfFunctions()->SetOwner (false);
    }
  }
  theGraph->SetName(( std::string(m_fitHist->GetName()) + "combinaed").c_str());

  theGraph->SetMarkerStyle(20);
  theGraph->SetMarkerColor(1);

  return theGraph;
}


std::shared_ptr<TGraphErrors> ZDCPulseAnalyzer::GetGraph() const {
  //
  // We defer filling the histogram if we don't have a pulse until the histogram is requested
  //
  GetHistogramPtr();

  std::shared_ptr<TGraphErrors> theGraph = std::make_shared<TGraphErrors>(TGraphErrors(m_Nsample));
  size_t npts = 0;

  for (int ipt = 0; ipt < m_fitHist->GetNbinsX(); ipt++) {
    theGraph->SetPoint(npts, m_fitHist->GetBinCenter(ipt + 1), m_fitHist->GetBinContent(ipt + 1));
    theGraph->SetPointError(npts++, 0, m_fitHist->GetBinError(ipt + 1));
  }

  TF1* func_p = (TF1*) m_fitHist->GetListOfFunctions()->Last();
  theGraph->GetListOfFunctions()->Add(func_p);
  theGraph->SetName(( std::string(m_fitHist->GetName()) + "not_combinaed").c_str());

  theGraph->SetMarkerStyle(20);
  theGraph->SetMarkerColor(1);

  return theGraph;
}

std::shared_ptr<TGraphErrors> ZDCPulseAnalyzer::GetUndelayedGraph() const {
  //
  // We defer filling the histogram if we don't have a pulse until the histogram is requested
  //
  GetHistogramPtr();

  std::shared_ptr<TGraphErrors> theGraph = std::make_shared<TGraphErrors>(TGraphErrors(m_Nsample));
  size_t npts = 0;

  for (int ipt = 0; ipt < m_fitHist->GetNbinsX(); ipt++) {
    theGraph->SetPoint(npts, m_fitHist->GetBinCenter(ipt + 1), m_fitHist->GetBinContent(ipt + 1));
    theGraph->SetPointError(npts++, 0, m_fitHist->GetBinError(ipt + 1));
  }

  TF1* func_p = (TF1*) m_fitHist->GetListOfFunctions()->Last();
  theGraph->GetListOfFunctions()->Add(func_p);
  theGraph->SetName(( std::string(m_fitHist->GetName()) + "undelayed").c_str());

  theGraph->SetMarkerStyle(20);
  theGraph->SetMarkerColor(1);

  return theGraph;
}

std::shared_ptr<TGraphErrors> ZDCPulseAnalyzer::GetDelayedGraph() const {
  //
  // We defer filling the histogram if we don't have a pulse until the histogram is requested
  //
  GetHistogramPtr();

  std::shared_ptr<TGraphErrors> theGraph = std::make_shared<TGraphErrors>(TGraphErrors(m_Nsample));
  size_t npts = 0;

  for (int iDelayPt = 0; iDelayPt < m_delayedHist->GetNbinsX(); iDelayPt++) {
    theGraph->SetPoint(npts, m_delayedHist->GetBinCenter(iDelayPt + 1), m_delayedHist->GetBinContent(iDelayPt + 1) - m_delayedBaselineShift);
    theGraph->SetPointError(npts++, 0, m_delayedHist->GetBinError(iDelayPt + 1));
  }

  TF1* func_p = (TF1*) m_fitHist->GetListOfFunctions()->Last();
  theGraph->GetListOfFunctions()->Add(func_p);
  theGraph->SetName(( std::string(m_fitHist->GetName()) + "delayed").c_str());

  theGraph->SetMarkerStyle(20);
  theGraph->SetMarkerColor(kBlue);

  return theGraph;
}

std::vector<float> ZDCPulseAnalyzer::Calculate2ndDerivative(const std::vector <float>& inputData, unsigned int step)
{
  // Start with two zero entries for which we can't calculate the double-step derivative
  //
  std::vector<float> results(step, 0);
  
  unsigned int nSamples = inputData.size();

  for (unsigned int sample = step; sample < nSamples - step; sample++) {
    int deriv2nd = inputData[sample + step] + inputData[sample - step] - 2*inputData[sample];
    results.push_back(deriv2nd);
  }

  for (unsigned int i = 0; i < step; i++) { 
    results.push_back(0);
  }

  return results;
}
 
