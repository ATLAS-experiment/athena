/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include <ZdcAnalysis/ZDCDataAnalyzer.h>
#include <ZdcAnalysis/ZDCPulseAnalyzer.h>

#include <utility>

#include "CxxUtils/trapping_fp.h"


const ZDCJSONConfig::JSONParamList ZDCDataAnalyzer::JSONConfigParams = {
  {"moduleEnabled", {JSON::value_t::array, 4, true, false}},
  {"iterativeCalibCorr", {JSON::value_t::array, 4, true, false}}
};


ZDCDataAnalyzer::ZDCDataAnalyzer(ZDCMsg::MessageFunctionPtr msgFunc_p, int nSample, float deltaTSample, size_t preSampleIdx, std::string fitFunction,
                                 const ZDCModuleIntArray& peak2ndDerivMinSamples,
                                 const ZDCModuleFloatArray& peak2ndDerivMinThresholdsHG,
                                 const ZDCModuleFloatArray& peak2ndDerivMinThresholdsLG,
                                 unsigned int LGMode) :
  m_msgFunc_p(std::move(msgFunc_p))
{
  m_moduleEnabled[0] = {{true, true, true, true}};
  m_moduleEnabled[1] = {{true, true, true, true}};

  m_moduleAnalyzers[0] = {{0, 0, 0, 0}};
  m_moduleAnalyzers[1] = {{0, 0, 0, 0}};

  m_calibAmplitude[0] = {{0, 0, 0, 0}};
  m_calibAmplitude[1] = {{0, 0, 0, 0}};

  m_calibTime[0] = {{0, 0, 0, 0}};
  m_calibTime[1] = {{0, 0, 0, 0}};

  m_dataLoaded[0] = {{false, false, false, false}};
  m_dataLoaded[1] = {{false, false, false, false}};

  m_delayedOrder[0] = {0, 0, 0, 0};
  m_delayedOrder[1] = {0, 0, 0, 0};

  // For now we are using hard-coded gain factors and pedestals
  //
  m_HGGains[0] = {{10, 10, 10, 10}};
  m_HGGains[1] = {{10, 10, 10, 10}};

  m_pedestals[0] = {{100, 100, 100, 100}};
  m_pedestals[1] = {{100, 100, 100, 100}};
  
  // Construct the per-module pulse analyzers
  //
  for (size_t side : {0, 1}) {
    for (size_t module : {0, 1, 2, 3}) {
      std::string moduleTag= "_s" + std::to_string(side) + "_m" +std::to_string(module);
      m_moduleAnalyzers[side][module] = make_unique<ZDCPulseAnalyzer>(m_msgFunc_p, std::move(moduleTag), nSample, deltaTSample, preSampleIdx,
								      m_pedestals[side][module], m_HGGains[side][module], fitFunction,
								      peak2ndDerivMinSamples[side][module],
								      peak2ndDerivMinThresholdsHG[side][module],
								      peak2ndDerivMinThresholdsLG[side][module]);
      m_moduleAnalyzers[side][module]->setLGMode(LGMode);
    }
  }
}

ZDCDataAnalyzer::ZDCDataAnalyzer(ZDCMsg::MessageFunctionPtr msgFunc_p, const JSON& configJSON) :
  m_msgFunc_p(msgFunc_p)
{
  init();
  
  // Construct the object that will extract the data and pulse analyzer
  //   configurations from the input JSON configuration
  //
  //  For the data anlyzer we use 1 channel since the relevant
  //    configurations will all be per-side
  //
  m_dataAnalyzerConfig = std::make_unique<ZDCJSONConfig>(std::vector<std::string>{"C", "A"}, 1);
  m_pulseAnalyzerConfig = std::make_unique<ZDCJSONConfig>(std::vector<std::string>{"C", "A"}, 4);


  // Extract the JSON object for the pulse analyzer(s)
  //
  JSON DAconfig = configJSON["DataAnalyzer"];
  if (DAconfig.is_null()) {
    (*m_msgFunc_p)(ZDCMsg::Fatal, "JSON configuration object for ZDCDataAnalyzer not found");
    return;
  }
  
  auto [result, resultStr] = m_dataAnalyzerConfig->ParseConfig(DAconfig, ZDCDataAnalyzer::JSONConfigParams);
  if (!result) {
    (*m_msgFunc_p)(ZDCMsg::Fatal, "Error parsing ZDCDataAnalyzer JSON config, error = " + resultStr);
    return;
  }
  
  // The data analyzer configuration is simple enough we handle it inline
  //
  for (size_t side : {0, 1}) {
    for (size_t module : {0, 1, 2, 3}) {
      m_moduleEnabled[side][module] = true;
    }

    // Now extract information from ZDCDataAnalyzer-specific configuration
    //
    JSON sideConfig = m_pulseAnalyzerConfig->getChannelConfig(side, 0);
    JSON modEnable = sideConfig["moduleEnabled"];
    if (!modEnable.is_null()) {
      if (modEnable.size() != 4) {
	(*m_msgFunc_p)(ZDCMsg::Fatal, "Error parsing ZDCDataAnalyzer JSON config, incorrect size of moduleEnabled");
	return;
      }
      
      for (size_t module : {0, 1, 2, 3}) {
	m_moduleEnabled[side][module] = modEnable[module];
      }
    }
  }

  // Extract the JSON object for the pulse analyzer(s)
  //
  JSON PAconfig = configJSON["PulseAnalyzer"];
  if (PAconfig.is_null()) {
    (*m_msgFunc_p)(ZDCMsg::Fatal, "JSON configuration object for ZDCPulseAnalyzer not found");
    return;
  }

  // Do the parsing of the pulse analyzer JSON configuration
  //
  auto [result2, resultStr2] = m_pulseAnalyzerConfig->ParseConfig(PAconfig, ZDCPulseAnalyzer::JSONConfigParams);
  if (!result2) {
    (*m_msgFunc_p)(ZDCMsg::Fatal, "Error parsing ZDCPulseAnalyzer JSON config, error = " + resultStr2);
    return;
  }

  // Now set up each of the pulse analyzers
  //
  for (size_t side : {0, 1}) {
    for (size_t module : {0, 1, 2, 3}) {
      
      (*m_msgFunc_p)(ZDCMsg::Info, "Setting up ZDCPulseAnalyzer for side " + std::to_string(side) +
		     ", module " + std::to_string(module));

      //
      // Get the parsed configuration JSON object for this module
      //
      JSON moduleConfig = m_pulseAnalyzerConfig->getChannelConfig(side, module);
      
      std::ostringstream ostr;
      ostr << "JSON configuration for ZDC pulse analyyzer for side " << std::to_string(side)
	   << ", module " <<  std::to_string(module) << "\n" <<  moduleConfig.dump(2);
      (*m_msgFunc_p)(ZDCMsg::Verbose, ostr.str().c_str());

      // Construct the ZDCPulseAnalyzer object
      //
      m_moduleAnalyzers[side][module] = std::make_unique<ZDCPulseAnalyzer>(msgFunc_p, moduleConfig);

      (*m_msgFunc_p)(ZDCMsg::Info, "Finished constructing ZDCPulseAnalyzer for side " + std::to_string(side) +
		     ", module " + std::to_string(module));

    }
  }

  // Check for the enabling of re-pass
  //
  getPulseAnalyzerGlobalPar("enableRepass", m_repassEnabled);
  
  (*m_msgFunc_p)(ZDCMsg::Info, "ZDCDataAnalyzer construction complete");
}

void ZDCDataAnalyzer::init()
{
  for (size_t side : {0, 1}) {
    m_moduleSum[side] = 0;
    m_moduleSumErrSq[side] = 0;
    m_moduleSumPreSample[side] = 0;
    m_calibModuleSum[side] = 0;
    m_calibModuleSumErrSq[side] = 0;

    m_NLcalibModuleSum[side] = 0;
    m_NLcalibModuleSumErrSq[side] = 0;
    m_averageTime[side] = 0;
    m_fail[side] = 0;

    for (size_t module : {0, 1, 2, 3}) {
      m_moduleEnabled[side][module] = true;
      m_calibAmplitude[side][module] = 0;
      m_calibTime[side][module] = 0;

      m_dataLoaded[side][module] = false;
      m_delayedOrder[side][module] = 0;

      // Default "calibrations"
      //
      m_currentECalibCoeff[side][module] = 1;
      m_currentT0OffsetsHG[side][module] = 0;
      m_currentT0OffsetsLG[side][module] = 0;

    }
  }

  m_NLcalibFactors = {{
      {{ {{0,0,0,0,0,0}},{{0,0,0,0,0,0}},{{0,0,0,0,0,0}} }},
      {{ {{0,0,0,0,0,0}},{{0,0,0,0,0,0}},{{0,0,0,0,0,0}} }}  }};

}

bool ZDCDataAnalyzer::disableModule(size_t side, size_t module)
{
  if (side < 2 && module < 4) {
    //
    // Can't disable in the middle of analysis
    //
    if (m_dataLoaded[side][module]) return false;
    else {
      m_moduleEnabled[side][module] = false;
      return true;
    }
  }
  else {
    return false;
  }
}

void ZDCDataAnalyzer::enableDelayed(float deltaT, const ZDCModuleFloatArray& undelayedDelayedPedestalDiff)
{
  int delayedOrder = deltaT < 0 ? -1 : 1;
  for (size_t side : {0, 1}) {
    for (size_t module : {0, 1, 2, 3}) {
      m_delayedOrder[side][module] = delayedOrder;
      m_moduleAnalyzers[side][module]->enableDelayed(std::abs(deltaT), undelayedDelayedPedestalDiff[side][module]);
    }
  }
}

void ZDCDataAnalyzer::enableDelayed(const ZDCModuleFloatArray& delayDeltaTArray, const ZDCModuleFloatArray& undelayedDelayedPedestalDiff)
{
  for (size_t side : {0, 1}) {
    for (size_t module : {0, 1, 2, 3}) {
      if (delayDeltaTArray[side][module] < 0) m_delayedOrder[side][module] = -1;
      else m_delayedOrder[side][module] = 1;

      (*m_msgFunc_p)(ZDCMsg::Verbose, "Enabling use of delayed samples on side, module = " + std::to_string(side) + ", " +
                     std::to_string(module) + ", delta t = " + std::to_string(delayDeltaTArray[side][module]));

      m_moduleAnalyzers[side][module]->enableDelayed(std::abs(delayDeltaTArray[side][module]), undelayedDelayedPedestalDiff[side][module]);
    }
  }
}

void ZDCDataAnalyzer::enablePreExclusion(unsigned int maxSamplesExcl, const ZDCModuleIntArray& HGADCThresh, const ZDCModuleIntArray& LGADCThresh)
{
  for (size_t side : {0, 1}) {
    for (size_t module : {0, 1, 2, 3}) {
      m_moduleAnalyzers[side][module]->enablePreExclusion(maxSamplesExcl, HGADCThresh[side][module], LGADCThresh[side][module]);
    }
  }
}

void ZDCDataAnalyzer::enablePreExclusion(unsigned int maxSamplesExcl, unsigned int HGADCThresh, unsigned int LGADCThresh)
{
  for (size_t side : {0, 1}) {
    for (size_t module : {0, 1, 2, 3}) {
      m_moduleAnalyzers[side][module]->enablePreExclusion(maxSamplesExcl, HGADCThresh, LGADCThresh);
    }
  }
}

void ZDCDataAnalyzer::enablePostExclusion(unsigned int maxSamplesExcl, const ZDCModuleIntArray& HGADCThresh, const ZDCModuleIntArray& LGADCThresh)
{
  for (size_t side : {0, 1}) {
    for (size_t module : {0, 1, 2, 3}) {
      m_moduleAnalyzers[side][module]->enablePostExclusion(maxSamplesExcl, HGADCThresh[side][module], LGADCThresh[side][module]);
    }
  }
}

void ZDCDataAnalyzer::enablePostExclusion(unsigned int maxSamplesExcl, unsigned int HGADCThresh, unsigned int LGADCThresh)
{
  for (size_t side : {0, 1}) {
    for (size_t module : {0, 1, 2, 3}) {
      m_moduleAnalyzers[side][module]->enablePostExclusion(maxSamplesExcl, HGADCThresh, LGADCThresh);
    }
  }
}


void ZDCDataAnalyzer::enableRepass(const ZDCModuleFloatArray& peak2ndDerivMinRepassHG, const ZDCModuleFloatArray& peak2ndDerivMinRepassLG)
{
  m_repassEnabled = true;
  for (size_t side : {0, 1}) {
    for (size_t module : {0, 1, 2, 3}) {
      m_moduleAnalyzers[side][module]->enableRepass(peak2ndDerivMinRepassHG[side][module], peak2ndDerivMinRepassLG[side][module]);
    }
  }
}

void ZDCDataAnalyzer::setMinimumSignificance(float sigMinHG, float sigMinLG)
{
  for (size_t side : {0, 1}) {
    for (size_t module : {0, 1, 2, 3}) {
      m_moduleAnalyzers[side][module]->setMinimumSignificance(sigMinHG, sigMinLG);
    }
  }
}

void ZDCDataAnalyzer::set2ndDerivStep(size_t step)
{
  for (size_t side : {0, 1}) {
    for (size_t module : {0, 1, 2, 3}) {
      m_moduleAnalyzers[side][module]->set2ndDerivStep(step);
    }
  }
}

void ZDCDataAnalyzer::SetGainFactorsHGLG(float gainFactorHG, float gainFactorLG)
{
  for (size_t side : {0, 1}) {
    for (size_t module : {0, 1, 2, 3}) {
      m_moduleAnalyzers[side][module]->SetGainFactorsHGLG(gainFactorHG, gainFactorLG);
    }
  }
}

void ZDCDataAnalyzer::SetGainFactorsHGLG(const ZDCModuleFloatArray& gainFactorsHG, const ZDCModuleFloatArray& gainFactorsLG)
{
  for (size_t side : {0, 1}) {
    for (size_t module : {0, 1, 2, 3}) {
      m_moduleAnalyzers[side][module]->SetGainFactorsHGLG(gainFactorsHG[side][module], gainFactorsLG[side][module]);
    }
  }
}

void ZDCDataAnalyzer::SetPeak2ndDerivMinTolerances(size_t tolerance) {
  for (size_t side : {0, 1}) {
    for (size_t module : {0, 1, 2, 3}) {
      m_moduleAnalyzers[side][module]->SetPeak2ndDerivMinTolerance(tolerance);
    }
  }
}


void ZDCDataAnalyzer::SetFitTimeMax(float tmax) {
  for (size_t side : {0, 1}) {
    for (size_t module : {0, 1, 2, 3}) {
      m_moduleAnalyzers[side][module]->SetFitTimeMax(tmax);
    }
  }
}



void ZDCDataAnalyzer::SetTauT0Values(const ZDCModuleBoolArray& fixTau1, const ZDCModuleBoolArray& fixTau2,
                                     const ZDCModuleFloatArray& tau1, const ZDCModuleFloatArray& tau2,
                                     const ZDCModuleFloatArray& t0HG, const ZDCModuleFloatArray& t0LG)
{
  for (size_t side : {0, 1}) {
    for (size_t module : {0, 1, 2, 3}) {
      m_moduleAnalyzers[side][module]->SetTauT0Values(fixTau1[side][module], fixTau2[side][module],
          tau1[side][module], tau2[side][module], t0HG[side][module], t0LG[side][module]);
    }
  }
}

void ZDCDataAnalyzer::SetNoiseSigmas(const ZDCModuleFloatArray& noiseSigmasHG, const ZDCModuleFloatArray& noiseSigmasLG)
{
  for (size_t side : {0, 1}) {
    for (size_t module : {0, 1, 2, 3}) {
      m_moduleAnalyzers[side][module]->SetNoiseSigmas(noiseSigmasHG[side][module], noiseSigmasLG[side][module]);
    }
  }
}

void ZDCDataAnalyzer::SetModuleAmpFractionLG(const ZDCDataAnalyzer::ZDCModuleFloatArray& moduleAmpFractionLG) {
  for (size_t side : {0, 1}) {
    for (size_t module : {0, 1, 2, 3}) {
      m_moduleAmpFractionLG[side][module] = moduleAmpFractionLG[side][module];
    }
  }
}

void ZDCDataAnalyzer::SetFitMinMaxAmpValues(const ZDCModuleFloatArray& minAmpHG, const ZDCModuleFloatArray& minAmpLG,
    const ZDCModuleFloatArray& maxAmpHG, const ZDCModuleFloatArray& maxAmpLG)
{
  for (size_t side : {0, 1}) {
    for (size_t module : {0, 1, 2, 3}) {
      m_moduleAnalyzers[side][module]->SetFitMinMaxAmp(minAmpHG[side][module], minAmpLG[side][module],
          maxAmpHG[side][module], maxAmpLG[side][module]);

    }
  }
}

void ZDCDataAnalyzer::SetFitMinMaxAmpValues(float minHG, float minLG, float maxHG, float maxLG)
{
  for (size_t side : {0, 1}) {
    for (size_t module : {0, 1, 2, 3}) {
      m_moduleAnalyzers[side][module]->SetFitMinMaxAmp(minHG, minLG, maxHG, maxLG);
    }
  }
}

void ZDCDataAnalyzer::SetADCOverUnderflowValues(const ZDCModuleFloatArray& HGOverflowADC, const ZDCModuleFloatArray& HGUnderflowADC,
    const ZDCModuleFloatArray& LGOverflowADC)
{
  for (size_t side : {0, 1}) {
    for (size_t module : {0, 1, 2, 3}) {
      m_moduleAnalyzers[side][module]->SetADCOverUnderflowValues(HGOverflowADC[side][module], HGUnderflowADC[side][module], LGOverflowADC[side][module]);
    }
  }
}

void ZDCDataAnalyzer::SetCutValues(const ZDCModuleFloatArray& chisqDivAmpCutHG, const ZDCModuleFloatArray& chisqDivAmpCutLG,
                                   const ZDCModuleFloatArray& deltaT0MinHG, const ZDCModuleFloatArray& deltaT0MaxHG,
                                   const ZDCModuleFloatArray&  deltaT0MinLG, const ZDCModuleFloatArray& deltaT0MaxLG)
{
  for (size_t side : {0, 1}) {
    for (size_t module : {0, 1, 2, 3}) {
      m_moduleAnalyzers[side][module]->SetCutValues(chisqDivAmpCutHG[side][module], chisqDivAmpCutLG[side][module],
          deltaT0MinHG[side][module], deltaT0MaxHG[side][module],
          deltaT0MinLG[side][module], deltaT0MaxLG[side][module]);
    }
  }
}

void ZDCDataAnalyzer::SetTimingCorrParams(ZDCPulseAnalyzer::TimingCorrMode mode, float refADC, float refScale,
					  const std::array<std::array<std::vector<float>, 4>, 2>& HGParamArr,
					  const std::array<std::array<std::vector<float>, 4>, 2>& LGParamArr)
{
  for (size_t side : {0, 1}) {
    for (size_t module : {0, 1, 2, 3}) {
      m_moduleAnalyzers[side][module]->SetTimingCorrParams(mode, refADC, refScale,
							   HGParamArr.at(side).at(module), LGParamArr.at(side).at(module));
    }
  }

}

void ZDCDataAnalyzer::SetNonlinCorrParams(float refADC, float refScale,
					  const std::array<std::array<std::vector<float>, 4>, 2>& HGNonlinCorrParams,
					  const std::array<std::array<std::vector<float>, 4>, 2>& LGNonlinCorrParams)
{
  for (size_t side : {0, 1}) {
    for (size_t module : {0, 1, 2, 3}) {
      m_moduleAnalyzers[side][module]->SetNonlinCorrParams(refADC, refScale,
							   HGNonlinCorrParams[side][module],
							   LGNonlinCorrParams[side][module]);
    }
  }
}

void ZDCDataAnalyzer::SetNLcalibParams(std::array< std::array< std::array<float,6>, 3>, 2>& nlcalibParams)
{
  for (size_t side: {0,1})
    {
      for (size_t module: {0,1,2})
	{
	  for (size_t val: {0,1,2,3,4,5})
	    {
	      m_NLcalibFactors[side][module][val] = nlcalibParams[side][module][val];
	    }
	}
    }
  m_haveNLcalib = true;
}

void ZDCDataAnalyzer::enableFADCCorrections(bool correctPerSample,
					    std::array<std::array<std::unique_ptr<const TH1>, 4>, 2>& corrHistHG,
					    std::array<std::array<std::unique_ptr<const TH1>, 4>, 2>& corrHistLG)
{
  if (correctPerSample)
    (*m_msgFunc_p)(ZDCMsg::Info, "ZDCDataAnalyzer::enabling FADC Corrections per sample");
  else
    (*m_msgFunc_p)(ZDCMsg::Info, "ZDCDataAnalyzer::enabling FADC Corrections per amplitude");
    
  for (size_t side : {0, 1}) {
    for (size_t module : {0, 1, 2, 3}) {
      m_moduleAnalyzers[side][module]->enableFADCCorrections(correctPerSample, corrHistHG[side][module], corrHistLG[side][module]);
    }
  }
}

void ZDCDataAnalyzer::disableFADCCorrections()
{
  for (size_t side : {0, 1}) {
    for (size_t module : {0, 1, 2, 3}) {
      m_moduleAnalyzers[side][module]->disableFADCCorrections();
    }
  }
}

void ZDCDataAnalyzer::enableTimeSigCut(bool AND, float sigCut, const std::string& TF1String,
				       const std::array<std::array<std::vector<double>, 4>, 2>& parsHGArr, 
				       const std::array<std::array<std::vector<double>, 4>, 2>& parsLGArr)
{
  for (size_t side : {0, 1}) {
    for (size_t module : {0, 1, 2, 3}) {
      m_moduleAnalyzers[side][module]->enableTimeSigCut(AND, sigCut, TF1String, parsHGArr[side][module], parsLGArr[side][module]);
    }
  }
}

void ZDCDataAnalyzer::StartEvent(int lumiBlock)
{
  (*m_msgFunc_p)(ZDCMsg::Verbose, ("Starting new event, event index = " + std::to_string(m_eventCount)));

  // By default we perform quiet pulse fits
  //
  /*
  if ((*m_msgFunc_p)(ZDCMsg::Verbose, "")) {
    invokeAll([](ZDCPulseAnalyzer* pa){pa->setQuietFits();});
  }
  else {
    invokeAll([](ZDCPulseAnalyzer* pa){pa->setQuietFits();});
  }
  */
  
  //  See if we have to load up new calibrations
  //
  if (lumiBlock != m_currentLB) {
    (*m_msgFunc_p)(ZDCMsg::Verbose,  ("Starting new luminosity block " + std::to_string(lumiBlock)));

    if (m_haveECalib) {
      (*m_msgFunc_p)(ZDCMsg::Verbose, ("Loading energy calibrations for event " + std::to_string(m_eventCount) + ", lumi block " +
                                       std::to_string(lumiBlock)));

      for (size_t side : {0, 1}) {
        for (size_t module : {0, 1, 2, 3}) {
          float splineLBMin = m_LBDepEcalibSplines[side][module]->GetXmin();
          float splineLBMax = m_LBDepEcalibSplines[side][module]->GetXmax();

          if (lumiBlock >= splineLBMin && lumiBlock <= splineLBMax) {
            m_currentECalibCoeff[side][module] = m_LBDepEcalibSplines[side][module]->Eval(lumiBlock);
          }
          else if (lumiBlock < splineLBMin) {
            m_currentECalibCoeff[side][module] = m_LBDepEcalibSplines[side][module]->Eval(splineLBMin);
          }
          else {
            m_currentECalibCoeff[side][module] = m_LBDepEcalibSplines[side][module]->Eval(splineLBMax);
          }
        }
      }
    } // end of if (_haveEcalib) {

    if (m_haveT0Calib) {
      (*m_msgFunc_p)(ZDCMsg::Verbose, ("Loading timing calibrations for event " + std::to_string(m_eventCount) + ", lumi block " + std::to_string(lumiBlock)));

      for (size_t side : {0, 1}) {
        for (size_t module : {0, 1, 2, 3}) {
          float splineLBMin = m_T0HGOffsetSplines[side][module]->GetXmin();
          float splineLBMax = m_T0HGOffsetSplines[side][module]->GetXmax();

          if (lumiBlock >= splineLBMin && lumiBlock <= splineLBMax) {
            m_currentT0OffsetsHG[side][module] = m_T0HGOffsetSplines[side][module]->Eval(lumiBlock);
            m_currentT0OffsetsLG[side][module] = m_T0LGOffsetSplines[side][module]->Eval(lumiBlock);
          }
          else if (lumiBlock < splineLBMin) {
            m_currentT0OffsetsHG[side][module] = m_T0HGOffsetSplines[side][module]->Eval(splineLBMin);
            m_currentT0OffsetsLG[side][module] = m_T0LGOffsetSplines[side][module]->Eval(splineLBMin);
          }
          else {
            m_currentT0OffsetsHG[side][module] = m_T0HGOffsetSplines[side][module]->Eval(splineLBMax);
            m_currentT0OffsetsLG[side][module] = m_T0LGOffsetSplines[side][module]->Eval(splineLBMax);
          }
        }
      }
    } // end of if (m_haveT0Calib)
  }

  // Initialize transient results
  //
  for (size_t side : {0, 1}) {
    for (size_t module : {0, 1, 2, 3}) {
      m_dataLoaded[side][module] = false;
      m_moduleStatus[side][module] = 0;
      m_calibAmplitude[side][module] = 0;
      m_calibTime[side][module] = 0;
      //      _moduleFail[side][module] = false;
    }

    m_moduleSum[side] = 0;
    m_moduleSumErrSq[side] = 0;
    m_moduleSumPreSample[side] = 0;
    m_moduleSumBkgdFrac[side] = 0;

    m_calibModuleSum[side] = 0;
    m_calibModuleSumErrSq[side] = 0;
    m_calibModSumBkgdFrac[side] = 0;

    m_NLcalibModuleSum[side] = 0;
    m_NLcalibModuleSumErrSq[side] = 0;
    m_NLcalibModSumBkgdFrac[side] = 0;
    
    m_averageTime[side] = 0;
    m_fail[side] = false;
  }

  m_moduleMask = 0;
  m_currentLB = lumiBlock;
}

void ZDCDataAnalyzer::LoadAndAnalyzeData(size_t side, size_t module, const std::vector<float>& HGSamples, const std::vector<float>& LGSamples)
{

  // We immediately return if this module is disabled
  //
  if (!m_moduleEnabled[side][module]) {
    (*m_msgFunc_p)(ZDCMsg::Verbose, ("Skipping analysis of disabled module for event index " + std::to_string(m_eventCount) + ", side, module = " + std::to_string(side) + ", " + std::to_string(module)));

    return;
  }

  (*m_msgFunc_p)(ZDCMsg::Verbose, ("/n Loading data for event index " + std::to_string(m_eventCount) + ", side, module = " + std::to_string(side) + ", " + std::to_string(module)));

  ZDCPulseAnalyzer* pulseAna_p = m_moduleAnalyzers[side][module].get();
  pulseAna_p->LoadAndAnalyzeData(HGSamples, LGSamples);
  m_dataLoaded[side][module] = true;

  if (pulseAna_p->failed()) {
    (*m_msgFunc_p)(ZDCMsg::Debug, ("ZDCPulseAnalyzer::LoadData() returned fail for event " + std::to_string(m_eventCount) + ", side, module = " + std::to_string(side) + ", " + std::to_string(module)));

    m_fail[side] = true;
  }

  m_moduleStatus[side][module] = pulseAna_p->GetStatusMask();
}

void ZDCDataAnalyzer::LoadAndAnalyzeData(size_t side, size_t module, const std::vector<float>& HGSamples, const std::vector<float>& LGSamples,
					 const std::vector<float>& HGSamplesDelayed, const std::vector<float>& LGSamplesDelayed)
{
  // We immediately return if this module is disabled
  //
  if (!m_moduleEnabled[side][module]) {
    (*m_msgFunc_p)(ZDCMsg::Debug,  ("Skipping analysis of disabled mofule for event index " + std::to_string(m_eventCount) + ", side, module = " + std::to_string(side) + ", " + std::to_string(module)));

    return;
  }

  if (m_delayedOrder[side][module] == 0) {
    (*m_msgFunc_p)(ZDCMsg::Error, ("Handling of delayed pulses not enabled, on side, module = " + std::to_string(side) +  ", " + std::to_string(module) + ", skipping processing for event index " + std::to_string(m_eventCount)));
    return;
  }

  (*m_msgFunc_p)(ZDCMsg::Verbose, ("Loading undelayed and delayed data for event index " + std::to_string(m_eventCount) + ", side, module = " + std::to_string(side) +  ", " + std::to_string(module)));

  ZDCPulseAnalyzer* pulseAna_p = m_moduleAnalyzers[side][module].get();
  if (m_delayedOrder[side][module] > 0) {
    pulseAna_p->LoadAndAnalyzeData(HGSamples, LGSamples, HGSamplesDelayed, LGSamplesDelayed);
  }
  else {
    pulseAna_p->LoadAndAnalyzeData(HGSamplesDelayed, LGSamplesDelayed, HGSamples, LGSamples);
  }
  m_dataLoaded[side][module] = true;

  if (pulseAna_p->failed()) {
    (*m_msgFunc_p)(ZDCMsg::Debug, ("ZDCPulseAnalyzer::LoadData() returned fail for event " + std::to_string(m_eventCount) + ", side, module = " + std::to_string(side) + ", " + std::to_string(module)));

    m_fail[side] = true;
  }

  m_moduleStatus[side][module] = pulseAna_p->GetStatusMask();
}

bool ZDCDataAnalyzer::FinishEvent()
{
  // First make sure that all data is loaded. while we're at it, count how many modules on each side have a pulse
  //
  unsigned int sideNPulsesMod[2] = {0, 0};

  for (size_t side : {0, 1}) {
    for (size_t module : {0, 1, 2, 3}) {
      if (!m_dataLoaded[side][module] && m_moduleEnabled[side][module]) {return false;}
      if (m_moduleAnalyzers[side][module]->armSumInclude()) {sideNPulsesMod[side]++;}
    }
  }

  // Are we doing a repass? If so, reanalyze modules for which no pulse was found the first time
  //   as long as we have one module with a pulse on the given side
  //
  if (m_repassEnabled) {
    for (size_t side : {0, 1}) {
      if (sideNPulsesMod[side] == 0) continue;

      for (size_t module : {0, 1, 2, 3}) {
	if (!m_moduleEnabled[side][module]) continue;

        ZDCPulseAnalyzer* pulseAna_p = m_moduleAnalyzers[side][module].get();

        // If this module had no pulse the first time, reanalyze it (with a lower 2nd derivative threshold)
        //
        if (!pulseAna_p->havePulse()) {
          (*m_msgFunc_p)(ZDCMsg::Debug, ("ZDCPulseAnalyzer:: performing a repass on data for side, module = " + std::to_string(side) + ", " + std::to_string(module)));
          pulseAna_p->ReanalyzeData();
	  m_moduleStatus[side][module] = pulseAna_p->GetStatusMask();
        }
      }
    }
  }

  // Now sum up amplitudes etc
  //
  for (size_t side : {0, 1}) {
    float tempFraction = 1.0;
    double sumAmpTimesBkgdFrac = 0.0;
    double sumCalibAmpTimesBkgdFrac = 0.0;
      
    for (size_t module : {0, 1, 2, 3}) {
      ZDCPulseAnalyzer* pulseAna_p = m_moduleAnalyzers[side][module].get();

      if (pulseAna_p->armSumInclude()) {
        int moduleMaskBit = 4 * side + module;
        m_moduleMask |= 1 << moduleMaskBit;

        float amplitude = pulseAna_p->GetAmplitude();
        float ampError = pulseAna_p->GetAmpError();
        float bkgdFraction = pulseAna_p->GetBkgdMaxFraction();

        m_calibAmplitude[side][module] = amplitude * m_currentECalibCoeff[side][module];

        float calibAmpError = ampError * m_currentECalibCoeff[side][module];

        float timeCalib = pulseAna_p->GetT0Corr();
        if (pulseAna_p->useLowGain()) {timeCalib -= m_currentT0OffsetsLG[side][module];}
        else {timeCalib -= m_currentT0OffsetsHG[side][module];}

        m_calibTime[side][module] = timeCalib;

        m_moduleSum[side] += amplitude;
        m_moduleSumErrSq[side] += ampError * ampError;
	sumAmpTimesBkgdFrac += amplitude*bkgdFraction;

        m_moduleSumPreSample[side] += pulseAna_p->GetPreSampleAmp();

        m_calibModuleSum[side] += m_calibAmplitude[side][module];
        m_calibModuleSumErrSq[side] += calibAmpError * calibAmpError;

        m_averageTime[side] += m_calibTime[side][module] * m_calibAmplitude[side][module];
	sumCalibAmpTimesBkgdFrac += amplitude*bkgdFraction;
      }

      // subtract the fraction of LGOverflow events if we have fraction available (<0 means unavailable)
      if (pulseAna_p->LGOverflow() && m_moduleAmpFractionLG[side][module] > 0) {tempFraction -= m_moduleAmpFractionLG[side][module];}
    }

    {
      CXXUTILS_TRAPPING_FP;
      if (m_moduleSum[side] > 0) m_moduleSumBkgdFrac[side] = sumAmpTimesBkgdFrac/m_moduleSum[side];
      else m_moduleSumBkgdFrac[side] = 0;
    }
    
    if (m_calibModuleSum[side] > 1e-6) {
      m_averageTime[side] /= m_calibModuleSum[side];
      m_calibModSumBkgdFrac[side] = sumCalibAmpTimesBkgdFrac/m_calibModuleSum[side];
    }
    else {
      m_averageTime[side] = 0;
      m_calibModSumBkgdFrac[side] = 0;
    }
    
    if (tempFraction < 1.0) {m_moduleSum[side] /= tempFraction;}
  }
  
  DoNLcalibModuleSum();
  
  m_eventCount++;
  return true;
}

void ZDCDataAnalyzer::DoNLcalibModuleSum()
{
  if (!m_haveNLcalib) return;
  
  for (int iside:{0,1})
    {
      if (m_calibModuleSum[iside]>0.)
	{
	  float fEM = m_calibAmplitude[iside][0] / m_calibModuleSum[iside];
	  float fHad1 = m_calibAmplitude[iside][1] / m_calibModuleSum[iside];
	  float fHad2 = m_calibAmplitude[iside][2] / m_calibModuleSum[iside];
	  
	  float EMCorrFact = 0;
	  
	  for (size_t i=0;i<m_NLcalibFactors[iside][0].size()-1;i++)
	    {	  
	      EMCorrFact += std::pow(fEM - m_NLcalibFactors[iside][0][0],i)*m_NLcalibFactors[iside][0][i+1];
	    }
	  
	  float Had1CorrFact = 0;
	  for (size_t i=0;i<m_NLcalibFactors[iside][1].size()-1;i++)
	    {
	      Had1CorrFact += std::pow(fHad1 - m_NLcalibFactors[iside][1][0],i)*m_NLcalibFactors[iside][1][i+1];
	    }
	  
	  float Had2CorrFact = 0;
	  for (size_t i=0;i<m_NLcalibFactors[iside][2].size()-1;i++)
	    {
	      Had2CorrFact += std::pow(fHad2 - m_NLcalibFactors[iside][2][0],i)*m_NLcalibFactors[iside][2][i+1];
	    }

	  std::ostringstream ostr;
	  ostr << "ZDCDataAnalyzer: " << m_calibModuleSum[iside] << " " << EMCorrFact << " " << Had1CorrFact << " " << Had2CorrFact << std::endl;
	  (*m_msgFunc_p)(ZDCMsg::Debug,ostr.str().c_str());
	  
	  float ECorrEM = m_calibModuleSum[iside]/EMCorrFact;
	  float ECorrEMHad1 = ECorrEM/Had1CorrFact;
	  float ECorrEMHad1Had2 = ECorrEMHad1/Had2CorrFact;
	  
	  
	  m_NLcalibModuleSum[iside] = ECorrEMHad1Had2;
	  m_NLcalibModuleSumErrSq[iside] = 0.; // no error for now
	}
      else
	{
	  (*m_msgFunc_p)(ZDCMsg::Info,"SUM = 0!!");
	  m_NLcalibModuleSum[iside] = 0.;
	  m_NLcalibModuleSumErrSq[iside] = 0.; // no error for now
	}
    }

}
