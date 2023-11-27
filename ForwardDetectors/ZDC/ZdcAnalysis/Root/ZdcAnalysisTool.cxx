/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "ZdcAnalysis/ZdcAnalysisTool.h"
#include "TGraph.h"
#include "TEnv.h"
#include "TSystem.h"
#include "ZdcAnalysis/ZdcSincInterp.h"
#include "xAODEventInfo/EventInfo.h"
#include "TFile.h"
#include <sstream>
#include <memory>
#include <xAODForward/ZdcModuleAuxContainer.h>
#include "PathResolver/PathResolver.h"
#include <AsgDataHandles/ReadHandle.h>
#include <AsgDataHandles/WriteHandle.h>
#include <AsgDataHandles/WriteDecorHandle.h>

namespace ZDC
{
ZdcAnalysisTool::ZdcAnalysisTool(const std::string& name)
    : asg::AsgTool(name), m_name(name), m_init(false),
      m_writeAux(false), m_eventReady(false),
      m_runNumber(0), m_lumiBlock(0),
      m_zdcTriggerEfficiency(0)
{

#ifndef XAOD_STANDALONE
  declareInterface<IZdcAnalysisTool>(this);
#endif

    declareProperty("ZdcModuleContainerName", m_zdcModuleContainerName = "ZdcModules", "Location of ZDC processed data");
    declareProperty("ZdcSumContainerName", m_zdcSumContainerName = "ZdcSums", "Location of ZDC processed sums");
    declareProperty("Configuration", m_configuration = "PbPb2015");
    declareProperty("FlipEMDelay", m_flipEMDelay = false);
    declareProperty("LowGainOnly", m_lowGainOnly = false);
    declareProperty("WriteAux", m_writeAux = true);
    declareProperty("AuxSuffix", m_auxSuffix = "");

    // The following job properties enable/disable and affect the calibration of the ZDC energies
    //
    declareProperty("DoCalib", m_doCalib = true);
    declareProperty("DoTrigEff", m_doTrigEff = true);
    declareProperty("DoTimeCalib", m_doTimeCalib = true);
    declareProperty("ZdcAnalysisConfigPath", m_zdcAnalysisConfigPath = "$ROOTCOREBIN/data/ZdcAnalysis", "ZDC Analysis config file path");
    //declareProperty("ForceCalibRun",m_forceCalibRun=287931); // last run of Pb+Pb 2015
    declareProperty("ForceCalibRun", m_forceCalibRun = -1); // last run of Pb+Pb 2015
    declareProperty("ForceCalibLB", m_forceCalibLB = 814); // last LB of Pb+Pb 2015

    // The following parameters are primarily used for the "default" configuration, but also may be
    //   use to modify/tailor other configurations
    //
    declareProperty("NumSampl", m_numSample = 7);
    declareProperty("DeltaTSample", m_deltaTSample = 25);
    declareProperty("Presample", m_presample = 0);
    declareProperty("CombineDelay", m_combineDelay = false);
    declareProperty("DelayDeltaT", m_delayDeltaT = -12.5);

    declareProperty("PeakSample", m_peakSample = 11);
    declareProperty("Peak2ndDerivThresh", m_Peak2ndDerivThresh = 20);

    declareProperty("T0", m_t0 = 30);
    declareProperty("Tau1", m_tau1 = 5);
    declareProperty("Tau2", m_tau2 = 25);
    declareProperty("FixTau1", m_fixTau1 = false);
    declareProperty("FixTau2", m_fixTau2 = false);

    declareProperty("DeltaTCut", m_deltaTCut = 10);
    declareProperty("ChisqRatioCut", m_ChisqRatioCut = 10);

    declareProperty("RpdNbaselineSamples", m_rpdNbaselineSamples = 7, "Number of baseline samples; the sample equal to this number is the start of signal region");
    declareProperty("RpdEndSignalSample", m_rpdEndSignalSample = 23, "Samples before (not including) this sample are the signal region; 0 or Nsamples goes to end of window");
    declareProperty("RpdPulse2ndDerivThresh", m_rpdPulse2ndDerivThresh = -18, "Second differences less than or equal to this number indicate a pulse"); // 3 sigma = 3*3.9, 2nd difference in baseline for first-in-train bcid evts
    declareProperty("RpdPostPulseFracThresh", m_rpdPostPulseFracThresh = 0.15, "If there is a good pulse and post-pulse and size of post-pulse as a fraction of good pulse is less than or equal to this number, ignore post-pulse");
    declareProperty("RpdGoodPulseSampleStart", m_rpdGoodPulseSampleStart = 8, "Pulses before this sample are considered pre-pulses");
    declareProperty("RpdGoodPulseSampleStop", m_rpdGoodPulseSampleStop = 10, "Pulses after this sample are considered post-pulses");
    declareProperty("RpdNominalBaseline", m_rpdNominalBaseline = 100, "The global nominal baseline; used when pileup is detected");
    declareProperty("RpdPileupBaselineSumThresh", m_rpdPileupBaselineSumThresh = 684 + 3*23, "Baseline sums less than this number indicate there is NO pileup"); // 3 sigma, sum of baseline samples in first-in-train bcid evts
    declareProperty("RpdPileupBaselineStdDevThresh", m_rpdPileupBaselineStdDevThresh = 2, "Baseline standard deviations less than this number indicate there is NO pileup");
    declareProperty("RpdNNegativesAllowed", m_rpdNNegativesAllowed = 2, "Maximum number of negative ADC values after baseline and pileup subtraction allowed in signal range");
    declareProperty("RpdAdcOverflow", m_rpdAdcOverflow = 4095, "ADC values greater than or equal to this number are considered overflow");
    declareProperty("RpdSideCCalibFactors", m_rpdSideCCalibFactors = std::vector<float>(16, 1.0), "Multiplicative calibration factors to apply to RPD side C (arm 1-2) channels in reconstruction");
    declareProperty("RpdSideACalibFactors", m_rpdSideACalibFactors = std::vector<float>(16, 1.0), "Multiplicative calibration factors to apply to RPD side A (arm 8-1) channels in reconstruction");

    declareProperty("LHCRun", m_LHCRun = 3);

}

ZdcAnalysisTool::~ZdcAnalysisTool()
{
    ATH_MSG_DEBUG("Deleting ZdcAnalysisTool named " << m_name);
}

void ZdcAnalysisTool::initializeTriggerEffs(unsigned int runNumber)
{
    if (!m_doTrigEff) return;

    if (!m_zdcTriggerEfficiency)
    {
        ATH_MSG_DEBUG("Creating new ZDCTriggerEfficiency");
        m_zdcTriggerEfficiency.reset (new ZDCTriggerEfficiency());
    }

    std::string filename = PathResolverFindCalibFile( ("ZdcAnalysis/" + m_zdcTriggerEffParamsFileName) );
    ATH_MSG_DEBUG ("Found trigger config file " << filename);
    ATH_MSG_DEBUG("Opening trigger efficiency file " << filename);

    std::unique_ptr<TFile> file (TFile::Open(filename.c_str(), "READ"));
    if (file == nullptr || file->IsZombie())
    {
        ATH_MSG_WARNING("No trigger efficiencies at "  << filename);
        return;
    }

    //file->Print();

    ATH_MSG_DEBUG("Reading in trigger efficiencies");

    std::stringstream Aalpha_name;
    Aalpha_name << "A_alpha_" << runNumber;
    TSpline3* par_A_alpha = (TSpline3*)file->GetObjectChecked(Aalpha_name.str().c_str(), "TSpline3");

    if (!par_A_alpha)
    {
        ATH_MSG_WARNING("No trigger efficiencies for run number " << runNumber);
        m_doCalib = false;
    }

    std::stringstream Abeta_name;
    Abeta_name << "A_beta_" << runNumber;
    TSpline3* par_A_beta = (TSpline3*)file->GetObjectChecked(Abeta_name.str().c_str(), "TSpline3");
    std::stringstream Atheta_name;
    Atheta_name << "A_theta_" << runNumber;
    TSpline3* par_A_theta = (TSpline3*)file->GetObjectChecked(Atheta_name.str().c_str(), "TSpline3");

    std::stringstream Calpha_name;
    Calpha_name << "C_alpha_" << runNumber;
    TSpline3* par_C_alpha = (TSpline3*)file->GetObjectChecked(Calpha_name.str().c_str(), "TSpline3");
    std::stringstream Cbeta_name;
    Cbeta_name << "C_beta_" << runNumber;
    TSpline3* par_C_beta = (TSpline3*)file->GetObjectChecked(Cbeta_name.str().c_str(), "TSpline3");
    std::stringstream Ctheta_name;
    Ctheta_name << "C_theta_" << runNumber;
    TSpline3* par_C_theta = (TSpline3*)file->GetObjectChecked(Ctheta_name.str().c_str(), "TSpline3");

    std::stringstream Err_Aalpha_name;
    Err_Aalpha_name << "A_alpha_error_" << runNumber;
    TSpline3* parErr_A_alpha = (TSpline3*)file->GetObjectChecked(Err_Aalpha_name.str().c_str(), "TSpline3");
    std::stringstream Err_Abeta_name;
    Err_Abeta_name << "A_beta_error_" << runNumber;
    TSpline3* parErr_A_beta = (TSpline3*)file->GetObjectChecked(Err_Abeta_name.str().c_str(), "TSpline3");
    std::stringstream Err_Atheta_name;
    Err_Atheta_name << "A_theta_error_" << runNumber;
    TSpline3* parErr_A_theta = (TSpline3*)file->GetObjectChecked(Err_Atheta_name.str().c_str(), "TSpline3");

    std::stringstream Err_Calpha_name;
    Err_Calpha_name << "C_alpha_error_" << runNumber;
    TSpline3* parErr_C_alpha = (TSpline3*)file->GetObjectChecked(Err_Calpha_name.str().c_str(), "TSpline3");
    std::stringstream Err_Cbeta_name;
    Err_Cbeta_name << "C_beta_error_" << runNumber;
    TSpline3* parErr_C_beta = (TSpline3*)file->GetObjectChecked(Err_Cbeta_name.str().c_str(), "TSpline3");
    std::stringstream Err_Ctheta_name;
    Err_Ctheta_name << "C_theta_error_" << runNumber;
    TSpline3* parErr_C_theta = (TSpline3*)file->GetObjectChecked(Err_Ctheta_name.str().c_str(), "TSpline3");


    std::stringstream Cov_A_alpha_beta_name;
    Cov_A_alpha_beta_name << "cov_A_alpha_beta_" << runNumber;
    TSpline3* cov_A_alpha_beta = (TSpline3*)file->GetObjectChecked(Cov_A_alpha_beta_name.str().c_str(), "TSpline3");
    std::stringstream Cov_A_alpha_theta_name;
    Cov_A_alpha_theta_name << "cov_A_alpha_theta_" << runNumber;
    TSpline3* cov_A_alpha_theta = (TSpline3*)file->GetObjectChecked(Cov_A_alpha_theta_name.str().c_str(), "TSpline3");
    std::stringstream Cov_A_beta_theta_name;
    Cov_A_beta_theta_name << "cov_A_beta_theta_" << runNumber;
    TSpline3* cov_A_beta_theta = (TSpline3*)file->GetObjectChecked(Cov_A_beta_theta_name.str().c_str(), "TSpline3");

    std::stringstream Cov_C_alpha_beta_name;
    Cov_C_alpha_beta_name << "cov_C_alpha_beta_" << runNumber;
    TSpline3* cov_C_alpha_beta = (TSpline3*)file->GetObjectChecked(Cov_C_alpha_beta_name.str().c_str(), "TSpline3");
    std::stringstream Cov_C_alpha_theta_name;
    Cov_C_alpha_theta_name << "cov_C_alpha_theta_" << runNumber;
    TSpline3* cov_C_alpha_theta = (TSpline3*)file->GetObjectChecked(Cov_C_alpha_theta_name.str().c_str(), "TSpline3");
    std::stringstream Cov_C_beta_theta_name;
    Cov_C_beta_theta_name << "cov_C_beta_theta_" << runNumber;
    TSpline3* cov_C_beta_theta = (TSpline3*)file->GetObjectChecked(Cov_C_beta_theta_name.str().c_str(), "TSpline3");

    std::array<std::vector<TSpline3*>, 2> effparams;
    std::array<std::vector<TSpline3*>, 2> effparamErrors;
    std::array<std::vector<TSpline3*>, 2> effparamsCorrCoeffs;
    //side0: C; side1: A
    effparams[0] = {par_C_alpha, par_C_beta, par_C_theta};
    effparams[1] = {par_A_alpha, par_A_beta, par_A_theta};
    effparamErrors[0] = {parErr_C_alpha, parErr_C_beta, parErr_C_theta};
    effparamErrors[1] = {parErr_A_alpha, parErr_A_beta, parErr_A_theta};
    effparamsCorrCoeffs[0] = {cov_C_alpha_beta, cov_C_alpha_theta, cov_C_beta_theta};
    effparamsCorrCoeffs[1] = {cov_A_alpha_beta, cov_A_alpha_theta, cov_A_beta_theta};

    ATH_MSG_DEBUG("Trying to set parameters and errors at " << m_zdcTriggerEfficiency);

    m_zdcTriggerEfficiency->SetEffParamsAndErrors(effparams, effparamErrors);
    m_zdcTriggerEfficiency->SetEffParamCorrCoeffs(effparamsCorrCoeffs);

    return;

}

std::unique_ptr<ZDCDataAnalyzer> ZdcAnalysisTool::initializeLHCf2022()
{
  
  m_deltaTSample = 3.125;
  m_numSample = 24;

  ZDCDataAnalyzer::ZDCModuleIntArray peak2ndDerivMinSamples = {{{0, 9, 9, 9}, {0, 9, 10, 8}}};

  ZDCDataAnalyzer::ZDCModuleFloatArray peak2ndDerivMinThresholdsHG, peak2ndDerivMinThresholdsLG;
  ZDCDataAnalyzer::ZDCModuleFloatArray deltaT0CutLow, deltaT0CutHigh, chisqDivAmpCut;
  ZDCDataAnalyzer::ZDCModuleBoolArray fixTau1Arr, fixTau2Arr;
  
  ZDCDataAnalyzer::ZDCModuleFloatArray tau1 = {{{0, 1.1, 1.1, 1.1},
                                                {0, 1.1, 1.1, 1.1}}};

  ZDCDataAnalyzer::ZDCModuleFloatArray tau2 = {{{6, 5, 5, 5}, {5.5, 5.5, 5.5, 5.5}}};

  ZDCDataAnalyzer::ZDCModuleFloatArray t0HG = {{{0, 26.3, 26.5, 26.8}, {32, 32, 32, 32}}};
					       
  ZDCDataAnalyzer::ZDCModuleFloatArray t0LG = {{{0, 31.1, 28.1, 27.0}, {0, 26.6, 26.3, 25.3}}};

  for (size_t side : {0, 1}) {
    for (size_t module : {0, 1, 2, 3}) {
      fixTau1Arr[side][module] = true;
      fixTau2Arr[side][module] = false;
      
      peak2ndDerivMinThresholdsHG[side][module] = -35;
      peak2ndDerivMinThresholdsLG[side][module] = -16;
      
      deltaT0CutLow[side][module] = -10;
      deltaT0CutHigh[side][module] = 10;
      chisqDivAmpCut[side][module] = 20;
    }
  }
  
  ATH_MSG_DEBUG( "LHCF2022: delta t cut, value low = " << deltaT0CutLow[0][0] << ", high = " << deltaT0CutHigh[0][0] );
  
  ZDCDataAnalyzer::ZDCModuleFloatArray HGOverFlowADC = {{{{4000, 4000, 4000, 4000}}, {{4000, 4000, 4000, 4000}}}};
  ZDCDataAnalyzer::ZDCModuleFloatArray HGUnderFlowADC = {{{{1, 1, 1, 1}}, {{1, 1, 1, 1}}}};
  ZDCDataAnalyzer::ZDCModuleFloatArray LGOverFlowADC = {{{{4000, 4000, 4000, 4000}}, {{4000, 4000, 4000, 4000}}}};
  
  // For the LHCf run, use low gain samples 
  //
  m_lowGainOnly = true;

  //  Construct the data analyzer
  //
  std::unique_ptr<ZDCDataAnalyzer> zdcDataAnalyzer (new ZDCDataAnalyzer(MakeMessageFunction(),
									m_numSample, m_deltaTSample, 
									m_presample, "FermiExpLHCf", 
									peak2ndDerivMinSamples,
									peak2ndDerivMinThresholdsHG, 
									peak2ndDerivMinThresholdsLG, 
									m_lowGainOnly)); 

  zdcDataAnalyzer->SetPeak2ndDerivMinTolerances(2);
  zdcDataAnalyzer->SetADCOverUnderflowValues(HGOverFlowADC, HGUnderFlowADC, LGOverFlowADC);
  zdcDataAnalyzer->SetTauT0Values(fixTau1Arr, fixTau2Arr, tau1, tau2, t0HG, t0LG);
  zdcDataAnalyzer->SetCutValues(chisqDivAmpCut, chisqDivAmpCut, deltaT0CutLow, deltaT0CutHigh, deltaT0CutLow, deltaT0CutHigh);

  zdcDataAnalyzer->SetGainFactorsHGLG(0.1, 1); // a gain adjustment of unity applied to LG ADC, 0.1 to HG ADC values

  ZDCDataAnalyzer::ZDCModuleFloatArray noiseSigmasLG = {{{2, 2, 2, 2}, {2, 2, 2, 2}}};
  ZDCDataAnalyzer::ZDCModuleFloatArray noiseSigmasHG = {{{20, 20, 20, 20}, {20, 20, 20, 20}}};

  zdcDataAnalyzer->SetNoiseSigmas(noiseSigmasHG, noiseSigmasLG);

  // Enable two-pass analysis
  // 
  ZDCDataAnalyzer::ZDCModuleFloatArray peak2ndDerivMinRepassHG = {{{-12, -12, -12, -12},
								   {-12, -12, -12, -12}}};

  ZDCDataAnalyzer::ZDCModuleFloatArray peak2ndDerivMinRepassLG = {{{-8, -8, -8, -8},
								   {-8, -8, -8, -8}}};

  zdcDataAnalyzer->enableRepass(peak2ndDerivMinRepassHG, peak2ndDerivMinRepassLG);

  // Set the amplitude fit range limits
  //
  zdcDataAnalyzer->SetFitMinMaxAmpValues(5, 2, 5000, 5000);

  // disable EM module on each side
  zdcDataAnalyzer->disableModule(0, 0);
  zdcDataAnalyzer->disableModule(1, 0);


  RPDConfig rpdConfig{};
  rpdConfig.nRows = 4;
  rpdConfig.nColumns = 4;
  rpdConfig.nSamples = m_numSample;
  rpdConfig.nBaselineSamples = m_rpdNbaselineSamples;
  rpdConfig.endSignalSample = m_rpdEndSignalSample;
  rpdConfig.pulse2ndDerivThresh = m_rpdPulse2ndDerivThresh;
  rpdConfig.postPulseFracThresh = m_rpdPostPulseFracThresh;
  rpdConfig.goodPulseSampleStart = m_rpdGoodPulseSampleStart;
  rpdConfig.goodPulseSampleStop = m_rpdGoodPulseSampleStop;
  rpdConfig.nominalBaseline = m_rpdNominalBaseline;
  rpdConfig.pileupBaselineSumThresh = m_rpdPileupBaselineSumThresh;
  rpdConfig.pileupBaselineStdDevThresh = m_rpdPileupBaselineStdDevThresh;
  rpdConfig.nNegativesAllowed = m_rpdNNegativesAllowed;
  rpdConfig.AdcOverflow = m_rpdAdcOverflow;
  m_rpdDataAnalyzer.push_back(std::make_unique<RPDDataAnalyzer>(MakeMessageFunction(), "rpdC", rpdConfig, m_rpdSideCCalibFactors));
  m_rpdDataAnalyzer.push_back(std::make_unique<RPDDataAnalyzer>(MakeMessageFunction(), "rpdA", rpdConfig, m_rpdSideACalibFactors));

 return zdcDataAnalyzer;

}

std::unique_ptr<ZDCDataAnalyzer> ZdcAnalysisTool::initializepp2023()
{
  
  m_deltaTSample = 3.125;
  m_numSample = 24;

  ZDCDataAnalyzer::ZDCModuleIntArray peak2ndDerivMinSamples = {{{12, 12, 12, 12}, {12, 12, 12, 12}}};

  ZDCDataAnalyzer::ZDCModuleFloatArray peak2ndDerivMinThresholdsHG, peak2ndDerivMinThresholdsLG;
  ZDCDataAnalyzer::ZDCModuleFloatArray deltaT0CutLow, deltaT0CutHigh, chisqDivAmpCut;
  ZDCDataAnalyzer::ZDCModuleBoolArray fixTau1Arr, fixTau2Arr;
  
  ZDCDataAnalyzer::ZDCModuleFloatArray tau1 = {{{1.1, 1.1, 1.1, 1.1},
                                                {1.1, 1.1, 1.1, 1.1}}};

  ZDCDataAnalyzer::ZDCModuleFloatArray tau2 = {{{5.5, 5.5, 5.5, 5.5}, {5.5, 5.5, 5.5, 5.5}}};

  ZDCDataAnalyzer::ZDCModuleFloatArray t0HG = {{{36, 36, 36, 36}, {36, 36, 36, 36}}};
					       
  ZDCDataAnalyzer::ZDCModuleFloatArray t0LG = {{{36, 36, 36, 36}, {36, 36, 36, 36}}};

  const int deriv2ndThreshDSHG = -35;
  const int deriv2ndThreshDSLG = -10;

  const float deltaTcutLow = -10;
  const float deltaTcutHigh = 10;
  const float chisqDivAmpCutVal = 10;

  for (size_t side : {0, 1}) {
    for (size_t module : {0, 1, 2, 3}) {
      fixTau1Arr[side][module] = true;
      fixTau2Arr[side][module] = false;
      
      peak2ndDerivMinThresholdsHG[side][module] = deriv2ndThreshDSHG;
      peak2ndDerivMinThresholdsLG[side][module] = deriv2ndThreshDSLG;
      
      deltaT0CutLow[side][module] = deltaTcutLow;
      deltaT0CutHigh[side][module] = deltaTcutHigh;
      chisqDivAmpCut[side][module] = chisqDivAmpCutVal;
    }
  }
  
  ATH_MSG_DEBUG( "pp2023: delta t cut, value low = " << deltaT0CutLow[0][0] << ", high = " << deltaT0CutHigh[0][0] );
  
  ZDCDataAnalyzer::ZDCModuleFloatArray HGOverFlowADC = {{{{4000, 4000, 4000, 4000}}, {{4000, 4000, 4000, 4000}}}};
  ZDCDataAnalyzer::ZDCModuleFloatArray HGUnderFlowADC = {{{{1, 1, 1, 1}}, {{1, 1, 1, 1}}}};
  ZDCDataAnalyzer::ZDCModuleFloatArray LGOverFlowADC = {{{{4000, 4000, 4000, 4000}}, {{4000, 4000, 4000, 4000}}}};
  
  //  Construct the data analyzer
  //
  std::unique_ptr<ZDCDataAnalyzer> zdcDataAnalyzer (new ZDCDataAnalyzer(MakeMessageFunction(),
									m_numSample, m_deltaTSample, 
									m_presample, "FermiExpLHCf", 
									peak2ndDerivMinSamples,
									peak2ndDerivMinThresholdsHG, 
									peak2ndDerivMinThresholdsLG, 
									m_lowGainOnly)); 

  zdcDataAnalyzer->SetPeak2ndDerivMinTolerances(3);
  zdcDataAnalyzer->SetADCOverUnderflowValues(HGOverFlowADC, HGUnderFlowADC, LGOverFlowADC);
  zdcDataAnalyzer->SetTauT0Values(fixTau1Arr, fixTau2Arr, tau1, tau2, t0HG, t0LG);
  zdcDataAnalyzer->SetCutValues(chisqDivAmpCut, chisqDivAmpCut, deltaT0CutLow, deltaT0CutHigh, deltaT0CutLow, deltaT0CutHigh);

  zdcDataAnalyzer->SetGainFactorsHGLG(1, 10); // a gain adjustment of 10 applied to LG ADC, 1 to HG ADC values

  ZDCDataAnalyzer::ZDCModuleFloatArray noiseSigmasLG = {{{0.5, 0.5, 0.5, 0.5}, {0.5, 0.5, 0.5, 0.5}}};
  ZDCDataAnalyzer::ZDCModuleFloatArray noiseSigmasHG = {{{2, 2, 2, 2}, {2, 2, 2, 2}}};

  zdcDataAnalyzer->SetNoiseSigmas(noiseSigmasHG, noiseSigmasLG);

  // Enable two-pass analysis
  // 
  ZDCDataAnalyzer::ZDCModuleFloatArray peak2ndDerivMinRepassHG = {{{-12, -12, -12, -12},
								   {-12, -12, -12, -12}}};

  ZDCDataAnalyzer::ZDCModuleFloatArray peak2ndDerivMinRepassLG = {{{-8, -8, -8, -8},
								   {-8, -8, -8, -8}}};

  zdcDataAnalyzer->enableRepass(peak2ndDerivMinRepassHG, peak2ndDerivMinRepassLG);

  // Set the amplitude fit range limits
  //
  zdcDataAnalyzer->SetFitMinMaxAmpValues(5, 2, 5000, 5000);


  RPDConfig rpdConfig{};
  rpdConfig.nRows = 4;
  rpdConfig.nColumns = 4;
  rpdConfig.nSamples = m_numSample;
  rpdConfig.nBaselineSamples = m_rpdNbaselineSamples;
  rpdConfig.endSignalSample = m_rpdEndSignalSample;
  rpdConfig.pulse2ndDerivThresh = m_rpdPulse2ndDerivThresh;
  rpdConfig.postPulseFracThresh = m_rpdPostPulseFracThresh;
  rpdConfig.goodPulseSampleStart = m_rpdGoodPulseSampleStart;
  rpdConfig.goodPulseSampleStop = m_rpdGoodPulseSampleStop;
  rpdConfig.nominalBaseline = m_rpdNominalBaseline;
  rpdConfig.pileupBaselineSumThresh = m_rpdPileupBaselineSumThresh;
  rpdConfig.pileupBaselineStdDevThresh = m_rpdPileupBaselineStdDevThresh;
  rpdConfig.nNegativesAllowed = m_rpdNNegativesAllowed;
  rpdConfig.AdcOverflow = m_rpdAdcOverflow;
  m_rpdDataAnalyzer.push_back(std::make_unique<RPDDataAnalyzer>(MakeMessageFunction(), "rpdC", rpdConfig, m_rpdSideCCalibFactors));
  m_rpdDataAnalyzer.push_back(std::make_unique<RPDDataAnalyzer>(MakeMessageFunction(), "rpdA", rpdConfig, m_rpdSideACalibFactors));

 return zdcDataAnalyzer;

}
  
std::unique_ptr<ZDCDataAnalyzer> ZdcAnalysisTool::initializePbPb2023()
{
  // Key configuration parameters needed for the data analyzer construction                                   
  //                                                                                                          
  m_deltaTSample = 3.125;
  m_numSample = 24;
  
  const int deriv2ndThreshDSHG = -35;
  const int deriv2ndThreshDSLG = -10;
  const unsigned int peakSample = 10;

  const float deltaTcutLow = -10;
  const float deltaTcutHigh = 10;
  const float chisqDivAmpCutHGVal = 30;
  const float chisqDivAmpCutLGVal = 50;

  ZDCDataAnalyzer::ZDCModuleIntArray peak2ndDerivMinSamples;
  ZDCDataAnalyzer::ZDCModuleFloatArray peak2ndDerivMinThresholdsHG, peak2ndDerivMinThresholdsLG;
  
  ZDCDataAnalyzer::ZDCModuleFloatArray deltaT0CutLow, deltaT0CutHigh;
    ZDCDataAnalyzer::ZDCModuleFloatArray chisqDivAmpCutHG, chisqDivAmpCutLG;
  ZDCDataAnalyzer::ZDCModuleBoolArray fixTau1Arr, fixTau2Arr;
  
  ZDCDataAnalyzer::ZDCModuleFloatArray tau1 = {{{1.1, 1.1, 1.1, 1.1},
						{1.1, 1.1, 1.1, 1.1}}};
  
  ZDCDataAnalyzer::ZDCModuleFloatArray tau2 = {{{4.4, 4.7, 4.5, 4.6}, {4.8, 4.6, 4.4, 4.2}}};
  
  ZDCDataAnalyzer::ZDCModuleFloatArray t0HG = {{{31.25, 31.25, 31.25, 31.25}, {31.25, 31.25, 31.25, 31.25}}};
  ZDCDataAnalyzer::ZDCModuleFloatArray t0LG = {{{31.25, 31.25, 31.25, 31.25}, {31.25, 31.25, 31.25, 31.25}}};
    
  ATH_MSG_DEBUG( "PbPb2023: delta t cut, value low = " << deltaT0CutLow[0][0] << ", high = " << deltaT0CutHigh[0][0] );

  for (size_t side : {0, 1}) {
    for (size_t module : {0, 1, 2, 3}) {
      fixTau1Arr[side][module] = true;
      fixTau2Arr[side][module] = false;

      peak2ndDerivMinSamples[side][module] = peakSample;
      peak2ndDerivMinThresholdsHG[side][module] = deriv2ndThreshDSHG;
      peak2ndDerivMinThresholdsLG[side][module] = deriv2ndThreshDSLG;
      
      deltaT0CutLow[side][module] = deltaTcutLow;
      deltaT0CutHigh[side][module] = deltaTcutHigh;
      chisqDivAmpCutLG[side][module] = chisqDivAmpCutLGVal;
      chisqDivAmpCutHG[side][module] = chisqDivAmpCutHGVal;
    }
  }
  
  //  Construct the data analyzer                                                                             
  //                                                                                                          
  std::unique_ptr<ZDCDataAnalyzer> zdcDataAnalyzer (new ZDCDataAnalyzer(MakeMessageFunction(),
									m_numSample, m_deltaTSample,
									m_presample, "FermiExpLHCf",
									peak2ndDerivMinSamples,
									peak2ndDerivMinThresholdsHG,
									peak2ndDerivMinThresholdsLG,
									m_lowGainOnly));
  zdcDataAnalyzer->set2ndDerivStep(2);
  zdcDataAnalyzer->SetPeak2ndDerivMinTolerances(3);

  ZDCDataAnalyzer::ZDCModuleFloatArray gainsHG = {{{1, 1, 1, 1},{1, 1, 1, 1}}};
  ZDCDataAnalyzer::ZDCModuleFloatArray gainsLG = {{{9.63, 9.29, 10.08, 9.34},{9.79, 9.45, 9.94, 10.36}}};

  zdcDataAnalyzer->SetGainFactorsHGLG(gainsHG, gainsLG); // a gain adjustment of 10 applied to LG ADC, 1 to HG ADC values

  // These noise sigmas we read off from the ch_x_BaselineStdev monitoring histograms with our 
  //   final readout configuration for the Pb+Pb run by BAC on 25-09-2023
  //
  ZDCDataAnalyzer::ZDCModuleFloatArray noiseSigmasLG = {{{1.0, 1.0,  1.0,  1.0}, {1.0, 1.0, 1.0, 1.0}}};
  ZDCDataAnalyzer::ZDCModuleFloatArray noiseSigmasHG = {{{1.5, 1.7, 3, 1.7}, {1.7, 1.6, 2.2, 1.8}}};
  
  zdcDataAnalyzer->SetNoiseSigmas(noiseSigmasHG, noiseSigmasLG);

  // Now set cuts and default fit parameters                                                                  
  //                                                                                                            
  ZDCDataAnalyzer::ZDCModuleFloatArray HGOverFlowADC = {{{{3500, 3500, 3500, 3500}}, {{3500, 3500, 3500, 3500}}}};
  ZDCDataAnalyzer::ZDCModuleFloatArray HGUnderFlowADC = {{{{1, 1, 1, 1}}, {{1, 1, 1, 1}}}};
  ZDCDataAnalyzer::ZDCModuleFloatArray LGOverFlowADC = {{{{4000, 4000, 4000, 4000}}, {{4000, 4000, 4000, 4000}}}};
  
  zdcDataAnalyzer->SetADCOverUnderflowValues(HGOverFlowADC, HGUnderFlowADC, LGOverFlowADC);
  zdcDataAnalyzer->SetTauT0Values(fixTau1Arr, fixTau2Arr, tau1, tau2, t0HG, t0LG);
  zdcDataAnalyzer->SetCutValues(chisqDivAmpCutHG, chisqDivAmpCutLG, deltaT0CutLow, deltaT0CutHigh, deltaT0CutLow, deltaT0CutHigh);
  
    // Enable two-pass analysis                                                                                 
  //                                                                                                          
  ZDCDataAnalyzer::ZDCModuleFloatArray peak2ndDerivMinRepassHG = {{{-12, -12, -12, -12},
								   {-12, -12, -12, -12}}};
  
  ZDCDataAnalyzer::ZDCModuleFloatArray peak2ndDerivMinRepassLG = {{{-8, -8, -8, -8},
								   {-8, -8, -8, -8}}};
  
  zdcDataAnalyzer->enableRepass(peak2ndDerivMinRepassHG, peak2ndDerivMinRepassLG);

  // Set up non-linear corrections for the ZDC
  //
  // Non-linear corrections
  //
  std::array<std::array<std::vector<float>, 4>, 2> nonLinearCorrCoeffic;
  
  nonLinearCorrCoeffic[0][0] = {-0.039464, 0.013250, -0.003676};
  nonLinearCorrCoeffic[0][1] = {-0.050573, 0.019664, -0.004340};
  nonLinearCorrCoeffic[0][2] = {-0.052816, 0.016221, -0.003364};
  nonLinearCorrCoeffic[0][3] = {-0.014327, 0.000226, -0.000943};
  nonLinearCorrCoeffic[1][0] = {-0.059663, 0.019482, -0.004615};
  nonLinearCorrCoeffic[1][1] = {-0.036908, 0.008920, -0.002805};
  nonLinearCorrCoeffic[1][2] = {-0.046814, 0.019434, -0.004207};
  nonLinearCorrCoeffic[1][3] = {-0.060879, 0.026635, -0.004833};

  zdcDataAnalyzer->SetNonlinCorrParams(0, nonLinearCorrCoeffic);

    std::array<std::array<std::vector<float>, 4>, 2> timeCorrCoeffic;
  timeCorrCoeffic[0][0] =  {2.410188, -0.025826, 0.065930, 0.006983, -0.008872, -0.001515};
  timeCorrCoeffic[0][1] = {2.963645, -0.008860, 0.014781, 0.005681, 0.005547, 0.000878};
  timeCorrCoeffic[0][2] = {2.449642, 0.014227, 0.013660, 0.004378, 0.000030, -0.000073};
  timeCorrCoeffic[0][3] = {2.156037, 0.015385, 0.006799, 0.001156, 0.001487, 0.000534};

  timeCorrCoeffic[1][0] = {2.846303, -0.103128, 0.078904, 0.027932, -0.009349, -0.002391};
  timeCorrCoeffic[1][1] = {-0.092121, 0.000107, 0.017273, 0.007691, 0.004378, 0.000415};
  timeCorrCoeffic[1][2] = {-0.678132, 0.006028, -0.002485, -0.003520, 0.000385, 0.000387};
  timeCorrCoeffic[1][3] = {1.873812, 0.016677, 0.004617, 0.002920, 0.003346, 0.000828};

  ana->SetSlewingCoeff(ZDCPulseAnalyzer::TimingCorrLog, 0, 700, timeCorrCoeffic, timeCorrCoeffic);

  
  // Set the amplitude fit range limits                                                                       
  //                                                                                                          
  zdcDataAnalyzer->SetFitMinMaxAmpValues(5, 2, 5000, 5000);
  
  RPDConfig rpdConfig{};
  rpdConfig.nRows = 4;
  rpdConfig.nColumns = 4;
  rpdConfig.nSamples = m_numSample;
  rpdConfig.nBaselineSamples = m_rpdNbaselineSamples;
  rpdConfig.endSignalSample = m_rpdEndSignalSample;
  rpdConfig.pulse2ndDerivThresh = m_rpdPulse2ndDerivThresh;
  rpdConfig.postPulseFracThresh = m_rpdPostPulseFracThresh;
  rpdConfig.goodPulseSampleStart = m_rpdGoodPulseSampleStart;
  rpdConfig.goodPulseSampleStop = m_rpdGoodPulseSampleStop;
  rpdConfig.nominalBaseline = m_rpdNominalBaseline;
  rpdConfig.pileupBaselineSumThresh = m_rpdPileupBaselineSumThresh;
  rpdConfig.pileupBaselineStdDevThresh = m_rpdPileupBaselineStdDevThresh;
  rpdConfig.nNegativesAllowed = m_rpdNNegativesAllowed;
  rpdConfig.AdcOverflow = m_rpdAdcOverflow;
  m_rpdDataAnalyzer.push_back(std::make_unique<RPDDataAnalyzer>(MakeMessageFunction(), "rpdC", rpdConfig, m_rpdSideCCalibFactors));
  m_rpdDataAnalyzer.push_back(std::make_unique<RPDDataAnalyzer>(MakeMessageFunction(), "rpdA", rpdConfig, m_rpdSideACalibFactors));
  
  return zdcDataAnalyzer;
}

std::unique_ptr<ZDCDataAnalyzer> ZdcAnalysisTool::initializeDefault()
{
    // We rely completely on the default parameters specified in the job properties to control:
    //   # samples
    //   frequency (more precisely, time/sample)
    //   which sample to use as the pre-sample
    //   where to expact the maxim of the peak (min 2nd derivative)
    //   thresholds on the 2nd derivative for valid pulses
    //   whether to fix the tau values in the pulse fitting
    //   the default tau values
    //   the nominal T0
    //   delta T and chisq/amp cuts
    //
    //   For now, we continue to use hard-coded values for the maximum and minimum ADC values
    //   For now we also use the FermiExp pulse model.
  ZDCDataAnalyzer::ZDCModuleIntArray  peak2ndDerivMinSamples;
    ZDCDataAnalyzer::ZDCModuleFloatArray tau1, tau2, t0;
    ZDCDataAnalyzer::ZDCModuleFloatArray peak2ndDerivMinThresholdsHG, peak2ndDerivMinThresholdsLG;
    ZDCDataAnalyzer::ZDCModuleFloatArray deltaT0CutLow, deltaT0CutHigh, chisqDivAmpCut;
    ZDCDataAnalyzer::ZDCModuleBoolArray fixTau1Arr, fixTau2Arr;

    for (size_t side : {0, 1}) {
        for (size_t module : {0, 1, 2, 3}) {
            fixTau1Arr[side][module] = m_fixTau1;
            fixTau2Arr[side][module] = m_fixTau2;
            tau1[side][module] = m_tau1;
            tau2[side][module] = m_tau2;

            peak2ndDerivMinSamples[side][module] = m_peakSample;
            peak2ndDerivMinThresholdsHG[side][module] = -m_Peak2ndDerivThresh;
            peak2ndDerivMinThresholdsLG[side][module] = -m_Peak2ndDerivThresh / 2;

            t0[side][module] = m_t0;
            deltaT0CutLow[side][module] = -m_deltaTCut;
            deltaT0CutHigh[side][module] = m_deltaTCut;
            chisqDivAmpCut[side][module] = m_ChisqRatioCut;
        }
    }

    ATH_MSG_DEBUG( "Default: delta t cut, value low = " << deltaT0CutLow[0][0] << ", high = " << deltaT0CutHigh[0][0] );

    ZDCDataAnalyzer::ZDCModuleFloatArray HGOverFlowADC = {{{{800, 800, 800, 800}}, {{800, 800, 800, 800}}}};
    ZDCDataAnalyzer::ZDCModuleFloatArray HGUnderFlowADC = {{{{10, 10, 10, 10}}, {{10, 10, 10, 10}}}};
    ZDCDataAnalyzer::ZDCModuleFloatArray LGOverFlowADC = {{{{1020, 1020, 1020, 1020}}, {{1020, 1020, 1020, 1020}}}};

    //  Construct the data analyzer
    //
    std::unique_ptr<ZDCDataAnalyzer> zdcDataAnalyzer (new ZDCDataAnalyzer(MakeMessageFunction(), m_numSample, m_deltaTSample, m_presample, "FermiExp", peak2ndDerivMinSamples,
            peak2ndDerivMinThresholdsHG, peak2ndDerivMinThresholdsLG, m_lowGainOnly));

    zdcDataAnalyzer->SetADCOverUnderflowValues(HGOverFlowADC, HGUnderFlowADC, LGOverFlowADC);
    zdcDataAnalyzer->SetTauT0Values(fixTau1Arr, fixTau2Arr, tau1, tau2, t0, t0);
    zdcDataAnalyzer->SetCutValues(chisqDivAmpCut, chisqDivAmpCut, deltaT0CutLow, deltaT0CutHigh, deltaT0CutLow, deltaT0CutHigh);

    if (m_combineDelay) {
        ZDCDataAnalyzer::ZDCModuleFloatArray defaultPedestalShifts = {{{{0, 0, 0, 0}}, {{0, 0, 0, 0}}}};

        zdcDataAnalyzer->enableDelayed(m_delayDeltaT, defaultPedestalShifts);
    }

    return zdcDataAnalyzer;
}

std::unique_ptr<ZDCDataAnalyzer> ZdcAnalysisTool::initializePbPb2015G4()
{
    // ref. https://indico.cern.ch/event/849143/contributions/3568263/attachments/1909759/3155352/ZDCWeekly_20190917_PengqiYin.pdf
    ZDCDataAnalyzer::ZDCModuleIntArray peak2ndDerivMinSamples;
    ZDCDataAnalyzer::ZDCModuleFloatArray peak2ndDerivMinThresholdsHG, peak2ndDerivMinThresholdsLG;
    ZDCDataAnalyzer::ZDCModuleFloatArray chisqDivAmpCut;
    ZDCDataAnalyzer::ZDCModuleBoolArray  fixTau1Arr, fixTau2Arr;

    const int   peakSample = 4;
    const float peak2ndDerivThreshHG = -12;
    const float peak2ndDerivThreshLG = -10;
    ZDCDataAnalyzer::ZDCModuleFloatArray tau1Arr = {{{4.000, 4.000, 4.000, 4.000},
                                                     {4.000, 4.000, 4.000, 4.000}}};
    ZDCDataAnalyzer::ZDCModuleFloatArray tau2Arr = {{{25.36, 25.05, 25.43, 25.60},
                                                     {25.11, 25.08, 25.18, 25.48}}};

    ZDCDataAnalyzer::ZDCModuleFloatArray t0HG = {{{57.31, 57.28, 57.30, 57.28},
                                                  {57.28, 57.29, 57.31, 57.33}}};
    ZDCDataAnalyzer::ZDCModuleFloatArray t0LG = {{{57.31, 57.28, 57.30, 57.28},
                                                  {57.28, 57.29, 57.31, 57.33}}};

    // Delta T0 cut
    ZDCDataAnalyzer::ZDCModuleFloatArray DeltaT0CutLowHG  = {{{-10, -10, -10, -10}, {-10, -10, -10, -10}}};
    ZDCDataAnalyzer::ZDCModuleFloatArray DeltaT0CutHighHG = {{{ 10,  10,  10,  10}, { 10,  10,  10,  10}}};
    ZDCDataAnalyzer::ZDCModuleFloatArray DeltaT0CutLowLG  = {{{-10, -10, -10, -10}, {-10, -10, -10, -10}}};
    ZDCDataAnalyzer::ZDCModuleFloatArray DeltaT0CutHighLG = {{{ 10,  10,  10,  10}, { 10,  10,  10,  10}}};

    for (size_t side : {0, 1}) {
        for (size_t module : {0, 1, 2, 3}) {
            fixTau1Arr[side][module] = true;
            fixTau2Arr[side][module] = true;

            peak2ndDerivMinSamples[side][module] = peakSample;
            peak2ndDerivMinThresholdsHG[side][module] = peak2ndDerivThreshHG;
            peak2ndDerivMinThresholdsLG[side][module] = peak2ndDerivThreshLG;

            chisqDivAmpCut[side][module] = 15;
        }
    }

    ZDCDataAnalyzer::ZDCModuleFloatArray HGOverFlowADC  = {{{{ 800,  800,  800,  800}}, {{ 800,  800,  800,  800}}}};
    ZDCDataAnalyzer::ZDCModuleFloatArray HGUnderFlowADC = {{{{  10,   10,   10,   10}}, {{  10,   10,   10,   10}}}};
    ZDCDataAnalyzer::ZDCModuleFloatArray LGOverFlowADC  = {{{{1020, 1020, 1020, 1020}}, {{1020, 1020, 1020, 1020}}}};

    m_deltaTSample = 12.5;

    std::unique_ptr<ZDCDataAnalyzer> zdcDataAnalyzer (new ZDCDataAnalyzer(MakeMessageFunction(), 7, m_deltaTSample, 0, "FermiExp", peak2ndDerivMinSamples,
        peak2ndDerivMinThresholdsHG, peak2ndDerivMinThresholdsLG, m_lowGainOnly));

    // Open up tolerances on the position of the peak for now
    //
    zdcDataAnalyzer->SetPeak2ndDerivMinTolerances(1);

    zdcDataAnalyzer->SetADCOverUnderflowValues(HGOverFlowADC, HGUnderFlowADC, LGOverFlowADC);
    zdcDataAnalyzer->SetTauT0Values(fixTau1Arr, fixTau2Arr, tau1Arr, tau2Arr, t0HG, t0LG);
    zdcDataAnalyzer->SetCutValues(chisqDivAmpCut, chisqDivAmpCut, DeltaT0CutLowHG, DeltaT0CutHighHG, DeltaT0CutLowLG, DeltaT0CutHighLG);

    zdcDataAnalyzer->SetFitTimeMax(85);
    zdcDataAnalyzer->SetSaveFitFunc(false);

    return zdcDataAnalyzer;
}

std::unique_ptr<ZDCDataAnalyzer> ZdcAnalysisTool::initializepPb2016()
{
    //
    //   For now, we continue to use hard-coded values for the maximum and minimum ADC values
    //   For now we also use the FermiExp pulse model.

    ZDCDataAnalyzer::ZDCModuleIntArray peak2ndDerivMinSamples;
    ZDCDataAnalyzer::ZDCModuleFloatArray peak2ndDerivMinThresholdsHG, peak2ndDerivMinThresholdsLG;
    ZDCDataAnalyzer::ZDCModuleFloatArray chisqDivAmpCut;
    ZDCDataAnalyzer::ZDCModuleBoolArray  fixTau1Arr, fixTau2Arr;

    //  For now we allow the tau values to be controlled by the job properties until they are better determined
    //
    const int peakSample = 5;
    const float peak2ndDerivThreshHG = -12;
    const float peak2ndDerivThreshLG = -10;
    ZDCDataAnalyzer::ZDCModuleFloatArray tau1Arr = {{{4.000, 3.380, 3.661, 3.679},
                                                     {4.472, 4.656, 3.871, 4.061}
                                                    }};

    ZDCDataAnalyzer::ZDCModuleFloatArray tau2Arr = {{{22,    24.81, 24.48, 24.45},
                                                     {24.17, 24.22, 25.46, 24.45}
                                                    }};

    ZDCDataAnalyzer::ZDCModuleFloatArray t0HG = {{{70.00, 72.74, 73.09, 72.25},
                                                  {75.11, 74.94, 73.93, 74.45}
                                                }};
    ZDCDataAnalyzer::ZDCModuleFloatArray t0LG = {{{70.00, 73.41, 74.27, 73.30},
                                                  {76.28, 76.07, 74.98, 76.54}
                                                }};

    // Delta T0 cut
    ZDCDataAnalyzer::ZDCModuleFloatArray DeltaT0CutLowHG  = {{{ -6, -5, -5, -5}, {-5, -5, -5, -5}}};
    ZDCDataAnalyzer::ZDCModuleFloatArray DeltaT0CutHighHG = {{{8, 8, 8, 11}, {8, 10, 8, 12}}};
    ZDCDataAnalyzer::ZDCModuleFloatArray DeltaT0CutLowLG  = {{{ -6, -5, -5, -5}, {-5, -5, -5, -5}}};
    ZDCDataAnalyzer::ZDCModuleFloatArray DeltaT0CutHighLG = {{{8, 8, 8, 11}, {8, 10, 8, 12}}};


    for (size_t side : {0, 1}) {
        for (size_t module : {0, 1, 2, 3}) {
            fixTau1Arr[side][module] = true;
            fixTau2Arr[side][module] = true;

            peak2ndDerivMinSamples[side][module] = peakSample;
            peak2ndDerivMinThresholdsHG[side][module] = peak2ndDerivThreshHG;
            peak2ndDerivMinThresholdsLG[side][module] = peak2ndDerivThreshLG;

            chisqDivAmpCut[side][module] = 15;
        }
    }

    ZDCDataAnalyzer::ZDCModuleFloatArray HGOverFlowADC = {{{{800, 800, 800, 800}}, {{800, 800, 800, 800}}}};
    ZDCDataAnalyzer::ZDCModuleFloatArray HGUnderFlowADC = {{{{10, 10, 10, 10}}, {{10, 10, 10, 10}}}};
    ZDCDataAnalyzer::ZDCModuleFloatArray LGOverFlowADC = {{{{1020, 1020, 1020, 1020}}, {{1020, 1020, 1020, 1020}}}};

    std::array<std::array<std::vector<float>, 4>, 2> slewingParamsHG, slewingParamsLG;
    // ref. https://indico.cern.ch/event/849143/contributions/3568263/attachments/1909759/3155352/ZDCWeekly_20190917_PengqiYin.pdf
    slewingParamsHG[0][0] = {0, 0, 0, 0};
    slewingParamsHG[0][1] = { -4.780244e-01, -7.150874e-02, 4.614585e-02,  8.015731e-04};
    slewingParamsHG[0][2] = { -5.253412e-01, -5.718167e-02, 5.243121e-02,  2.128398e-03};
    slewingParamsHG[0][3] = { -5.773952e-01, -5.687478e-02, 4.564267e-02,  1.462294e-03};

    slewingParamsHG[1][0] = { 7.105115e-01, -3.686143e-02, 7.727447e-02,  5.924152e-03};
    slewingParamsHG[1][1] = { 4.052120e-02,  4.450686e-03, 8.031615e-02,  4.038097e-03};
    slewingParamsHG[1][2] = { 3.389476e-02, -2.056782e-02, 4.805321e-02, -2.627999e-03};
    slewingParamsHG[1][3] = { 2.069765e-01, -2.890419e-02, 6.084375e-02,  3.742011e-03};

    slewingParamsLG[0][0] = {0, 0, 0, 0};
    slewingParamsLG[0][1] = { -1.632547e+00, -4.827813e-01, -1.379131e-01, -2.522607e-02};
    slewingParamsLG[0][2] = { -7.254288e+00, -5.454064e+00, -1.619126e+00, -1.739665e-01};
    slewingParamsLG[0][3] = { -1.548400e+01, -1.277708e+01, -3.729333e+00, -3.700458e-01};

    slewingParamsLG[1][0] = {  1.142767e-01, -3.608906e-02,  9.642735e-02, -3.097043e-03};
    slewingParamsLG[1][1] = { -5.615388e-01, -1.655047e-02,  8.327350e-02, -4.231348e-03};
    slewingParamsLG[1][2] = { -7.370728e-01, -2.887482e-02,  8.293875e-02, -4.482743e-03};
    slewingParamsLG[1][3] = { -1.270636e+00, -2.791777e-01, -5.807295e-02, -2.332612e-02};

    //  Construct the data analyzer
    //
    //  We adopt hard-coded values for the number of samples and the frequency which we kept fixed for all physics data
    //
    std::unique_ptr<ZDCDataAnalyzer> zdcDataAnalyzer (new ZDCDataAnalyzer(MakeMessageFunction(), 7, 25, 0, "FermiExpLinear", peak2ndDerivMinSamples,
            peak2ndDerivMinThresholdsHG, peak2ndDerivMinThresholdsLG, m_lowGainOnly));

    // Open up tolerances on the position of the peak for now
    //
    zdcDataAnalyzer->SetPeak2ndDerivMinTolerances(1);

    // We alwyas disable the 12EM (sideC) module which was not present (LHCf)
    //
    zdcDataAnalyzer->disableModule(0, 0);

    zdcDataAnalyzer->SetADCOverUnderflowValues(HGOverFlowADC, HGUnderFlowADC, LGOverFlowADC);
    zdcDataAnalyzer->SetTauT0Values(fixTau1Arr, fixTau2Arr, tau1Arr, tau2Arr, t0HG, t0LG);
    zdcDataAnalyzer->SetCutValues(chisqDivAmpCut, chisqDivAmpCut, DeltaT0CutLowHG, DeltaT0CutHighHG, DeltaT0CutLowLG, DeltaT0CutHighLG);

    // We allow the combineDelay to be controlled by the properties
    //
    //  if (m_combineDelay) {
    m_combineDelay = true;
    ZDCDataAnalyzer::ZDCModuleFloatArray defaultPedestalShifts = {{{{0, 0, 0, 0}}, {{0, 0, 0, 0}}}};

    zdcDataAnalyzer->enableDelayed(-12.5, defaultPedestalShifts);
    zdcDataAnalyzer->SetFitTimeMax(140); // This restrict the fit range of the pulse fitting
    zdcDataAnalyzer->SetSaveFitFunc(false);
    zdcDataAnalyzer->SetTimingCorrParams(ZDCPulseAnalyzer::TimingCorrLin, 500, 100, 
					 slewingParamsHG, slewingParamsLG); // add time slewing correction Sep 17 2019 Bill
    // ref. https://indico.cern.ch/event/849143/contributions/3568263/attachments/1909759/3155352/ZDCWeekly_20190917_PengqiYin.pdf

    return zdcDataAnalyzer;
}

std::unique_ptr<ZDCDataAnalyzer> ZdcAnalysisTool::initializePbPb2018()
{
    ZDCDataAnalyzer::ZDCModuleIntArray peak2ndDerivMinSamples;
    ZDCDataAnalyzer::ZDCModuleFloatArray peak2ndDerivMinThresholdsHG, peak2ndDerivMinThresholdsLG, peak2ndDerivMinRepassHG, peak2ndDerivMinRepassLG;
    ZDCDataAnalyzer::ZDCModuleFloatArray chisqDivAmpCut;
    ZDCDataAnalyzer::ZDCModuleBoolArray fixTau1Arr, fixTau2Arr;

    static constexpr int peakSample = 5;
    static constexpr float peak2ndDerivThreshHG = -35;
    static constexpr float peak2ndDerivThreshLG = -20;
    static constexpr float peak2ndDerivRepassHG = -10;
    static constexpr float peak2ndDerivRepassLG = -6;

    ZDCDataAnalyzer::ZDCModuleFloatArray tau1Arr = {{{3.877, 3.998, 3.821, 3.858},
                                                     {4.296, 4.064, 3.497, 3.642}
                                                     }};

    ZDCDataAnalyzer::ZDCModuleFloatArray tau2Arr = {{{24.40, 25.28, 25.66, 24.12},
                                                     {24.42, 24.99, 25.72, 25.29}
                                                     }};

    ZDCDataAnalyzer::ZDCModuleFloatArray t0HG = {{{70.51, 70.57, 70.13, 69.98},
                                                  {74.18, 72.79, 71.77, 72.62}
                                                  }};
    ZDCDataAnalyzer::ZDCModuleFloatArray t0LG = {{{70.70, 70.78, 70.76, 70.91},
                                                  {75.16, 73.71, 72.25, 73.61}
                                                  }};

    ZDCDataAnalyzer::ZDCModuleFloatArray moduleAmpFractionLG = {{{0.2760, 0.3045, 0.2369, 0.1826},
                                                                 {0.3216, 0.2593, 0.2511, 0.1680}
                                                                 }};

    // Delta T0 cut
    ZDCDataAnalyzer::ZDCModuleFloatArray DeltaT0CutLowHG = {{{ -6, -5, -5, -5}, {-5, -5, -5, -5}}};
    ZDCDataAnalyzer::ZDCModuleFloatArray DeltaT0CutHighHG = {{{8, 8, 8, 11}, {8, 10, 8, 12}}};
    ZDCDataAnalyzer::ZDCModuleFloatArray DeltaT0CutLowLG = {{{ -6, -5, -5, -5}, {-5, -5, -5, -5}}};
    ZDCDataAnalyzer::ZDCModuleFloatArray DeltaT0CutHighLG = {{{8, 8, 8, 11}, {8, 10, 8, 12}}};

    for (size_t side : {0, 1}) {
        for (size_t module : {0, 1, 2, 3}) {
            fixTau1Arr[side][module] = true;
            fixTau2Arr[side][module] = true;

            peak2ndDerivMinSamples[side][module] = peakSample;
            peak2ndDerivMinThresholdsHG[side][module] = peak2ndDerivThreshHG;
            peak2ndDerivMinThresholdsLG[side][module] = peak2ndDerivThreshLG;
            peak2ndDerivMinRepassHG    [side][module] = peak2ndDerivRepassHG;
            peak2ndDerivMinRepassLG    [side][module] = peak2ndDerivRepassLG;

            chisqDivAmpCut[side][module] = 15;
        }
    }

    ZDCDataAnalyzer::ZDCModuleFloatArray HGOverFlowADC = {{{{800, 800, 800, 800}}, {{800, 800, 800, 800}}}};
    ZDCDataAnalyzer::ZDCModuleFloatArray HGUnderFlowADC = {{{{10, 10, 10, 10}}, {{10, 10, 10, 10}}}};
    ZDCDataAnalyzer::ZDCModuleFloatArray LGOverFlowADC = {{{{1020, 1020, 1020, 1020}}, {{1020, 1020, 1020, 1020}}}};

    ZDCDataAnalyzer::ZDCModuleFloatArray noiseSigmasLG = {{{1, 1, 1, 1}, {1, 1, 1, 1}}};
    ZDCDataAnalyzer::ZDCModuleFloatArray noiseSigmasHG = {{{12, 12, 12, 12}, {12, 12, 12, 12}}};
    
    
    std::array<std::array<std::vector<float>, 4>, 2> slewingParamsHG, slewingParamsLG;
    // ref. https://indico.cern.ch/event/849143/contributions/3568263/attachments/1909759/3155352/ZDCWeekly_20190917_PengqiYin.pdf
    slewingParamsHG[0][0] = { -1.335560e-01, -6.071869e-03, 5.858193e-02,  2.473300e-03};
    slewingParamsHG[0][1] = { -1.223062e-01, -4.379469e-02, 4.452285e-02,  2.130210e-03};
    slewingParamsHG[0][2] = { -1.021415e-01, -4.254239e-02, 4.939866e-02,  3.849738e-03};
    slewingParamsHG[0][3] = { -8.234056e-02, -3.938803e-02, 4.689029e-02,  2.784816e-03};

    slewingParamsHG[1][0] = { -1.640979e-01, -2.780350e-02, 5.755065e-02, -4.244651e-04};
    slewingParamsHG[1][1] = { -1.422324e-01,  2.663803e-02, 7.295366e-02,  3.740496e-03};
    slewingParamsHG[1][2] = { -9.858124e-02, -2.426132e-02, 4.895967e-02,  2.291393e-03};
    slewingParamsHG[1][3] = { -1.070401e-01, -2.256383e-03, 5.833770e-02,  2.255208e-03};

    slewingParamsLG[0][0] = { -2.588446e-01, -3.241086e-02, 7.828661e-02,  1.945547e-03};
    slewingParamsLG[0][1] = { -3.112495e-01, -7.419508e-02, 6.825776e-02,  2.148860e-03};
    slewingParamsLG[0][2] = { -3.470650e-01, -5.836748e-02, 6.204396e-02,  1.550421e-03};
    slewingParamsLG[0][3] = { -4.485435e-01, -4.603790e-02, 5.944799e-02, -1.174585e-03};

    slewingParamsLG[1][0] = { -3.291676e-01, -4.023732e-02, 8.608755e-02, -3.958167e-03};
    slewingParamsLG[1][1] = { -2.608969e-01, -2.129786e-03, 6.930791e-02, -4.141910e-03};
    slewingParamsLG[1][2] = { -2.505712e-01, -2.195804e-02, 5.137261e-02, -4.058378e-03};
    slewingParamsLG[1][3] = { -5.083206e-01,  3.776601e-02, 1.284275e-01,  1.014067e-02};

    //  Construct the data analyzer
    //
    //  We adopt hard-coded values for the number of samples and the frequency which we kept fixed for all physics data
    //
    std::unique_ptr<ZDCDataAnalyzer> zdcDataAnalyzer = std::make_unique<ZDCDataAnalyzer>(MakeMessageFunction(), 7, 25, 0, "FermiExpLinear", peak2ndDerivMinSamples, // presample index changed to zero 4/6/19
            peak2ndDerivMinThresholdsHG, peak2ndDerivMinThresholdsLG, m_lowGainOnly);

    // Open up tolerances on the position of the peak for now
    //
    zdcDataAnalyzer->SetPeak2ndDerivMinTolerances(1);

    // We alwyas disable the 12EM (sideC) module which was not present (LHCf)
    //

    zdcDataAnalyzer->SetADCOverUnderflowValues(HGOverFlowADC, HGUnderFlowADC, LGOverFlowADC);
    zdcDataAnalyzer->SetTauT0Values(fixTau1Arr, fixTau2Arr, tau1Arr, tau2Arr, t0HG, t0LG);
    zdcDataAnalyzer->SetModuleAmpFractionLG(moduleAmpFractionLG);   // fraction calculation for LGOverflows     added Nov 23, 2020
    zdcDataAnalyzer->SetCutValues(chisqDivAmpCut, chisqDivAmpCut, DeltaT0CutLowHG, DeltaT0CutHighHG, DeltaT0CutLowLG, DeltaT0CutHighLG);

    // We allow the combineDelay to be controlled by the properties
    //
    m_combineDelay = true;
    ZDCDataAnalyzer::ZDCModuleFloatArray defaultPedestalShifts = {{{{0, 0, 0, 0}}, {{0, 0, 0, 0}}}};

    //  We use per-module delays to handle the delayed-undelayed swap on EMC
    //
    ZDCDataAnalyzer::ZDCModuleFloatArray delayDeltaTs = {{{{12.5, -12.5, -12.5, -12.5}},
            {{ -12.5, -12.5, -12.5, -12.5}}
        }
    };

    zdcDataAnalyzer->enableDelayed(delayDeltaTs, defaultPedestalShifts);
    zdcDataAnalyzer->SetFitTimeMax(140); // This restrict the fit range of the pulse fitting, requested by BAC 4/6/19
    zdcDataAnalyzer->SetSaveFitFunc(false);
    zdcDataAnalyzer->enableRepass(peak2ndDerivMinRepassHG, peak2ndDerivMinRepassLG); // add repass as default Jul 21 2020 Bill
    zdcDataAnalyzer->SetTimingCorrParams(ZDCPulseAnalyzer::TimingCorrLin, 500, 100,
					 slewingParamsHG, slewingParamsLG); // add time slewing correction Sep 17 2019 Bill
    // ref. https://indico.cern.ch/event/849143/contributions/3568263/attachments/1909759/3155352/ZDCWeekly_20190917_PengqiYin.pdf

    zdcDataAnalyzer->SetNoiseSigmas(noiseSigmasHG, noiseSigmasLG);

    return zdcDataAnalyzer;
}

void ZdcAnalysisTool::initialize40MHz()
{
    // We have a complete configuration and so we override all of the default parameters
    //

    ZDCDataAnalyzer::ZDCModuleFloatArray tau1 = {{{{4.2, 3.8, 5.2, 5.0}},
            {{5.0, 3.7, 3.5, 3.5}}
        }
    };

    // identical to 80 MHz -- is this right
    ZDCDataAnalyzer::ZDCModuleFloatArray tau2 = {{{{20.0, 20.4, 18.9, 20.8}},
            {{19.1, 21.9, 22.6, 23.4}}
        }
    };

    ZDCDataAnalyzer::ZDCModuleIntArray peak2ndDerivMinSamples = {{{{1, 1, 2, 1}},
            {{1, 1, 1, 1}}
        }
    };

    ZDCDataAnalyzer::ZDCModuleFloatArray peak2ndDerivMinThresholdsHG = {{{{ -8, -8, -8, -8}},
            {{ -8, -8, -8, -8}}
        }
    };

    ZDCDataAnalyzer::ZDCModuleFloatArray peak2ndDerivMinThresholdsLG = {{{{ -4, -4, -4, -4}},
            {{ -4, -4, -4, -4}}
        }
    };

    ZDCDataAnalyzer::ZDCModuleFloatArray HGOverFlowADC = {{{{800, 800, 800, 800}}, {{800, 800, 800, 800}}}};
    ZDCDataAnalyzer::ZDCModuleFloatArray HGUnderFlowADC = {{{{10, 10, 10, 10}}, {{10, 10, 10, 10}}}};
    ZDCDataAnalyzer::ZDCModuleFloatArray LGOverFlowADC = {{{{1020, 1020, 1020, 1020}}, {{1020, 1020, 1020, 1020}}}};

    // Set Tau and nominal timing offsets
    ZDCDataAnalyzer::ZDCModuleBoolArray fixTau1Arr, fixTau2Arr;

    bool fixTau1 = true;
    bool fixTau2 = true;

    for (size_t side : {0, 1}) {
        for (size_t module : {0, 1, 2, 3}) {
            fixTau1Arr[side][module] = fixTau1;
            fixTau2Arr[side][module] = fixTau2;
        }
    }

    ZDCDataAnalyzer::ZDCModuleFloatArray t0HG = {{{{53.942, 49.887, 59.633, 46.497}},
            {{46.314, 42.267, 50.327, 41.605}}
        }
    };
    ZDCDataAnalyzer::ZDCModuleFloatArray t0LG = {{{{51.771, 47.936, 57.438, 44.191}},
            {{44.295, 41.755, 48.081, 40.175}}
        }
    };


    ZDCDataAnalyzer::ZDCModuleFloatArray chisqDivAmpCutHG = {{{{10, 10, 10, 10}}, {{10, 10, 10, 10}}}};
    ZDCDataAnalyzer::ZDCModuleFloatArray chisqDivAmpCutLG = {{{{10, 10, 10, 10}}, {{10, 10, 10, 10}}}};
    ZDCDataAnalyzer::ZDCModuleFloatArray DeltaT0CutLowHG = {{{{ -6, -5, -5, -5}}, {{ -5, -5, -5, -5}}}};
    ZDCDataAnalyzer::ZDCModuleFloatArray DeltaT0CutHighHG = {{{{8, 8, 8, 11}}, {{8, 10, 8, 12}}}};
    ZDCDataAnalyzer::ZDCModuleFloatArray DeltaT0CutLowLG = {{{{ -6, -5, -5, -5}}, {{ -5, -5, -5, -5}}}};
    ZDCDataAnalyzer::ZDCModuleFloatArray DeltaT0CutHighLG = {{{{8, 8, 8, 11}}, {{8, 10, 8, 12}}}};

    std::array<std::array<std::vector<float>, 4>, 2> slewingParamsHG, slewingParamsLG;

    slewingParamsHG[0][0] = {0, -7.904e-02, 4.686e-02, 1.530e-03 };
    slewingParamsHG[0][1] = {0, 2.250e-02, 4.732e-02, 6.050e-03  };
    slewingParamsHG[0][2] = {0, 4.388e-02, 6.707e-02, -5.526e-05 };
    slewingParamsHG[0][3] = {0, 1.205e-01, 2.726e-02, 2.610e-03  };

    slewingParamsHG[1][0] = {0, 6.861e-02, 5.175e-03, -9.018e-04  };
    slewingParamsHG[1][1] = {0, 3.855e-01, -4.442e-02, -2.022e-02 };
    slewingParamsHG[1][2] = {0, -4.337e-03, 3.841e-02, 4.661e-03  };
    slewingParamsHG[1][3] = {0, 3.623e-01, -3.882e-02, -1.805e-02 };

    slewingParamsLG[0][0] = {0, 1.708e-02, 7.929e-02, 5.079e-03   };
    slewingParamsLG[0][1] = {0, 1.406e-01, 1.209e-01, -1.922e-04  };
    slewingParamsLG[0][2] = {0, 1.762e-01, 1.118e-01, 1.679e-04   };
    slewingParamsLG[0][3] = {0, 1.361e-02, -2.685e-02, -4.168e-02 };

    slewingParamsLG[1][0] = {0, 1.962e-01, -5.025e-03, -2.001e-02 };
    slewingParamsLG[1][1] = {0, 3.258e-01, 1.229e-02, -2.925e-02  };
    slewingParamsLG[1][2] = {0, 1.393e-01, 8.113e-02, -2.594e-03  };
    slewingParamsLG[1][3] = {0, 1.939e-01, 2.188e-02, -5.579e-02  };

    std::array<std::array<std::vector<float>, 4>, 2> moduleHGNonLinCorr;
    moduleHGNonLinCorr[0][0] = { -3.76800e-02, 4.63597e-02};
    moduleHGNonLinCorr[0][1] = { -1.02185e-01, -1.17548e-01};
    moduleHGNonLinCorr[0][2] = { -8.78451e-02, -1.52174e-01};
    moduleHGNonLinCorr[0][3] = { -1.04835e-01, -1.96514e-01};
    moduleHGNonLinCorr[1][0] = { -6.83115e-02, 3.57802e-02};
    moduleHGNonLinCorr[1][1] = { -1.08162e-01, -1.91413e-01};
    moduleHGNonLinCorr[1][2] = { -7.82514e-02, -1.21218e-01};
    moduleHGNonLinCorr[1][3] = { -2.34354e-02, -2.52033e-01};

    m_zdcDataAnalyzer_40MHz.reset (new ZDCDataAnalyzer(MakeMessageFunction(), 7, 25, 0, "FermiExp", peak2ndDerivMinSamples,
                                   peak2ndDerivMinThresholdsHG, peak2ndDerivMinThresholdsLG, m_lowGainOnly));

    m_zdcDataAnalyzer_40MHz->SetADCOverUnderflowValues(HGOverFlowADC, HGUnderFlowADC, LGOverFlowADC);
    m_zdcDataAnalyzer_40MHz->SetTauT0Values(fixTau1Arr, fixTau2Arr, tau1, tau2, t0HG, t0LG);
    m_zdcDataAnalyzer_40MHz->SetCutValues(chisqDivAmpCutHG, chisqDivAmpCutLG, DeltaT0CutLowHG, DeltaT0CutHighHG, DeltaT0CutLowLG, DeltaT0CutHighLG);
    m_zdcDataAnalyzer_40MHz->SetTimingCorrParams(ZDCPulseAnalyzer::TimingCorrLin, 500, 100,
						 slewingParamsHG, slewingParamsLG);
    m_zdcDataAnalyzer_40MHz->SetNonlinCorrParams(500, moduleHGNonLinCorr);
    m_zdcDataAnalyzer_40MHz->SetSaveFitFunc(false);

}

void ZdcAnalysisTool::initialize80MHz()
{
    // We have a complete configuration and so we override all of the default parameters
    //

    m_peak2ndDerivMinSamples = {{{{3, 2, 3, 2}},
            {{2, 2, 2, 2}}
        }
    };

    m_peak2ndDerivMinThresholdsHG = {{{{ -8, -8, -8, -8}},
            {{ -8, -8, -8, -8}}
        }
    };

    m_peak2ndDerivMinThresholdsLG = {{{{ -4, -4, -4, -4}},
            {{ -4, -4, -4, -4}}
        }
    };

    ZDCDataAnalyzer::ZDCModuleFloatArray HGOverFlowADC = {{{{800, 800, 800, 800}}, {{800, 800, 800, 800}}}};
    ZDCDataAnalyzer::ZDCModuleFloatArray HGUnderFlowADC = {{{{10, 10, 10, 10}}, {{10, 10, 10, 10}}}};
    ZDCDataAnalyzer::ZDCModuleFloatArray LGOverFlowADC = {{{{950, 950, 950, 950}}, {{950, 950, 950, 950}}}};

    // Set Tau and nominal timing offsets
    ZDCDataAnalyzer::ZDCModuleBoolArray fixTau1Arr, fixTau2Arr;

    bool fixTau1 = true;
    bool fixTau2 = true;

    for (size_t side : {0, 1}) {
        for (size_t module : {0, 1, 2, 3}) {
            fixTau1Arr[side][module] = fixTau1;
            fixTau2Arr[side][module] = fixTau2;
        }
    }

    ZDCDataAnalyzer::ZDCModuleFloatArray tau1 = {{{{3.9, 3.4, 4.1, 4.2}},
            {{4.2, 3.6, 3.3, 3.4}}
        }
    };

    ZDCDataAnalyzer::ZDCModuleFloatArray tau2 = {{{{20.0, 20.4, 18.9, 20.8}},
            {{19.1, 21.9, 22.6, 23.4}}
        }
    };

    ZDCDataAnalyzer::ZDCModuleFloatArray t0HG = {{{{44.24, 40.35, 49.3, 36.0}},
            {{36.0, 31.1, 40.75, 30.5}}
        }
    };

    ZDCDataAnalyzer::ZDCModuleFloatArray t0LG = {{{{42.65, 38.5, 47.4, 34}},
            {{33.7, 29.9, 39.0, 29.3}}
        }
    };

    ZDCDataAnalyzer::ZDCModuleFloatArray chisqDivAmpCutHG = {{{{10, 10, 10, 10}}, {{10, 10, 10, 10}}}};
    ZDCDataAnalyzer::ZDCModuleFloatArray chisqDivAmpCutLG = {{{{10, 10, 10, 10}}, {{10, 10, 10, 10}}}};
    ZDCDataAnalyzer::ZDCModuleFloatArray DeltaT0CutLowHG = {{{{ -6, -5, -5, -5}}, {{ -5, -5, -5, -5}}}};
    ZDCDataAnalyzer::ZDCModuleFloatArray DeltaT0CutHighHG = {{{{8, 8, 8, 11}}, {{8, 10, 8, 12}}}};
    ZDCDataAnalyzer::ZDCModuleFloatArray DeltaT0CutLowLG = {{{{ -6, -5, -5, -5}}, {{ -5, -5, -5, -5}}}};
    ZDCDataAnalyzer::ZDCModuleFloatArray DeltaT0CutHighLG = {{{{8, 8, 8, 11}}, {{8, 10, 8, 12}}}};

    std::array<std::array<std::vector<float>, 4>, 2> slewingParamsHG, slewingParamsLG;

    slewingParamsHG[0][0] = {0, -6.5e-2,  2.85e-2, -2.83e-3};
    slewingParamsHG[0][1] = {0, -5.5e-2,  5.13e-2,  5.6e-3};
    slewingParamsHG[0][2] = {0, -1.45e-3, 9.3e-2,   3.9e-3};
    slewingParamsHG[0][3] = {0, -2.36e-2, 8.3e-2,   1.1e-3};

    slewingParamsHG[1][0] = {0, -6.5e-2,  4.84e-2, -3.7e-3};
    slewingParamsHG[1][1] = {0,  1.34e-2, 6.57e-2,  5.37e-3};
    slewingParamsHG[1][2] = {0, -5.37e-2, 3.49e-2,  3.8e-3};
    slewingParamsHG[1][3] = {0, -3.3e-2,  3.9e-2,   2.2e-3};

    slewingParamsLG[0][0] = {0, -9.6e-2,  4.39e-2,  2.93e-3 };
    slewingParamsLG[0][1] = {0, -5.0e-2, 14.9e-2,  20.6e-3  };
    slewingParamsLG[0][2] = {0, -4.4e-2,  5.3e-2,   0,      };
    slewingParamsLG[0][3] = {0, -9.90e-2, 4.08e-2,  0,      };

    slewingParamsLG[1][0] = {0,  -8.7e-2,  4.2e-2,  -3.2e-3 };
    slewingParamsLG[1][1] = {0,  -3.26e-2, 3.84e-2, -2.32e-3};
    slewingParamsLG[1][2] = {0, -26.8e-2, -2.64e-2, -5.3e-3 };
    slewingParamsLG[1][3] = {0, -13.2e-2,  0.45e-2, -2.4e-3 };

    std::array<std::array<std::vector<float>, 4>, 2> moduleHGNonLinCorr;
    moduleHGNonLinCorr[0][0] = { -3.76800e-02, 4.63597e-02};
    moduleHGNonLinCorr[0][1] = { -1.02185e-01, -1.17548e-01};
    moduleHGNonLinCorr[0][2] = { -8.78451e-02, -1.52174e-01};
    moduleHGNonLinCorr[0][3] = { -1.04835e-01, -1.96514e-01};
    moduleHGNonLinCorr[1][0] = { -6.83115e-02, 3.57802e-02};
    moduleHGNonLinCorr[1][1] = { -1.08162e-01, -1.91413e-01};
    moduleHGNonLinCorr[1][2] = { -7.82514e-02, -1.21218e-01};
    moduleHGNonLinCorr[1][3] = { -2.34354e-02, -2.52033e-01};

    m_zdcDataAnalyzer_80MHz.reset (new ZDCDataAnalyzer(MakeMessageFunction(), 7 , 12.5, 0, "FermiExp", m_peak2ndDerivMinSamples,
                                   m_peak2ndDerivMinThresholdsHG, m_peak2ndDerivMinThresholdsLG, m_lowGainOnly));

    m_zdcDataAnalyzer_80MHz->SetADCOverUnderflowValues(HGOverFlowADC, HGUnderFlowADC, LGOverFlowADC);
    m_zdcDataAnalyzer_80MHz->SetTauT0Values(fixTau1Arr, fixTau2Arr, tau1, tau2, t0HG, t0LG);
    m_zdcDataAnalyzer_80MHz->SetCutValues(chisqDivAmpCutHG, chisqDivAmpCutLG, DeltaT0CutLowHG, DeltaT0CutHighHG, DeltaT0CutLowLG, DeltaT0CutHighLG);
    m_zdcDataAnalyzer_80MHz->SetTimingCorrParams(ZDCPulseAnalyzer::TimingCorrLin, 500, 100,
						 slewingParamsHG, slewingParamsLG);
    m_zdcDataAnalyzer_80MHz->SetNonlinCorrParams(500, moduleHGNonLinCorr);
    m_zdcDataAnalyzer_80MHz->SetSaveFitFunc(false);
}

StatusCode ZdcAnalysisTool::initialize()
{
    m_tf1SincInterp.reset (new TF1("SincInterp", ZDC::SincInterp, -5., 160., 8));
    m_tf1SincInterp->SetNpx(300);

    // Set up calibrations
    //
    std::string filename = PathResolverFindCalibFile( "ZdcAnalysis/ZdcAnalysisConfig.conf" );
    TEnv env(filename.c_str());

    m_zdcEnergyCalibFileName = std::string(env.GetValue("ZdcEnergyCalibFileName", "ZdcCalibrations_v1.root"));
    ATH_MSG_INFO("ZDC energy calibration filename " << m_zdcEnergyCalibFileName);
    m_zdcTimeCalibFileName = std::string(env.GetValue("ZdcTimeCalibFileName", "ZdcTimeCalibrations_v1.root"));
    ATH_MSG_INFO("ZDC time calibration filename " << m_zdcTimeCalibFileName);
    m_zdcTriggerEffParamsFileName = std::string(env.GetValue("ZdcTriggerEffFileName", "ZdcTriggerEffParameters_v6.root"));
    ATH_MSG_INFO("ZDC trigger efficiencies filename " << m_zdcTriggerEffParamsFileName);


    if (m_forceCalibRun > -1) {
        ATH_MSG_DEBUG("CAREFUL: forcing calibration run/LB =" << m_forceCalibRun << "/" << m_forceCalibLB);

        if (m_forceCalibLB < 0) {
            ATH_MSG_ERROR("Invalid settings: Forced run > 0 but lumi block < 0");
            return StatusCode::FAILURE;
        }
    }

    // Use configuration to direct initialization
    //
    if (m_configuration == "default") {
        m_zdcDataAnalyzer = initializeDefault();
    }
    else if (m_configuration == "PbPb2015") {
        initialize80MHz();
        initialize40MHz();

        m_zdcDataAnalyzer = m_zdcDataAnalyzer_80MHz; // default
    }
    else if (m_configuration == "pPb2016") {
        m_zdcDataAnalyzer = initializepPb2016();
    }
    else if (m_configuration == "PbPb2018") {
        m_zdcDataAnalyzer = initializePbPb2018();
    }
    else if (m_configuration == "PbPb2015G4") {
      m_zdcDataAnalyzer = initializePbPb2015G4();
    }
    else if (m_configuration == "LHCf2022") {
      m_zdcDataAnalyzer = initializeLHCf2022();
    }
    else if (m_configuration == "pp2023") {
      m_zdcDataAnalyzer = initializepp2023();
    }
    else if (m_configuration == "PbPb2023") {
      m_zdcDataAnalyzer = initializePbPb2023();
    }
    else {
        ATH_MSG_ERROR("Unknown configuration: "  << m_configuration);
        return StatusCode::FAILURE;
    }

    // If an aux suffix is provided, prepend it with "_" so we don't have to do so at each use
    //

    ATH_MSG_INFO("Configuration: " << m_configuration);
    ATH_MSG_DEBUG("FlipEMDelay: " << m_flipEMDelay);
    ATH_MSG_DEBUG("LowGainOnly: " << m_lowGainOnly);

    ATH_MSG_DEBUG("Using Combined delayed and undelayed samples: " << m_combineDelay);

    ATH_MSG_DEBUG("WriteAux: " << m_writeAux);
    ATH_MSG_DEBUG("AuxSuffix: " << m_auxSuffix);
    ATH_MSG_DEBUG("DoCalib: " << m_doCalib);
    ATH_MSG_DEBUG("ForceCalibRun: " << m_forceCalibRun);
    ATH_MSG_DEBUG("ForceCalibLB: " << m_forceCalibLB);
    ATH_MSG_DEBUG("NumSampl: " << m_numSample);
    ATH_MSG_DEBUG("DeltaTSample: " << m_deltaTSample);
    ATH_MSG_DEBUG("Presample: " << m_presample);
    ATH_MSG_DEBUG("PeakSample: " << m_peakSample);
    ATH_MSG_DEBUG("Peak2ndDerivThresh: " << m_Peak2ndDerivThresh);

    if (m_combineDelay)  ATH_MSG_DEBUG("DelayDeltaT: " << m_delayDeltaT);

    ATH_MSG_DEBUG("T0: " << m_t0);
    ATH_MSG_DEBUG("Tau1: " << m_tau1);
    ATH_MSG_DEBUG("Tau2: " << m_tau2);
    ATH_MSG_DEBUG("FixTau1: " << m_fixTau1);
    ATH_MSG_DEBUG("FixTau2: " << m_fixTau2);
    ATH_MSG_DEBUG("DeltaTCut: " << m_deltaTCut);
    ATH_MSG_DEBUG("ChisqRatioCut: " << m_ChisqRatioCut);

    ATH_CHECK( m_eventInfoKey.initialize());

    // Initialize decorations

    m_zdcModuleAmplitude = m_zdcModuleContainerName+".Amplitude"+m_auxSuffix;
    ATH_CHECK( m_zdcModuleAmplitude.initialize());
    m_zdcModuleCalibEnergy = m_zdcModuleContainerName+".CalibEnergy"+m_auxSuffix;
    ATH_CHECK( m_zdcModuleCalibEnergy.initialize());
    m_zdcModuleCalibTime = m_zdcModuleContainerName+".CalibTime"+m_auxSuffix;
    ATH_CHECK( m_zdcModuleCalibTime.initialize());
    m_zdcModuleStatus = m_zdcModuleContainerName+".Status"+m_auxSuffix;
    ATH_CHECK( m_zdcModuleStatus.initialize());
    m_zdcModuleTime = m_zdcModuleContainerName+".Time"+m_auxSuffix;
    ATH_CHECK( m_zdcModuleTime.initialize());
    m_zdcModuleChisq = m_zdcModuleContainerName+".Chisq"+m_auxSuffix;
    ATH_CHECK( m_zdcModuleChisq.initialize());
    m_zdcModuleFitAmp = m_zdcModuleContainerName+".FitAmp"+m_auxSuffix;
    ATH_CHECK( m_zdcModuleFitAmp.initialize());
    m_zdcModuleFitAmpError = m_zdcModuleContainerName+".FitAmpError"+m_auxSuffix;
    ATH_CHECK( m_zdcModuleFitAmpError.initialize());
    m_zdcModuleFitT0 = m_zdcModuleContainerName+".FitT0"+m_auxSuffix;
    ATH_CHECK( m_zdcModuleFitT0.initialize());
    m_zdcModuleBkgdMaxFraction = m_zdcModuleContainerName+".BkgdMaxFraction"+m_auxSuffix;
    ATH_CHECK( m_zdcModuleBkgdMaxFraction.initialize());
    m_zdcModulePreSampleAmp = m_zdcModuleContainerName+".PreSampleAmp"+m_auxSuffix;
    ATH_CHECK( m_zdcModulePreSampleAmp.initialize());
    m_zdcModulePresample = m_zdcModuleContainerName+".Presample"+m_auxSuffix;
    ATH_CHECK( m_zdcModulePresample.initialize());
    m_zdcModuleMinDeriv2nd = m_zdcModuleContainerName+".MinDeriv2nd"+m_auxSuffix;
    ATH_CHECK( m_zdcModuleMinDeriv2nd.initialize());
    m_zdcModuleMaxADC = m_zdcModuleContainerName+".MaxADC"+m_auxSuffix;
    ATH_CHECK( m_zdcModuleMaxADC.initialize());
    m_rpdChannelBaseline = m_zdcModuleContainerName+".RPDChannelBaseline"+m_auxSuffix;
    ATH_CHECK( m_rpdChannelBaseline.initialize());
    m_rpdChannelPileupExpFitParams = m_zdcModuleContainerName+".RPDChannelPileupExpFitParams"+m_auxSuffix;
    ATH_CHECK( m_rpdChannelPileupExpFitParams.initialize());
    m_rpdChannelPileupStretchedExpFitParams = m_zdcModuleContainerName+".RPDChannelPileupStretchedExpFitParams"+m_auxSuffix;
    ATH_CHECK( m_rpdChannelPileupStretchedExpFitParams.initialize());
    m_rpdChannelPileupExpFitParamErrs = m_zdcModuleContainerName+".RPDChannelPileupExpFitParamErrs"+m_auxSuffix;
    ATH_CHECK( m_rpdChannelPileupExpFitParamErrs.initialize());
    m_rpdChannelPileupStretchedExpFitParamErrs = m_zdcModuleContainerName+".RPDChannelPileupStretchedExpFitParamErrs"+m_auxSuffix;
    ATH_CHECK( m_rpdChannelPileupStretchedExpFitParamErrs.initialize());
    m_rpdChannelPileupExpFitMSE = m_zdcModuleContainerName+".RPDChannelPileupExpFitMSE"+m_auxSuffix;
    ATH_CHECK( m_rpdChannelPileupExpFitMSE.initialize());
    m_rpdChannelPileupStretchedExpFitMSE = m_zdcModuleContainerName+".RPDChannelPileupStretchedExpFitMSE"+m_auxSuffix;
    ATH_CHECK( m_rpdChannelPileupStretchedExpFitMSE.initialize());
    m_rpdChannelAmplitude = m_zdcModuleContainerName+".RPDChannelAmplitude"+m_auxSuffix;
    ATH_CHECK( m_rpdChannelAmplitude.initialize());
    m_rpdChannelAmplitudeCalib = m_zdcModuleContainerName+".RPDChannelAmplitudeCalib"+m_auxSuffix;
    ATH_CHECK( m_rpdChannelAmplitudeCalib.initialize());
    m_rpdChannelMaxAdc = m_zdcModuleContainerName+".RPDChannelMaxADC"+m_auxSuffix;
    ATH_CHECK( m_rpdChannelMaxAdc.initialize());
    m_rpdChannelMaxAdcCalib = m_zdcModuleContainerName+".RPDChannelMaxADCCalib"+m_auxSuffix;
    ATH_CHECK( m_rpdChannelMaxAdcCalib.initialize());
    m_rpdChannelMaxSample = m_zdcModuleContainerName+".RPDChannelMaxSample"+m_auxSuffix;
    ATH_CHECK( m_rpdChannelMaxSample.initialize());
    m_rpdChannelStatus = m_zdcModuleContainerName+".RPDChannelStatus"+m_auxSuffix;
    ATH_CHECK( m_rpdChannelStatus.initialize());
    m_rpdChannelPileupFrac = m_zdcModuleContainerName+".RPDChannelPileupFrac"+m_auxSuffix;
    ATH_CHECK( m_rpdChannelPileupFrac.initialize());

    m_zdcSumUncalibSum = m_zdcSumContainerName+".UncalibSum"+m_auxSuffix;
    ATH_CHECK( m_zdcSumUncalibSum.initialize());
    m_zdcSumCalibEnergy = m_zdcSumContainerName+".CalibEnergy"+m_auxSuffix;
    ATH_CHECK( m_zdcSumCalibEnergy.initialize());
    m_zdcSumCalibEnergyErr = m_zdcSumContainerName+".CalibEnergyErr"+m_auxSuffix;
    ATH_CHECK( m_zdcSumCalibEnergyErr.initialize());
    m_zdcSumUncalibSumErr = m_zdcSumContainerName+".UncalibSumErr"+m_auxSuffix;
    ATH_CHECK( m_zdcSumUncalibSumErr.initialize());
    m_zdcSumFinalEnergy = m_zdcSumContainerName+".FinalEnergy"+m_auxSuffix;
    ATH_CHECK( m_zdcSumFinalEnergy.initialize());
    m_zdcSumAverageTime = m_zdcSumContainerName+".AverageTime"+m_auxSuffix;
    ATH_CHECK( m_zdcSumAverageTime.initialize());
    m_zdcSumStatus = m_zdcSumContainerName+".Status"+m_auxSuffix;
    ATH_CHECK( m_zdcSumStatus.initialize());
    m_zdcSumModuleMask = m_zdcSumContainerName+".ModuleMask"+m_auxSuffix;
    ATH_CHECK( m_zdcSumModuleMask.initialize());
    m_zdcSumRPDStatus = m_zdcSumContainerName+".RPDStatus"+m_auxSuffix;
    ATH_CHECK( m_zdcSumRPDStatus.initialize());

    if (m_writeAux && m_auxSuffix != "") {
        ATH_MSG_DEBUG("suffix string = " << m_auxSuffix);
    }

    m_init = true;

    return StatusCode::SUCCESS;
}
  
void ZdcAnalysisTool::initializeDecorations()
{

}

StatusCode ZdcAnalysisTool::configureNewRun(unsigned int runNumber)
{
    ATH_MSG_DEBUG("Setting up new run " << runNumber);

    // We do nothing for the default configuration
    //
    if (m_configuration != "default") {
        if (m_configuration == "PbPb2015") {
            //
            // Two periods, 40 MHz and 80 MHz readout
            //
            if (runNumber < 287222) m_zdcDataAnalyzer = m_zdcDataAnalyzer_40MHz;
            else m_zdcDataAnalyzer = m_zdcDataAnalyzer_80MHz;
        }
    }

    return StatusCode::SUCCESS;
}


StatusCode ZdcAnalysisTool::recoZdcModules(const xAOD::ZdcModuleContainer& moduleContainer, const xAOD::ZdcModuleContainer& moduleSumContainer)
{

  if (moduleContainer.size()==0) return StatusCode::SUCCESS; // if no modules, do nothing

  SG::ReadHandle<xAOD::EventInfo> eventInfo(m_eventInfoKey);
  if (!eventInfo.isValid()) return StatusCode::FAILURE;
    
  // check for new run number, if new, possibly update configuration and/or calibrations
  //
  unsigned int thisRunNumber = eventInfo->runNumber();
  if (thisRunNumber != m_runNumber) {
    ATH_MSG_DEBUG("ZDC analysis tool will be configured for run " << thisRunNumber);
    
    ATH_CHECK(configureNewRun(thisRunNumber)); // ALWAYS check methods that return StatusCode
    
    ATH_MSG_DEBUG("Setting up calibrations");
    
    if (m_doCalib) {
      //
      // Check for calibration override
      //
      unsigned int calibRunNumber = thisRunNumber;
      if (m_forceCalibRun > -1) calibRunNumber = m_forceCalibRun;
      
      setEnergyCalibrations(calibRunNumber);
      if (m_doTrigEff) initializeTriggerEffs(calibRunNumber); // if energy calibrations fail to load, then so will trigger efficiencies
      if (m_doTimeCalib) setTimeCalibrations(calibRunNumber);
    }
    
    m_runNumber = thisRunNumber;
  }
  
  m_lumiBlock = eventInfo->lumiBlock();
  
  unsigned int calibLumiBlock = m_lumiBlock;
  if (m_doCalib) {
    if (m_forceCalibRun > 0) calibLumiBlock = m_forceCalibLB;
  }
  
  ATH_MSG_DEBUG("Starting event processing");
  ATH_MSG_DEBUG("LB=" << calibLumiBlock);
  
  m_zdcDataAnalyzer->StartEvent(calibLumiBlock);

  if (m_LHCRun==3)
    {
      m_rpdDataAnalyzer.at(0)->reset();
      m_rpdDataAnalyzer.at(1)->reset();
    }

  const std::vector<unsigned short>* adcUndelayLG = 0;
  const std::vector<unsigned short>* adcUndelayHG = 0;
  
  const std::vector<unsigned short>* adcDelayLG = 0;
  const std::vector<unsigned short>* adcDelayHG = 0;
  
  ATH_MSG_DEBUG("Processing modules");
    for (const auto zdcModule : moduleContainer)
      {
	int side = -1;
	if (zdcModule->zdcSide() == -1) side =  0;
	else if (zdcModule->zdcSide() == 1) side = 1;
	else {
	  // Invalid side
	  //
	  ATH_MSG_WARNING("Invalid side value found for module number: " << zdcModule->zdcModule() << ", side value = " << side);
	  continue;
	}

	if (zdcModule->zdcType() == 1) {
	  //  This is RPD data in Run 3
	  //
	  if (m_LHCRun < 3) continue; // type == 1 -> pixel data in runs 2 and 3, skip
	  ATH_MSG_DEBUG("RPD side " << side << " chan " << zdcModule->zdcChannel() );

	  unsigned int rpdChannel = zdcModule->zdcChannel(); // channel numbers are fixed in mapping, numbered 0-15
	  if (rpdChannel > 15) {
	    //
	    //  The data is somehow corrupt, spit out an error
	    //
	    ATH_MSG_WARNING("Invalid RPD channel found on side " << side << ", channel number = " << rpdChannel << ", skipping this module");
	    continue;
	  }
	  else {
	    const std::vector<uint16_t>* vector_p = &(zdcModule->auxdata<std::vector<uint16_t>>("g0data"));
	    if (!vector_p) {
	      //  This is obviously a problem, generate a non-fatal but serious error and continue
	      //
	      ATH_MSG_WARNING("Could not retrieve waveform for side " << side << ", module " << zdcModule->zdcModule() << ", skipping this module");
	      continue;
	    }
	    
	    //
	    // Pass the data to the RPD analysis tool 
	    //
	    m_rpdDataAnalyzer.at(side)->loadChannelData(rpdChannel, *vector_p);
	  }
	}
	else {
	  //
	  // This is ZDC data
	  //
	  if (m_LHCRun==3) // no delay channels, so we drop the index
	    {
	      adcUndelayLG = &(zdcModule->auxdata<std::vector<uint16_t>>("g0data")); // g0
	      adcUndelayHG = &(zdcModule->auxdata<std::vector<uint16_t>>("g1data")); // g1
	    }
	  else if (m_LHCRun==2)
	    {
	      if (zdcModule->zdcType() == 1) continue; // skip position sensitive modules
	      
	      if (zdcModule->zdcModule() == 0 && m_flipEMDelay) // flip delay/non-delay for 2015 ONLY
		{
		  adcUndelayLG = &(zdcModule->auxdata<std::vector<uint16_t>>("g0d1Data")); // g0d1
		  adcUndelayHG = &(zdcModule->auxdata<std::vector<uint16_t>>("g1d1Data")); // g1d1
		  
		  adcDelayLG = &(zdcModule->auxdata<std::vector<uint16_t>>("g0d0Data")); // g0d0
		  adcDelayHG = &(zdcModule->auxdata<std::vector<uint16_t>>("g1d0Data")); // g1d0
		}
	      else // nominal configuation
		{
		  adcUndelayLG = &(zdcModule->auxdata<std::vector<uint16_t>>("g0d0Data")); // g0d0
		  adcUndelayHG = &(zdcModule->auxdata<std::vector<uint16_t>>("g1d0Data")); // g1d0
		  
		  adcDelayLG = &(zdcModule->auxdata<std::vector<uint16_t>>("g0d1Data")); // g0d1
		  adcDelayHG = &(zdcModule->auxdata<std::vector<uint16_t>>("g1d1Data")); // g1d1
		}
	    }
	  else
	    {
	      ATH_MSG_WARNING("Unknown LHC Run " << m_LHCRun);
	      return StatusCode::FAILURE;
	    }
	  
	  // Why were these static? to optimize processing time	
	  std::vector<float> HGUndelADCSamples(m_numSample);
	  std::vector<float> LGUndelADCSamples(m_numSample);
	  
	  std::copy(adcUndelayLG->begin(), adcUndelayLG->end(), LGUndelADCSamples.begin());
	  std::copy(adcUndelayHG->begin(), adcUndelayHG->end(), HGUndelADCSamples.begin());
	  
	  int side = (zdcModule->zdcSide() == -1) ? 0 : 1 ;
	  
	  if (!m_combineDelay) {
	    m_zdcDataAnalyzer->LoadAndAnalyzeData(side, zdcModule->zdcModule(), HGUndelADCSamples, LGUndelADCSamples);
	  }
	  else {
	    std::vector<float> HGDelayADCSamples(m_numSample);
	    std::vector<float> LGDelayADCSamples(m_numSample);
	    
	    std::copy(adcDelayLG->begin(), adcDelayLG->end(), LGDelayADCSamples.begin());
	    std::copy(adcDelayHG->begin(), adcDelayHG->end(), HGDelayADCSamples.begin());
	    
	    // If the delayed channels actually come earlier (as in the pPb in 2016), we invert the meaning of delayed and undelayed
	    //   see the initialization sections for similar inversion on the sign of the pedestal difference
	    //
	    
	    m_zdcDataAnalyzer->LoadAndAnalyzeData(side, zdcModule->zdcModule(),
						  HGUndelADCSamples, LGUndelADCSamples,
						  HGDelayADCSamples, LGDelayADCSamples);
	  }
	}
      }
    
    // analyze RPD data only once all channels have been loaded
    if (m_LHCRun==3)
      {
	m_rpdDataAnalyzer.at(0)->analyzeData();
	m_rpdDataAnalyzer.at(1)->analyzeData();
      }

    ATH_MSG_DEBUG("Finishing event processing");
    
    m_zdcDataAnalyzer->FinishEvent();
    
    ATH_MSG_DEBUG("Adding variables with suffix=" + m_auxSuffix);

    SG::WriteDecorHandle<xAOD::ZdcModuleContainer,float> zdcModuleAmplitude(m_zdcModuleAmplitude);
    SG::WriteDecorHandle<xAOD::ZdcModuleContainer,float> zdcModuleCalibEnergy(m_zdcModuleCalibEnergy);
    SG::WriteDecorHandle<xAOD::ZdcModuleContainer,float> zdcModuleCalibTime(m_zdcModuleCalibTime);
    SG::WriteDecorHandle<xAOD::ZdcModuleContainer,unsigned int> zdcModuleStatus(m_zdcModuleStatus);
    SG::WriteDecorHandle<xAOD::ZdcModuleContainer,float> zdcModuleTime(m_zdcModuleTime);
    SG::WriteDecorHandle<xAOD::ZdcModuleContainer,float> zdcModuleChisq(m_zdcModuleChisq);
    SG::WriteDecorHandle<xAOD::ZdcModuleContainer,float> zdcModuleFitAmp(m_zdcModuleFitAmp);
    SG::WriteDecorHandle<xAOD::ZdcModuleContainer,float> zdcModuleFitAmpError(m_zdcModuleFitAmpError);
    SG::WriteDecorHandle<xAOD::ZdcModuleContainer,float> zdcModuleFitT0(m_zdcModuleFitT0);
    SG::WriteDecorHandle<xAOD::ZdcModuleContainer,float> zdcModuleBkgdMaxFraction(m_zdcModuleBkgdMaxFraction);
    SG::WriteDecorHandle<xAOD::ZdcModuleContainer,float> zdcModulePreSampleAmp(m_zdcModulePreSampleAmp);
    SG::WriteDecorHandle<xAOD::ZdcModuleContainer,float> zdcModulePresample(m_zdcModulePresample);
    SG::WriteDecorHandle<xAOD::ZdcModuleContainer,float> zdcModuleMinDeriv2nd(m_zdcModuleMinDeriv2nd);
    SG::WriteDecorHandle<xAOD::ZdcModuleContainer,float> zdcModuleMaxADC(m_zdcModuleMaxADC);
    SG::WriteDecorHandle<xAOD::ZdcModuleContainer,float> rpdChannelBaseline(m_rpdChannelBaseline);
    SG::WriteDecorHandle<xAOD::ZdcModuleContainer,std::vector<float>> rpdChannelPileupExpFitParams(m_rpdChannelPileupExpFitParams);
    SG::WriteDecorHandle<xAOD::ZdcModuleContainer,std::vector<float>> rpdChannelPileupStretchedExpFitParams(m_rpdChannelPileupStretchedExpFitParams);
    SG::WriteDecorHandle<xAOD::ZdcModuleContainer,std::vector<float>> rpdChannelPileupExpFitParamErrs(m_rpdChannelPileupExpFitParamErrs);
    SG::WriteDecorHandle<xAOD::ZdcModuleContainer,std::vector<float>> rpdChannelPileupStretchedExpFitParamErrs(m_rpdChannelPileupStretchedExpFitParamErrs);
    SG::WriteDecorHandle<xAOD::ZdcModuleContainer,float> rpdChannelPileupExpFitMSE(m_rpdChannelPileupExpFitMSE);
    SG::WriteDecorHandle<xAOD::ZdcModuleContainer,float> rpdChannelPileupStretchedExpFitMSE(m_rpdChannelPileupStretchedExpFitMSE);
    SG::WriteDecorHandle<xAOD::ZdcModuleContainer,float> rpdChannelAmplitude(m_rpdChannelAmplitude);
    SG::WriteDecorHandle<xAOD::ZdcModuleContainer,float> rpdChannelAmplitudeCalib(m_rpdChannelAmplitudeCalib);
    SG::WriteDecorHandle<xAOD::ZdcModuleContainer,float> rpdChannelMaxAdc(m_rpdChannelMaxAdc);
    SG::WriteDecorHandle<xAOD::ZdcModuleContainer,float> rpdChannelMaxAdcCalib(m_rpdChannelMaxAdcCalib);
    SG::WriteDecorHandle<xAOD::ZdcModuleContainer,unsigned int> rpdChannelMaxSample(m_rpdChannelMaxSample);
    SG::WriteDecorHandle<xAOD::ZdcModuleContainer,unsigned int> rpdChannelStatus(m_rpdChannelStatus);
    SG::WriteDecorHandle<xAOD::ZdcModuleContainer,float> rpdChannelPileupFrac(m_rpdChannelPileupFrac);
    
    // CalibTime
    // Status
    // Time
    // Chisq
    // FitAmp
    // FitAmpError
    // FitT0
    // BkgdMaxFraction
    // PreSampleAmp
    // Presample
    // MinDeriv2nd
    // MaxADC

    for (const auto zdcModule : moduleContainer)
    {
        int side = (zdcModule->zdcSide() == -1) ? 0 : 1 ;
        int mod = zdcModule->zdcModule();

        if (zdcModule->zdcType() == 1 && m_LHCRun==3) {
          // this is the RPD
          if (m_writeAux) {
            int rpdChannel = zdcModule->zdcChannel(); // channel numbers are fixed in mapping, numbered 0-15
	    rpdChannelBaseline(*zdcModule) = m_rpdDataAnalyzer.at(side)->getChBaseline(rpdChannel);
	    rpdChannelPileupExpFitParams(*zdcModule) = m_rpdDataAnalyzer.at(side)->getChPileupExpFitParams(rpdChannel);
	    rpdChannelPileupStretchedExpFitParams(*zdcModule) = m_rpdDataAnalyzer.at(side)->getChPileupStretchedExpFitParams(rpdChannel);
	    rpdChannelPileupExpFitParamErrs(*zdcModule) = m_rpdDataAnalyzer.at(side)->getChPileupExpFitParamErrs(rpdChannel);
	    rpdChannelPileupStretchedExpFitParamErrs(*zdcModule) = m_rpdDataAnalyzer.at(side)->getChPileupStretchedExpFitParamErrs(rpdChannel);
	    rpdChannelPileupExpFitMSE(*zdcModule) = m_rpdDataAnalyzer.at(side)->getChPileupExpFitMSE(rpdChannel);
	    rpdChannelPileupStretchedExpFitMSE(*zdcModule) = m_rpdDataAnalyzer.at(side)->getChPileupStretchedExpFitMSE(rpdChannel);
	    rpdChannelAmplitude(*zdcModule) = m_rpdDataAnalyzer.at(side)->getChSumAdc(rpdChannel);
	    rpdChannelAmplitudeCalib(*zdcModule) = m_rpdDataAnalyzer.at(side)->getChSumAdcCalib(rpdChannel);
	    rpdChannelMaxAdc(*zdcModule) = m_rpdDataAnalyzer.at(side)->getChMaxAdc(rpdChannel);
	    rpdChannelMaxAdcCalib(*zdcModule) = m_rpdDataAnalyzer.at(side)->getChMaxAdcCalib(rpdChannel);
	    rpdChannelMaxSample(*zdcModule) = m_rpdDataAnalyzer.at(side)->getChMaxSample(rpdChannel);
	    rpdChannelStatus(*zdcModule) =  m_rpdDataAnalyzer.at(side)->getChStatus(rpdChannel);
	    rpdChannelPileupFrac(*zdcModule) =  m_rpdDataAnalyzer.at(side)->getChPileupFrac(rpdChannel);
          }
        } else if (zdcModule->zdcType() == 0) {
          // this is the main ZDC
          if (m_writeAux) {
              if (m_doCalib) {
                  float calibEnergy = m_zdcDataAnalyzer->GetModuleCalibAmplitude(side, mod);
		  zdcModuleCalibEnergy(*zdcModule) = calibEnergy;
		  zdcModuleCalibTime(*zdcModule) = m_zdcDataAnalyzer->GetModuleCalibTime(side, mod);
              }
              else
              {
		  zdcModuleCalibEnergy(*zdcModule) = -1000;
		  zdcModuleCalibTime(*zdcModule) = -1000;
              }

	      zdcModuleAmplitude(*zdcModule) = m_zdcDataAnalyzer->GetModuleAmplitude(side, mod);
	      zdcModuleStatus(*zdcModule) = m_zdcDataAnalyzer->GetModuleStatus(side, mod);
	      zdcModuleTime(*zdcModule) = m_zdcDataAnalyzer->GetModuleTime(side, mod);

              const ZDCPulseAnalyzer* pulseAna_p = m_zdcDataAnalyzer->GetPulseAnalyzer(side, mod);
              zdcModuleChisq(*zdcModule) = pulseAna_p->GetChisq();
	      zdcModuleFitAmp(*zdcModule) = pulseAna_p->GetFitAmplitude();
	      zdcModuleFitAmpError(*zdcModule) =  pulseAna_p->GetAmpError();
	      zdcModuleFitT0(*zdcModule) = pulseAna_p->GetFitT0();
	      zdcModuleBkgdMaxFraction(*zdcModule) = pulseAna_p->GetBkgdMaxFraction();
              zdcModulePreSampleAmp(*zdcModule) = pulseAna_p->GetPreSampleAmp();
	      zdcModulePresample(*zdcModule) = pulseAna_p->GetPresample();
	      zdcModuleMinDeriv2nd(*zdcModule) = pulseAna_p->GetMinDeriv2nd();
	      zdcModuleMaxADC(*zdcModule) = pulseAna_p->GetMaxADC();
            }
	  //ATH_MSG_DEBUG ("side = " << side << " module=" << zdcModule->zdcModule() << " CalibEnergy=" << zdcModule->auxdecor<float>("CalibEnergy")
          //                  << " should be " << m_zdcDataAnalyzer->GetModuleCalibAmplitude(side, mod));
        }
    }

    // Output sum information
    // In Run 3 - we have to assume the container already exists (since it is needed to store the per-side trigger info)
    // reprocessing will add new variables with the suffix

    SG::WriteDecorHandle<xAOD::ZdcModuleContainer,float> zdcSumUncalibSum(m_zdcSumUncalibSum);
    SG::WriteDecorHandle<xAOD::ZdcModuleContainer,float> zdcSumUncalibSumErr(m_zdcSumUncalibSumErr);
    SG::WriteDecorHandle<xAOD::ZdcModuleContainer,float> zdcSumCalibEnergy(m_zdcSumCalibEnergy);
    SG::WriteDecorHandle<xAOD::ZdcModuleContainer,float> zdcSumCalibEnergyErr(m_zdcSumCalibEnergyErr);
    SG::WriteDecorHandle<xAOD::ZdcModuleContainer,float> zdcSumFinalEnergy(m_zdcSumFinalEnergy);
    SG::WriteDecorHandle<xAOD::ZdcModuleContainer,float> zdcSumAverageTime(m_zdcSumAverageTime);
    SG::WriteDecorHandle<xAOD::ZdcModuleContainer,unsigned int> zdcSumStatus(m_zdcSumStatus);
    SG::WriteDecorHandle<xAOD::ZdcModuleContainer,unsigned int> zdcSumModuleMask(m_zdcSumModuleMask);
    SG::WriteDecorHandle<xAOD::ZdcModuleContainer,unsigned int> zdcSumRPDStatus(m_zdcSumRPDStatus);

    for (const auto zdc_sum: moduleSumContainer)
      {
	ATH_MSG_DEBUG("Extracting ZDC side " << zdc_sum->zdcSide());

	if (zdc_sum->zdcSide()==0) continue; // skip new global sum

	int iside = (zdc_sum->zdcSide()==-1) ? 0 : 1;

	float uncalibSum = getUncalibModuleSum(iside);
	zdcSumUncalibSum(*zdc_sum) = uncalibSum;
	float uncalibSumErr = getUncalibModuleSumErr(iside);
	zdcSumUncalibSumErr(*zdc_sum) = uncalibSumErr;

	float calibEnergy = getCalibModuleSum(iside);
	zdcSumCalibEnergy(*zdc_sum) = calibEnergy;
	float calibEnergyErr = getCalibModuleSumErr(iside);
	zdcSumCalibEnergyErr(*zdc_sum) = calibEnergyErr;

	float finalEnergy = calibEnergy;
	zdcSumFinalEnergy(*zdc_sum) = finalEnergy;
	zdcSumAverageTime(*zdc_sum) = getAverageTime(iside);
	zdcSumStatus(*zdc_sum) = !sideFailed(iside);
	zdcSumModuleMask(*zdc_sum) = (getModuleMask() >> (4 * iside)) & 0xF;
	if (m_LHCRun==3)
	  {
	    zdcSumRPDStatus(*zdc_sum) = m_rpdDataAnalyzer.at(iside)->getSideStatus();
	  }
      }

    return StatusCode::SUCCESS;
}

void ZdcAnalysisTool::setEnergyCalibrations(unsigned int runNumber)
{

    char name[128];

    std::string filename;
    if (m_LHCRun==3)
      {
	filename = PathResolverFindCalibFile( ("ZdcAnalysis/ZdcCalib_Run"+TString::Itoa(runNumber,10)+".root").Data() );
      }
    else if (m_LHCRun==2)
      {
	filename = PathResolverFindCalibFile( ("ZdcAnalysis/" + m_zdcEnergyCalibFileName).c_str() );
      }
    else
      {
	ATH_MSG_WARNING("No LHC Run defined, so no calibration allowed");
	return;
      }

    ATH_MSG_INFO("Opening energy calibration file " << filename);
    std::unique_ptr<TFile> fCalib (TFile::Open(filename.c_str(), "READ"));

    if (fCalib == nullptr || fCalib->IsZombie())
    {
        ATH_MSG_INFO ("failed to open file: " << filename);
        throw std::runtime_error ("failed to open file " + filename);
    }

    std::array<std::array<std::unique_ptr<TSpline>, 4>, 2> splines;

    for (int iside = 0; iside < 2; iside++)
    {
        for (int imod = 0; imod < 4; imod++)
        {
            sprintf(name, "ZDC_Ecalib_run%u_s%d_m%d", runNumber, iside, imod);
            ATH_MSG_DEBUG("Searching for spline " << name);
            TSpline3* s = (TSpline3*) fCalib->GetObjectChecked(name, "TSpline3");
            if (!s && m_doCalib)
            {
                ATH_MSG_WARNING("No calibrations for run " << runNumber);
                m_doCalib = false;
            }

            if (s)
            {
                splines[iside][imod].reset (s);
            }
            else
            {
                ATH_MSG_WARNING("No spline " << name);
            }
        }
    }
    fCalib->Close();

    if (m_doCalib) m_zdcDataAnalyzer->LoadEnergyCalibrations(splines);

    return;
}

void ZdcAnalysisTool::setTimeCalibrations(unsigned int runNumber)
{
    std::string filename;

    if (m_LHCRun==3)
      {
	filename = PathResolverFindCalibFile( ("ZdcAnalysis/ZdcCalib_Run"+TString::Itoa(runNumber,10)+".root").Data() );
      }
    else if (m_LHCRun==2)
      {
	filename = PathResolverFindCalibFile( "ZdcAnalysis/" + m_zdcTimeCalibFileName );
      }
    else
      {
	ATH_MSG_WARNING("No LHC Run defined, so no time calibration allowed");
	return;
      }

    char name[128] = {0};
    ATH_MSG_INFO("Opening time calibration file " << filename);
    std::unique_ptr<TFile> fCalib (TFile::Open(filename.c_str(), "READ"));

    if (fCalib && !fCalib->IsZombie())
    {
      bool success = true;
        std::array<std::array<std::unique_ptr<TSpline>, 4>, 2> T0HGOffsetSplines;
        std::array<std::array<std::unique_ptr<TSpline>, 4>, 2> T0LGOffsetSplines;
        std::unique_ptr<TSpline3> spline;
        for (int iside = 0; iside < 2; iside++)
        {
            for (int imod = 0; imod < 4; imod++)
            {
                sprintf(name, "ZDC_T0calib_run%u_HG_s%d_m%d", runNumber, iside, imod);
                spline.reset (static_cast<TSpline3*>(fCalib->GetObjectChecked(name, "TSpline3")));
                if (spline)
                {
                    T0HGOffsetSplines[iside][imod] = std::move (spline);
                }
                else
                {
                    ATH_MSG_WARNING("No time calib. spline " << name);
		    success = false;
                }

                sprintf(name, "ZDC_T0calib_run%u_LG_s%d_m%d", runNumber, iside, imod);
                spline.reset (static_cast<TSpline3*>(fCalib->GetObjectChecked(name, "TSpline3")));
                if (spline)
                {
                    T0LGOffsetSplines[iside][imod] = std::move (spline);
                }
                else
                {
                    ATH_MSG_WARNING("No time calib. spline " << name);
		    success = false;
                }
            }
        }

        if (success) 
	  m_zdcDataAnalyzer->LoadT0Calibrations(T0HGOffsetSplines, T0LGOffsetSplines);
	else
	  ATH_MSG_WARNING("Time calibration failed - no T0 offsets loaded " << name);

        fCalib->Close();
    }
    else
    {
        ATH_MSG_WARNING("No time calibration file " << name);
    }
}

StatusCode ZdcAnalysisTool::reprocessZdc()
{
    if (!m_init)
    {
        ATH_MSG_WARNING("Tool not initialized!");
        return StatusCode::FAILURE;
    }
    m_eventReady = false;
    ATH_MSG_DEBUG ("Trying to retrieve " << m_zdcModuleContainerName);

    m_zdcModules = 0;
    ATH_CHECK(evtStore()->retrieve(m_zdcModules, m_zdcModuleContainerName));

    m_zdcSums = 0;
    ATH_CHECK(evtStore()->retrieve(m_zdcSums, m_zdcSumContainerName));

    m_eventReady = true;

    ATH_CHECK(recoZdcModules(*m_zdcModules, *m_zdcSums));

    return StatusCode::SUCCESS;
}

bool ZdcAnalysisTool::sigprocMaxFinder(const std::vector<unsigned short>& adc, float deltaT, float& amp, float& time, float& qual)
{
    size_t nsamp = adc.size();
    float presamp = adc.at(0);
    unsigned short max_adc = 0;
    int max_index = -1;
    for (size_t i = 0; i < nsamp; i++)
    {
        if (adc[i] > max_adc)
        {
            max_adc = adc[i];
            max_index = i;
        }
    }
    amp = max_adc - presamp;
    time = max_index * deltaT;
    qual = 1.;

    if (max_index == -1)
    {
        qual = 0.;
        return false;
    }

    return true;
}

bool ZdcAnalysisTool::sigprocSincInterp(const std::vector<unsigned short>& adc, float deltaT, float& amp, float& time, float& qual)
{
    size_t nsamp = adc.size();
    float presamp = adc.at(0);
    m_tf1SincInterp->SetParameter(0, deltaT);
    for (size_t i = 0; i < nsamp; i++)
    {
        m_tf1SincInterp->SetParameter(i + 1, adc.at(i) - presamp);
    }
    amp = m_tf1SincInterp->GetMaximum();
    time = m_tf1SincInterp->GetMaximumX();
    qual = 1.;
    return true;
}

float ZdcAnalysisTool::getModuleSum(int side)
{
    if (!m_zdcDataAnalyzer) return 0;
    return m_zdcDataAnalyzer->GetModuleSum(side);
}

float ZdcAnalysisTool::getCalibModuleSum(int side)
{
    if (!m_zdcDataAnalyzer) return 0;
    return m_zdcDataAnalyzer->GetCalibModuleSum(side);
}

float ZdcAnalysisTool::getCalibModuleSumErr(int side)
{
    if (!m_zdcDataAnalyzer) return 0;
    return m_zdcDataAnalyzer->GetCalibModuleSumErr(side);
}

float ZdcAnalysisTool::getUncalibModuleSum(int side)
{
    if (!m_zdcDataAnalyzer) return 0;
    return m_zdcDataAnalyzer->GetModuleSum(side);
}

float ZdcAnalysisTool::getUncalibModuleSumErr(int side)
{
    if (!m_zdcDataAnalyzer) return 0;
    return m_zdcDataAnalyzer->GetModuleSumErr(side);
}

float ZdcAnalysisTool::getAverageTime(int side)
{
    if (!m_zdcDataAnalyzer) return 0;
    return m_zdcDataAnalyzer->GetAverageTime(side);
}

bool ZdcAnalysisTool::sideFailed(int side)
{
    if (!m_zdcDataAnalyzer) return 0;
    return m_zdcDataAnalyzer->SideFailed(side);
}

unsigned int ZdcAnalysisTool::getModuleMask()
{
    if (!m_zdcDataAnalyzer) return 0;
    return m_zdcDataAnalyzer->GetModuleMask();
}

double ZdcAnalysisTool::getTriggerEfficiency(int side)
{
    if (!m_doTrigEff) return -1;

    m_zdcTriggerEfficiency->UpdatelumiBlock(m_lumiBlock);
    float adcSum = getModuleSum(side);
    double eff = m_zdcTriggerEfficiency->GetEfficiency(side, adcSum);
    return eff;
}

double ZdcAnalysisTool::getTriggerEfficiencyUncertainty(int side)
{
    if (!m_doCalib) return -1;

    m_zdcTriggerEfficiency->UpdatelumiBlock(m_lumiBlock);
    float adcSum = getModuleSum(side);
    std::pair<double, double> eff_pair = m_zdcTriggerEfficiency->GetEfficiencyAndError(msg(), side, adcSum);
    return eff_pair.second;
}


} // namespace ZDC
