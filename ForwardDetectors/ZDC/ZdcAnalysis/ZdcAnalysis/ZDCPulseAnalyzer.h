/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ZDCANALYSIS_ZDCPulseAnalyzer_h
#define ZDCANALYSIS_ZDCPulseAnalyzer_h

#include "CxxUtils/checker_macros.h"
#include "ZdcAnalysis/ZDCFitWrapper.h"
#include "ZdcAnalysis/ZDCJSONConfig.h"
#include "ZdcAnalysis/ZDCMsg.h"
#include "TGraphErrors.h"
#include "TFitter.h"
#include "TF1.h"
#include "TH1.h"

#include <vector>
#include <string>
#include <memory>
#include <tuple>


class ATLAS_NOT_THREAD_SAFE ZDCPulseAnalyzer
{
public:
  using JSON = ZDCJSONConfig::JSON;

  enum {PulseBit              = 0,  //  &1
        LowGainBit            = 1,  //  &2
        FailBit               = 2,  //  &4
        HGOverflowBit         = 3,  //  &8
        // -------------------------
        HGUnderflowBit        = 4,  //  &16
        PSHGOverUnderflowBit  = 5,  //  &32
        LGOverflowBit         = 6,  //  &64
        LGUnderflowBit        = 7,  //  &128
        // -------------------------
        PrePulseBit           = 8,  //  &256
        PostPulseBit          = 9,  //  &512
        FitFailedBit          = 10, //  &1024
        BadChisqBit           = 11, //  &2048
        // -------------------------
        BadT0Bit              = 12, //  &4096
        ExcludeEarlyLGBit     = 13, //  &8192
        ExcludeLateLGBit      = 14, //  &16384
        preExpTailBit         = 15, //  &32768
        //
        FitMinAmpBit          = 16, // 0x10000
        RepassPulseBit        = 17, // 0x20000
        ArmSumIncludeBit      = 18, // 0x40000
	FailSigCutBit         = 19, // 0x80000
        N_STATUS_BITS
       };

  enum LowGainMode {
    LGModeNormal = 0,
    LGModeForceLG,
    LGModeRefitLG
  };
  
  enum TimingCorrMode {NoTimingCorr = 0, TimingCorrLin, TimingCorrLog};

  //
  // List of allowed JSON configuration parameters
  //
  //  For each parameter we have name, JSON value type, whether it can be set per channel, and whether it is required
  //
  //  if the type is -1, then there's no value, the presence of the parameter itself is a boolean -- i.e. enabling  
  //
  
  static const ZDCJSONConfig::JSONParamList JSONConfigParams;
private:
  typedef std::vector<float>::const_iterator SampleCIter;

  //  Static data
  //
  bool m_quietFits;
  bool m_saveFitFunc;
  
  static TH1* s_undelayedFitHist;
  static TH1* s_delayedFitHist;
  static TF1* s_combinedFitFunc;
  static float s_combinedFitTMax;
  static float s_combinedFitTMin;
  static std::vector<float> s_pullValues;

  // Quantities provided/set in the constructor
  //
  ZDCMsg::MessageFunctionPtr m_msgFunc_p{};
  std::string m_tag;
  unsigned int m_Nsample{};
  unsigned int m_preSampleIdx{};
  float m_freqMHz;
  float m_deltaTSample{};
  int m_pedestal{};
  float m_gainHG{};
  unsigned int m_LGMode{LGModeNormal};
  float m_tmin{};
  float m_tmax{};

  std::string m_fitFunction;
  size_t m_2ndDerivStep{1};
  size_t m_peak2ndDerivMinSample{};
  size_t m_peak2ndDerivMinTolerance{1};
  float m_peak2ndDerivMinThreshLG{};
  float m_peak2ndDerivMinThreshHG{};

  bool m_useDelayed{false};

  bool m_enableRepass{false};
  float m_peak2ndDerivMinRepassLG{};
  float m_peak2ndDerivMinRepassHG{};

  // Gain factors for low gain and high gain
  //
  float m_gainFactorHG{};
  float m_gainFactorLG{};

  // Uncertainties on the ADC values due to noise
  //
  float m_noiseSigHG{};
  float m_noiseSigLG{};

  // Default fit values and cuts that can be set via modifier methods
  //
  std::string m_fitOptions;
  int m_HGOverflowADC{};
  int m_HGUnderflowADC{};
  int m_LGOverflowADC{};

  float m_nominalT0HG{};
  float m_nominalT0LG{};

  float m_nominalTau1{};
  float m_nominalTau2{};

  bool m_fixTau1{};
  bool m_fixTau2{};

  float m_defaultFitTMax{};   // user-provided upper limit on samples to be included in fit
  float m_defaultFitTMin{};   // user-provided upper limit on samples to be included in fit

  float m_chisqDivAmpCutLG{}; // maximum good LG chisq / amplitude
  float m_chisqDivAmpCutHG{}; // maximum good HG chisq / amplitude

  float m_T0CutLowLG{};  // minimum good corrected time for LG fits
  float m_T0CutHighLG{}; // maximum good corrected time for LG fits

  float m_T0CutLowHG{};  // minimum good corrected time for HG fits
  float m_T0CutHighHG{}; // maximum good corrected time for HG fits

  std::unique_ptr<const TF1> m_timeResFuncHG_p{};
  std::unique_ptr<const TF1> m_timeResFuncLG_p{};
  float m_t0CutSig{};
  unsigned int m_timeCutMode{0}; // 0 - no significance cut, 1 - cut ORed with fixed cut, 2 - cut ANDed with fixed cut

  float m_defaultT0Max{};   // Upper limit on pulse t0
  float m_defaultT0Min{};   // Lower limit on pulse t0

  float m_fitAmpMinHG{};      // Minimum amplitude in the fit
  float m_fitAmpMinLG{};      // Minimum amplitude in the fit

  float m_fitAmpMaxHG{};      // Minimum am`plitude in the fit
  float m_fitAmpMaxLG{};      // Minimum amplitude in the fit

  bool m_haveSignifCuts{false};
  float m_sigMinHG;           // Minimum amplitude significance to be considered valid pulse
  float m_sigMinLG;           // Minimum amplitude significance to be considered valid pulse
  
  // Enabling (or not) of exclusion of early or late samples from OOT pileup
  //
  bool m_enablePreExcl{false};
  unsigned int m_maxSamplesPreExcl{0};
  unsigned int m_preExclHGADCThresh{0};
  unsigned int m_preExclLGADCThresh{0};
  
  bool m_enablePostExcl{false};
  unsigned int m_postExclHGADCThresh{0};
  unsigned int m_postExclLGADCThresh{0};
  unsigned int m_maxSamplesPostExcl{0};

  //
  unsigned int m_timingCorrMode{NoTimingCorr};
  float m_timingCorrRefADC{500};
  float m_timingCorrScale{100};
  std::vector<float> m_LGT0CorrParams; // Parameters used to correct the fit LG times
  std::vector<float> m_HGT0CorrParams; // Parameters used to correct the fit HG times

  bool m_haveNonlinCorr{false};
  float m_nonLinCorrRefADC{500};
  float m_nonLinCorrRefScale{100};
  std::vector<float> m_nonLinCorrParamsHG;
  std::vector<float> m_nonLinCorrParamsLG;

  bool m_haveFADCCorrections{false};
  std::string m_fadcCorrFileName;
  bool m_FADCCorrPerSample{false};
  std::unique_ptr<const TH1> m_FADCCorrHG{};
  std::unique_ptr<const TH1> m_FADCCorrLG{};
  
  // Histogram used to perform the fits and function wrappers
  //
  std::unique_ptr<TH1> m_fitHist;
  std::unique_ptr<TH1> m_fitHistLGRefit;

  bool m_initializedFits{false};
  std::unique_ptr<ZDCFitWrapper> m_defaultFitWrapper;
  std::unique_ptr<ZDCPrePulseFitWrapper> m_prePulseFitWrapper;
  std::unique_ptr<ZDCPreExpFitWrapper> m_preExpFitWrapper;

  // Members to keep track of adjustments to time range used in analysis/fit
  //
  bool m_adjTimeRangeEvent{false}; // indicates whether we adjust the time range for this specific event

  unsigned int m_minSampleEvt{};
  unsigned int m_maxSampleEvt{};

  // Delayed pulse members
  //
  bool  m_useFixedBaseline{};
  float m_delayedDeltaT{};
  float m_delayedPedestalDiff{};
  std::unique_ptr<TH1> m_delayedHist;
  std::unique_ptr<TH1> m_delayedHistLGRefit;

  std::unique_ptr<TFitter> m_prePulseCombinedFitter;
  std::unique_ptr<TFitter> m_defaultCombinedFitter;

  // Dynamic data loaded for each pulse (event)
  // ==========================================

  // -----------------------
  // Statuses
  //
  bool m_haveData{};

  bool m_havePulse{};
  bool m_useLowGain{};
  bool m_fail{};
  bool m_HGOverflow{};

  bool m_HGUnderflow{};
  bool m_PSHGOverUnderflow{};
  bool m_LGOverflow{};
  bool m_LGUnderflow{};

  bool m_prePulse{};
  bool m_postPulse{};
  bool m_fitFailed{};
  bool m_badChisq{};

  bool m_badT0{};
  bool m_ExcludeEarly{};
  bool m_ExcludeLate{};
  bool m_preExpTail{};

  bool m_fixPrePulse{};
  bool m_fitMinAmp{};
  bool m_repassPulse{};
  bool m_failSigCut{};

  // -----------------------

  bool  m_backToHG_pre{};
  float m_baselineCorr{};

  // Pulse analysis
  //
  int m_usedPresampIdx{};
  float m_preSample{};

  float m_minADCHG{};
  float m_maxADCHG{};
  int m_minADCSampleHG;
  int m_maxADCSampleHG;
  
  float m_maxADCLG{};
  float m_minADCLG{};
  int m_minADCSampleLG;
  int m_maxADCSampleLG;

  float m_ADCPeakHG{};
  float m_ADCPeakLG{};

  float m_maxDelta{};
  float m_minDelta{};

  float m_initialExpAmp{};
  float m_minDeriv2nd{};
  int   m_minDeriv2ndIndex{};

  float m_fitTMax{};          // event-by-event specified fit tmax
  float m_fitTMin{};          // event-by-event specified fit tmin

  float m_fitPostT0lo{};      // use to assign lower bound of post pulse T0

  float m_minDeriv2ndSig;
  float m_preExpSig;
  float m_prePulseSig;
  
  float m_initialPrePulseT0{};
  float m_initialPrePulseAmp{};

  float m_initialPostPulseT0{};

  float m_fitAmplitude{};
  float m_fitAmpError{};
  float m_fitTime{};
  float m_fitTimeSub{};
  float m_fitTimeCorr{};
  float m_timeSig{};
  float m_fitTCorr2nd{};
  float m_fitTau1{};
  float m_fitTau2{};
  float m_fitChisq{};
  float m_fitNDoF{};
  float m_fitPreT0{};
  float m_fitPreAmp{};
  float m_fitPostT0{};
  float m_fitPostAmp{};
  float m_fitExpAmp{};
  float m_amplitude{};
  float m_ampNoNonLin{};
  float m_ampError{};
  float m_preSampleAmp{};
  float m_preAmplitude{};
  float m_postAmplitude{};
  float m_expAmplitude{};
  float m_bkgdMaxFraction{};
  float m_delayedBaselineShift{};

  bool m_evtLGRefit{false};
  float m_refitLGAmpl{0};
  float m_refitLGFitAmpl{0};
  float m_refitLGAmplCorr{0};
  float m_refitLGAmpError{0};
  float m_refitLGChisq{0};
  float m_refitLGTime{0};
  float m_refitLGTimeSub{0};
  
  int m_lastHGOverFlowSample{-1};
  int m_firstHGOverFlowSample{-1};

  unsigned int m_NSamplesAna{0};
  std::vector<float> m_ADCSamplesHG;
  std::vector<float> m_ADCSamplesLG;
  std::vector<float> m_ADCSamplesHGSub;
  std::vector<float> m_ADCSamplesLGSub;

  std::vector<bool> m_useSampleLG;
  std::vector<bool> m_useSampleHG;

  std::vector<float> m_ADCSSampSigHG;
  std::vector<float> m_ADCSSampSigLG;

  std::vector<float> m_samplesSub;
  std::vector<float> m_samplesSig;

  std::vector<float> m_samplesLGRefit;
  std::vector<float> m_samplesSigLGRefit;
  
  std::vector<float> m_samplesDeriv2nd;

  // When using combined delayed + undelayed pulses we calculate the chisquare ourselves
  //   so fill this vector as part of that calculation. For the cases where we do not use
  //   delayed samples (2015 and Run3 onward) this vector is not used as the pulls are calculated
  //   when they are fetched.
  //
  std::vector<float> m_fitPulls;

  // Private methods
  //
  void Reset(bool reanalyze = false);
  void SetDefaults();
  
  std::pair<bool, std::string> ValidateJSONConfig(const JSON& config);
  std::pair<bool, std::string> ConfigFromJSON(const JSON& config);

  void SetupFitFunctions();

  bool DoAnalysis(bool repass);

  bool ScanAndSubtractSamples();

  bool AnalyzeData(size_t nSamples, size_t preSample,
                   const std::vector<float>& samples,        // The samples used for this event
		   const std::vector<bool>& useSamples,        // The samples used for this event
                   float peak2ndDerivMinThresh,
                   float noiseSig,                           // The "resolution" on the ADC value
                   const std::vector<float>& toCorrParams,   // The parameters used to correct the t0
                   float maxChisqDivAmp,                     // The maximum chisq / amplitude ratio
                   float minT0Corr, float maxT0Corr          // The minimum and maximum corrected T0 values
                  );


  double getAmplitudeCorrection(bool highGain);
    
  static std::vector<float> Calculate2ndDerivative(const std::vector <float>& inputData, unsigned int step);
  static std::vector<float> CalculateDerivative(const std::vector <float>& inputData, unsigned int step);
  static float obtainDelayedBaselineCorr(const std::vector<float>& samples);

  void prepareLGRefit(const std::vector<float>& samplesLG, const std::vector<float>& samplesSig,
		      const std::vector<bool>& useSamples);
  
  void FillHistogram(bool refitLG)
  {
    if (!m_useDelayed) {
      if (!refitLG) {
	// Set the data and errors in the histogram object
	//
	for (size_t isample = 0; isample < m_NSamplesAna; isample++) {
	  m_fitHist->SetBinContent(isample + 1, m_samplesSub[isample]);
	  m_fitHist->SetBinError(isample + 1, m_samplesSig[isample]);
	}
      }
      else {
	for (size_t isample = 0; isample < m_NSamplesAna; isample++) {
	  m_fitHistLGRefit->SetBinContent(isample + 1, m_samplesLGRefit[isample]);
	  m_fitHistLGRefit->SetBinError(isample + 1, m_samplesSigLGRefit[isample]);
	}
      }
    }
    else {
      if (!refitLG) {
        // Set the data and errors in the histogram object
	//
	for (size_t isample = 0; isample < m_Nsample; isample++) {
	  m_fitHist->SetBinContent(isample + 1, m_samplesSub[isample * 2]);
	  m_delayedHist->SetBinContent(isample + 1, m_samplesSub[isample * 2 + 1]);
	  
	  m_fitHist->SetBinError(isample + 1, m_samplesSig[isample]); 
	  m_delayedHist->SetBinError(isample + 1, m_samplesSig[isample]);
	}
      }
      else {
        // Set the data and errors in the histogram object
	//
	for (size_t isample = 0; isample < m_Nsample; isample++) {
	  m_fitHistLGRefit->SetBinContent(isample + 1, m_samplesLGRefit[isample * 2]);
	  m_delayedHistLGRefit->SetBinContent(isample + 1, m_samplesLGRefit[isample * 2 + 1]);
	  
	  m_fitHistLGRefit->SetBinError(isample + 1, m_samplesSigLGRefit[isample]); 
	  m_delayedHistLGRefit->SetBinError(isample + 1, m_samplesSigLGRefit[isample]);
	}
      }
    }
  }

  void checkTF1Limits(TF1* func);
  
  void DoFit(bool refitLG = false);
  void DoFitCombined(bool refitLG = false);

  static std::unique_ptr<TFitter> MakeCombinedFitter(TF1* func);

  //  The minuit FCN used for fitting combined undelayed and delayed pulses
  //
  static void CombinedPulsesFCN(int& numParam, double*, double& f, double* par, int flag);

  void UpdateFitterTimeLimits(TFitter* fitter, ZDCFitWrapper* wrapper, bool prePulse);

public:

  ZDCPulseAnalyzer(ZDCMsg::MessageFunctionPtr msgFunc_p, const std::string& tag, int Nsample, float deltaTSample, size_t preSampleIdx,
		   int pedestal, float gainHG, const std::string& fitFunction, int peak2ndDerivMinSample, float peak2DerivMinThreshHG,
		   float peak2DerivMinThreshLG);

  ZDCPulseAnalyzer(ZDCMsg::MessageFunctionPtr msgFunc_p, const JSON& configJSON);

  ~ZDCPulseAnalyzer(){}

  void setFitOPtions(const std::string& fitOptions) { m_fitOptions = fitOptions;}
  void saveFitFunc() {m_saveFitFunc = true;}

  bool quietFits() const {return m_quietFits;}
  void setQuietFits() {m_quietFits = true;}
  void setUnquietFits() {m_quietFits = false;}

  void enableDelayed(float deltaT, float pedestalShift, bool fixedBaseline = false);

  void enableRepass(float peak2ndDerivMinRepassHG, float peak2ndDerivMinRepassLG);

  void enableTimeSigCut(bool AND, float sigCut, const std::string& TF1String,
			const std::vector<double>& parsHG, 
			const std::vector<double>& parsLG); 

  void enablePreExclusion(unsigned int maxSamplesExcl, unsigned int HGADCThresh, unsigned int LGADCThresh)
  {
    m_enablePreExcl = true;
    m_maxSamplesPreExcl = maxSamplesExcl;
    m_preExclHGADCThresh = HGADCThresh;
    m_preExclLGADCThresh = LGADCThresh;
  }

  void enablePostExclusion(unsigned int maxSamplesExcl, unsigned int HGADCThresh, unsigned int LGADCThresh)
  {
    m_enablePostExcl = true;
    m_maxSamplesPostExcl = maxSamplesExcl;
    m_postExclHGADCThresh = HGADCThresh;
    m_postExclLGADCThresh = LGADCThresh;
  }

  void SetPeak2ndDerivMinTolerance(size_t tolerance) {
    m_peak2ndDerivMinTolerance = tolerance;
    m_initializedFits = false;
  }

  void setLGMode(unsigned int mode) {m_LGMode = mode;}
  unsigned int getLGMode() const {return m_LGMode;}

  void set2ndDerivStep(size_t step) {m_2ndDerivStep = step;}

  void SetCutValues(float chisqDivAmpCutHG, float chisqDivAmpCutLG,
                    float deltaT0MinHG, float deltaT0MaxHG,
                    float deltaT0MinLG, float deltaT0MaxLG) ;

  void SetNoiseSigmas(float noiseSigHG, float noiseSigLG) 
  {
    m_noiseSigHG = noiseSigHG;
    m_noiseSigLG = noiseSigLG;
  }

  void SetGainFactorsHGLG(float gainFactorHG, float gainFactorLG); 

  void SetFitMinMaxAmp(float minAmpHG, float minAmpLG, float maxAmpHG, float maxAmpLG);

  void setMinimumSignificance(float sigMinHG, float sigMinLG);
  
  void SetTauT0Values(bool fixTau1, bool fixTau2, float tau1, float tau2, float t0HG, float t0LG);

  void SetADCOverUnderflowValues(int HGOverflowADC, int HGUnderflowADC, int LGOverflowADC);

  void SetTimingCorrParams(TimingCorrMode mode, float refADC, float refScale,
			   const std::vector<float>& HGT0CorrParams, const std::vector<float>& LGT0CorrParams)
  {
    m_timingCorrMode = mode;
    if (mode != NoTimingCorr) {
      m_timingCorrRefADC = refADC;
      m_timingCorrScale = refScale;

      m_HGT0CorrParams = HGT0CorrParams;
      m_LGT0CorrParams = LGT0CorrParams;
    }
  }

  void SetFitTimeMax(float tmax);

  void SetNonlinCorrParams(float refADC, float refScale, const std::vector<float>& paramsHG, const std::vector<float>& paramsLG)
  {
    std::string HGParamsStr = "HG coefficients = ", LGParamsStr = "LG coefficients = ";

    for (auto val : paramsHG) {HGParamsStr += std::to_string(val) + " ";}
    for (auto val : paramsLG) {LGParamsStr += std::to_string(val) + " ";}
    
    (*m_msgFunc_p)(ZDCMsg::Info, ("Setting non-linear parameters for module: " + m_tag + ", reference ADC = " +
				  std::to_string(refADC) + ", reference scale = " + std::to_string(refScale)));

    (*m_msgFunc_p)(ZDCMsg::Info, std::move(HGParamsStr));
    (*m_msgFunc_p)(ZDCMsg::Info, std::move(LGParamsStr));

    m_nonLinCorrRefADC = refADC;
    m_nonLinCorrRefScale = refScale;
    m_nonLinCorrParamsHG = paramsHG;
    m_nonLinCorrParamsLG = paramsLG;
    m_haveNonlinCorr = true;
  }

  // Provide a historam that provides per-ADC channel correction factors for integral and differential
  //   non-linearities
  //
  void enableFADCCorrections(bool correctPerSample, std::unique_ptr<const TH1>& correHistHG, std::unique_ptr<const TH1>& correHistLG);
  void disableFADCCorrections() {m_haveFADCCorrections = false;}
  
  bool LoadAndAnalyzeData(const std::vector<float>& ADCSamplesHG, const std::vector<float>& ADCSamplesLG);

  bool LoadAndAnalyzeData(const std::vector<float>& ADCSamplesHG, const std::vector<float>& ADCSamplesLG,
                          const std::vector<float>& ADCSamplesHGDelayed, const std::vector<float>& ADCSamplesLGDelayed);

  bool ReanalyzeData();

  bool HaveData() const {return m_haveData;}

  // ------------------------------------------------------------
  // Status bit setting functions
  //
  bool havePulse()  const {return m_havePulse;}
  bool useLowGain() const {return m_useLowGain;}
  bool failed()     const {return m_fail;}
  bool HGOverflow() const {return m_HGOverflow;}

  bool HGUnderflow()       const {return m_HGUnderflow;}
  bool PSHGOverUnderflow() const {return m_PSHGOverUnderflow;}
  bool LGOverflow()        const {return m_LGOverflow;}
  bool LGUnderflow()       const {return m_LGUnderflow;}

  bool prePulse()  const {return m_prePulse;}
  bool postPulse() const {return m_postPulse;}
  bool fitFailed() const {return m_fitFailed;}
  bool badChisq()  const {return m_badChisq;}

  bool badT0()          const {return m_badT0;}
  bool excludeEarlyLG() const {return m_ExcludeEarly;}
  bool excludeLateLG()  const {return m_ExcludeLate;}
  bool preExpTail()     const {return m_preExpTail;}
  bool fitMinimumAmplitude() const {return m_fitMinAmp;}
  bool repassPulse() const {return m_repassPulse;}
  bool armSumInclude() const {return havePulse() && !(fitFailed() || badChisq() || badT0() || fitMinimumAmplitude() || LGOverflow() || failSigCut());}
  bool failSigCut() const {return m_failSigCut;}

  // ------------------------------------------------------------


  // ---------------------------
  // Get fit parameters
  //
  float GetFitAmplitude() const {return m_fitAmplitude;}
  float GetFitT0()        const {return m_fitTime;}
  float GetT0Sub()        const {return m_fitTimeSub;}
  float GetT0Corr()       const {return m_fitTimeCorr;}
  float getTimeSig()      const {return m_timeSig;}
  float GetChisq()        const {return m_fitChisq;}
  float GetFitTau1()      const {return m_fitTau1;}
  float GetFitTau2()      const {return m_fitTau2;}
  float GetFitPreT0()     const {return m_fitPreT0;}
  float GetFitPreAmp()    const {return m_preAmplitude;}
  float GetFitPostT0()    const {return m_fitPostT0;}
  float GetFitPostAmp()   const {return m_postAmplitude;}
  float GetFitExpAmp()    const {return m_fitExpAmp;}
  // ---------------------------

  float GetAmpNoNonLin() const {return m_ampNoNonLin;}
  float GetAmplitude() const {return m_amplitude;}
  float GetAmpError() const {return m_ampError;}
  float GetPreExpAmp() const {return m_expAmplitude;}

  float getRefitLGAmp() const
  {
    if (m_evtLGRefit) return m_refitLGAmpl;
    else return 0;
  }

  float getRefitLGFitAmp() const
  {
    if (m_evtLGRefit) return m_refitLGFitAmpl;
    else return 0;
  }

  float getRefitLGAmpCorr() const
  {
    if (m_evtLGRefit) return m_refitLGAmplCorr;
    else return 0;
  }

  float getRefitLGChisq() const
  {
    if (m_evtLGRefit) return m_refitLGChisq;
    else return 0;
  }

  float getRefitLGTime() const
  {
    if (m_evtLGRefit) return m_refitLGTime;
    else return 0;
  }

  float getRefitLGTimeSub() const
  {
    if (m_evtLGRefit) return m_refitLGTimeSub;
    else return 0;
  }

  float getPresample() const {return m_preSample;}
  float getMaxADCHG() const {return m_maxADCHG;}
  float getMaxADCLG() const {return m_maxADCLG;}
  float getMinADCHG() const {return m_minADCHG;}
  float getMinADCLG() const {return m_minADCLG;}

  float getMaxADCSub() const {
    float maxADCNosub = m_useLowGain ? m_maxADCLG : m_maxADCHG;
    return maxADCNosub - m_pedestal - m_preSample;
  }
  
  float getMinADCSub() const {
    float minADCNosub = m_useLowGain ? m_minADCLG : m_minADCHG;
    return minADCNosub - m_pedestal - m_preSample;
  }

  int getMaxADCSampleHG() const {return m_maxADCSampleHG;}
  int getMinADCSampleHG() const {return m_minADCSampleHG;}

  int getMaxADCSampleLG() const {return m_maxADCSampleLG;}
  int getMinADCSampleLG() const {return m_minADCSampleLG;}

  float getADCPeakHG() const {return m_ADCPeakHG;}
  float getADCPeakLG() const {return m_ADCPeakLG;}
  
  float GetMaxDelta() const {return m_maxDelta;}
  float GetMinDelta() const {return m_minDelta;}

  float GetFitTMax() const {return m_fitTMax;}
  float GetFitTMin() const {return m_fitTMin;}

  float GetdelayBS() const {return m_delayedBaselineShift;}

  float GetMinDeriv2nd() const {return m_minDeriv2nd;}
  float GetMinDeriv2ndIndex() const {return m_minDeriv2ndIndex;}

  unsigned int GetStatusMask() const;

  float GetPreSampleAmp() const {return m_preSampleAmp;}
  float GetBkgdMaxFraction() const {return m_bkgdMaxFraction;}

  float GetDelayedBaselineShiftFit() const {return m_delayedBaselineShift;}
  float GetDelayedBaselineCorr() const {return m_baselineCorr;}

  const TH1* GetHistogramPtr(bool refitLG = false)
  {
    //
    // We defer filling the histogram if we don't have a pulse until the histogram is requested
    //
    if (!m_havePulse) {
      FillHistogram(refitLG);
    }
    
    return refitLG ? m_fitHistLGRefit.get() : m_fitHist.get();
  }

  std::shared_ptr<TGraphErrors> GetCombinedGraph(bool forceLG = false);
  std::shared_ptr<TGraphErrors> GetGraph(bool forceLG = false);

  std::vector<float> GetFitPulls(bool forceLG = false) const;

  void dump() const;
  void dumpConfiguration() const;
  void dumpTF1(const TF1*) const;

  const std::vector<float>& GetSamplesSub() const {return m_samplesSub;}
  const std::vector<float>& GetSamplesDeriv2nd() const {return m_samplesDeriv2nd;}
};


#endif
