/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include <AsgDataHandles/WriteDecorHandle.h>

#include "ZdcAnalysis/RPDAnalysisTool.h"
#include "xAODEventInfo/EventInfo.h"
#include "ZdcUtils/ZdcEventInfo.h"

namespace ZDC {

RPDAnalysisTool::RPDAnalysisTool(std::string const& name) : asg::AsgTool(name) {
  declareProperty("ZdcModuleContainerName", m_ZDCModuleContainerName = "ZdcModules", "Location of ZDC processed data");
  declareProperty("ZdcSumContainerName", m_ZDCSumContainerName = "ZdcSums", "Location of ZDC processed sums");
  declareProperty("WriteAux", m_writeAux = true);
  declareProperty("AuxSuffix", m_auxSuffix = "");

  declareProperty("RpdNSamples", m_nSamples = 24, "Total number of FADC samples in readout window");
  declareProperty("RpdNbaselineSamples", m_nBaselineSamples = 7, "Number of baseline samples; the sample equal to this number is the start of signal region");
  declareProperty("RpdEndSignalSample", m_endSignalSample = 23, "Samples before (not including) this sample are the signal region; 0 or Nsamples goes to end of window");
  declareProperty("RpdPulse2ndDerivThresh", m_pulse2ndDerivThresh = -18, "Second differences less than or equal to this number indicate a pulse");
  declareProperty("RpdPostPulseFracThresh", m_postPulseFracThresh = 0.15, "If there is a good pulse and post-pulse and size of post-pulse as a fraction of good pulse is less than or equal to this number, ignore post-pulse");
  declareProperty("RpdGoodPulseSampleStart", m_goodPulseSampleStart = 8, "Pulses before this sample are considered pre-pulses");
  declareProperty("RpdGoodPulseSampleStop", m_goodPulseSampleStop = 10, "Pulses after this sample are considered post-pulses");
  declareProperty("RpdNominalBaseline", m_nominalBaseline = 100, "The global nominal baseline; used when pileup is detected");
  declareProperty("RpdPileupBaselineSumThresh", m_pileupBaselineSumThresh = 53, "Baseline sum (after subtracting nominal baseline) less than this number indicates there is NO pileup");
  declareProperty("RpdPileupBaselineStdDevThresh", m_pileupBaselineStdDevThresh = 2, "Baseline standard deviations less than this number indicate there is NO pileup");
  declareProperty("RpdNNegativesAllowed", m_nNegativesAllowed = 2, "Maximum number of negative ADC values after baseline and pileup subtraction allowed in signal range");
  declareProperty("RpdAdcOverflow", m_ADCOverflow = 4095, "ADC values greater than or equal to this number are considered overflow");
  declareProperty("RpdSideCCalibFactors", m_outputCalibFactors.at(RPDUtils::sideC) = std::vector<float>(16, 1.0), "Multiplicative calibration factors to apply to RPD output, e.g., sum/max ADC, per channel on side C");
  declareProperty("RpdSideACalibFactors", m_outputCalibFactors.at(RPDUtils::sideA) = std::vector<float>(16, 1.0), "Multiplicative calibration factors to apply to RPD output, e.g., sum/max ADC, per channel on side A");
}

StatusCode RPDAnalysisTool::initializeKey(std::string const& containerName, SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> & writeHandleKey, std::string const& key) {
  writeHandleKey = containerName + key + m_auxSuffix;
  return writeHandleKey.initialize();
}

StatusCode RPDAnalysisTool::initialize() {
  RPDConfig config {};
  config.nSamples = m_nSamples;
  config.nBaselineSamples = m_nBaselineSamples;
  config.endSignalSample = m_endSignalSample;
  config.pulse2ndDerivThresh = m_pulse2ndDerivThresh;
  config.postPulseFracThresh = m_postPulseFracThresh;
  config.goodPulseSampleStart = m_goodPulseSampleStart;
  config.goodPulseSampleStop = m_goodPulseSampleStop;
  config.nominalBaseline = m_nominalBaseline;
  config.pileupBaselineSumThresh = m_pileupBaselineSumThresh;
  config.pileupBaselineStdDevThresh = m_pileupBaselineStdDevThresh;
  config.nNegativesAllowed = m_nNegativesAllowed;
  config.AdcOverflow = m_ADCOverflow;
  m_dataAnalyzers.at(RPDUtils::sideC) = std::make_unique<RPDDataAnalyzer>(MakeMessageFunction(), "rpdC", config, m_outputCalibFactors.at(RPDUtils::sideC));
  m_dataAnalyzers.at(RPDUtils::sideA) = std::make_unique<RPDDataAnalyzer>(MakeMessageFunction(), "rpdA", config, m_outputCalibFactors.at(RPDUtils::sideA));

  // initialize per-channel decorations (in ZdcModules)
  ATH_CHECK(initializeKey(m_ZDCModuleContainerName, m_chBaselineKey, ".RPDChannelBaseline"));
  ATH_CHECK(initializeKey(m_ZDCModuleContainerName, m_chPileupExpFitParamsKey, ".RPDChannelPileupExpFitParams"));
  ATH_CHECK(initializeKey(m_ZDCModuleContainerName, m_chPileupStretchedExpFitParamsKey, ".RPDChannelPileupStretchedExpFitParams"));
  ATH_CHECK(initializeKey(m_ZDCModuleContainerName, m_chPileupExpFitParamErrsKey, ".RPDChannelPileupExpFitParamErrs"));
  ATH_CHECK(initializeKey(m_ZDCModuleContainerName, m_chPileupStretchedExpFitParamErrsKey, ".RPDChannelPileupStretchedExpFitParamErrs"));
  ATH_CHECK(initializeKey(m_ZDCModuleContainerName, m_chPileupExpFitMSEKey, ".RPDChannelPileupExpFitMSE"));
  ATH_CHECK(initializeKey(m_ZDCModuleContainerName, m_chPileupStretchedExpFitMSEKey, ".RPDChannelPileupStretchedExpFitMSE"));
  ATH_CHECK(initializeKey(m_ZDCModuleContainerName, m_chAmplitudeKey, ".RPDChannelAmplitude"));
  ATH_CHECK(initializeKey(m_ZDCModuleContainerName, m_chAmplitudeCalibKey, ".RPDChannelAmplitudeCalib"));
  ATH_CHECK(initializeKey(m_ZDCModuleContainerName, m_chMaxADCKey, ".RPDChannelMaxADC"));
  ATH_CHECK(initializeKey(m_ZDCModuleContainerName, m_chMaxADCCalibKey, ".RPDChannelMaxADCCalib"));
  ATH_CHECK(initializeKey(m_ZDCModuleContainerName, m_chMaxSampleKey, ".RPDChannelMaxSample"));
  ATH_CHECK(initializeKey(m_ZDCModuleContainerName, m_chStatusKey, ".RPDChannelStatus"));
  ATH_CHECK(initializeKey(m_ZDCModuleContainerName, m_chPileupFracKey, ".RPDChannelPileupFrac"));

  // initialize per-side decorations (in ZdcSums)
  ATH_CHECK(initializeKey(m_ZDCSumContainerName, m_sideStatusKey, ".RPDStatus"));

  ATH_CHECK( m_eventInfoKey.initialize());

  if (m_writeAux && !m_auxSuffix.empty()) {
    ATH_MSG_DEBUG("Suffix string = " << m_auxSuffix);
  }

  m_initialized = true;
  return StatusCode::SUCCESS;
}

ZDCMsg::MessageFunctionPtr RPDAnalysisTool::MakeMessageFunction() {
  return std::make_shared<ZDCMsg::MessageFunction>(
    [this] (int const messageZdcLevel, const std::string& message) -> bool {
      auto const messageAthenaLevel = static_cast<MSG::Level>(messageZdcLevel);
      bool const passesStreamOutputLevel = messageAthenaLevel >= this->msg().level();
      if (passesStreamOutputLevel) {
        this->msg(messageAthenaLevel) << message << endmsg;
      }
      return passesStreamOutputLevel;
    }
  );
}

void RPDAnalysisTool::reset() {
  for (auto & analyzer : m_dataAnalyzers) {
    analyzer->reset();
  }
}

void RPDAnalysisTool::readAOD(xAOD::ZdcModuleContainer const& moduleContainer) {
  // loop through ZDC modules to find those which are RPD channels
  for (auto const& module : moduleContainer) {
    if (module->zdcType() != RPDUtils::ZDCModuleRPDType) {
      // this is not an RPD channel, so skip it
      continue;
    }
    unsigned int const side = RPDUtils::ZDCSideToSideIndex(module->zdcSide());
    if (module->zdcChannel() < 0 || static_cast<unsigned int>(module->zdcChannel()) > RPDUtils::nChannels - 1) {
      ATH_MSG_ERROR("Invalid RPD channel found on side " << side << ": channel number = " << module->zdcChannel());
    }
    // channel numbers are fixed in mapping in ZdcConditions, numbered 0-15
    unsigned int const channel = module->zdcChannel();
    ATH_MSG_DEBUG("RPD side " << side << " channel " << module->zdcChannel());
    SG::ConstAccessor<std::vector<uint16_t>> accessor("g0data");
    auto const& waveform = accessor(*module);
    m_dataAnalyzers.at(side)->loadChannelData(channel, waveform);
  }
}

void RPDAnalysisTool::analyze() {
  for (auto & analyzer : m_dataAnalyzers) {
    analyzer->analyzeData();
  }
}

void RPDAnalysisTool::writeAOD(xAOD::ZdcModuleContainer const& moduleContainer, xAOD::ZdcModuleContainer const& moduleSumContainer) const {
  if (!m_writeAux) {
    return;
  }

  ATH_MSG_DEBUG("Adding variables with suffix = " + m_auxSuffix);

  // write per-channel decorations (in ZdcModules)
  SG::WriteDecorHandle<xAOD::ZdcModuleContainer, float> chBaseline(m_chBaselineKey);
  SG::WriteDecorHandle<xAOD::ZdcModuleContainer, std::vector<float>> chPileupExpFitParams(m_chPileupExpFitParamsKey);
  SG::WriteDecorHandle<xAOD::ZdcModuleContainer, std::vector<float>> chPileupStretchedExpFitParams(m_chPileupStretchedExpFitParamsKey);
  SG::WriteDecorHandle<xAOD::ZdcModuleContainer, std::vector<float>> chPileupExpFitParamErrs(m_chPileupExpFitParamErrsKey);
  SG::WriteDecorHandle<xAOD::ZdcModuleContainer, std::vector<float>> chPileupStretchedExpFitParamErrs(m_chPileupStretchedExpFitParamErrsKey);
  SG::WriteDecorHandle<xAOD::ZdcModuleContainer, float> chPileupExpFitMSE(m_chPileupExpFitMSEKey);
  SG::WriteDecorHandle<xAOD::ZdcModuleContainer, float> chPileupStretchedExpFitMSE(m_chPileupStretchedExpFitMSEKey);
  SG::WriteDecorHandle<xAOD::ZdcModuleContainer, float> chAmplitude(m_chAmplitudeKey);
  SG::WriteDecorHandle<xAOD::ZdcModuleContainer, float> chAmplitudeCalib(m_chAmplitudeCalibKey);
  SG::WriteDecorHandle<xAOD::ZdcModuleContainer, float> chMaxADC(m_chMaxADCKey);
  SG::WriteDecorHandle<xAOD::ZdcModuleContainer, float> chMaxADCCalib(m_chMaxADCCalibKey);
  SG::WriteDecorHandle<xAOD::ZdcModuleContainer, unsigned int> chMaxSample(m_chMaxSampleKey);
  SG::WriteDecorHandle<xAOD::ZdcModuleContainer, unsigned int> chStatus(m_chStatusKey);
  SG::WriteDecorHandle<xAOD::ZdcModuleContainer, float> chPileupFrac(m_chPileupFracKey);
  for (auto const& module : moduleContainer) {
    if (module->zdcType() != RPDUtils::ZDCModuleRPDType) {
      // this is not an RPD channel, so skip it
      continue;
    }
    unsigned int const side = RPDUtils::ZDCSideToSideIndex(module->zdcSide());
    unsigned int const channel = module->zdcChannel();
    chBaseline(*module) = m_dataAnalyzers.at(side)->getChBaseline(channel);
    chPileupExpFitParams(*module) = m_dataAnalyzers.at(side)->getChPileupExpFitParams(channel);
    chPileupStretchedExpFitParams(*module) = m_dataAnalyzers.at(side)->getChPileupStretchedExpFitParams(channel);
    chPileupExpFitParamErrs(*module) = m_dataAnalyzers.at(side)->getChPileupExpFitParamErrs(channel);
    chPileupStretchedExpFitParamErrs(*module) = m_dataAnalyzers.at(side)->getChPileupStretchedExpFitParamErrs(channel);
    chPileupExpFitMSE(*module) = m_dataAnalyzers.at(side)->getChPileupExpFitMSE(channel);
    chPileupStretchedExpFitMSE(*module) = m_dataAnalyzers.at(side)->getChPileupStretchedExpFitMSE(channel);
    chAmplitude(*module) = m_dataAnalyzers.at(side)->getChSumAdc(channel);
    chAmplitudeCalib(*module) = m_dataAnalyzers.at(side)->getChSumAdcCalib(channel);
    chMaxADC(*module) = m_dataAnalyzers.at(side)->getChMaxAdc(channel);
    chMaxADCCalib(*module) = m_dataAnalyzers.at(side)->getChMaxAdcCalib(channel);
    chMaxSample(*module) = m_dataAnalyzers.at(side)->getChMaxSample(channel);
    chStatus(*module) =  m_dataAnalyzers.at(side)->getChStatus(channel);
    chPileupFrac(*module) =  m_dataAnalyzers.at(side)->getChPileupFrac(channel);
  }

  // write per-side decorations (in ZdcSums)
  SG::WriteDecorHandle<xAOD::ZdcModuleContainer, unsigned int> sideStatus(m_sideStatusKey);
  for (auto const& sum: moduleSumContainer) {
    if (sum->zdcSide() == RPDUtils::ZDCSumsGlobalZDCSide) {
      // skip global sum (it's like the side between sides)
      continue;
    }
    unsigned int const side = RPDUtils::ZDCSideToSideIndex(sum->zdcSide());
    sideStatus(*sum) = m_dataAnalyzers.at(side)->getSideStatus();
  }
}

StatusCode RPDAnalysisTool::recoZdcModules(xAOD::ZdcModuleContainer const& moduleContainer, xAOD::ZdcModuleContainer const& moduleSumContainer) {
    if (moduleContainer.empty()) {
      return StatusCode::SUCCESS; // if no modules, do nothing
    }
    
    SG::ReadHandle<xAOD::EventInfo> eventInfo(m_eventInfoKey);                     
  if (!eventInfo.isValid()) return StatusCode::FAILURE;
  
  bool rpdErr = eventInfo->isEventFlagBitSet(xAOD::EventInfo::ForwardDet, ZdcEventInfo::RPDDECODINGERROR );
  if (rpdErr)
    {
      ATH_MSG_WARNING("RPD decoding error found!");
      return StatusCode::SUCCESS;
    }
  
  reset();
  readAOD(moduleContainer);
  analyze();
  writeAOD(moduleContainer, moduleSumContainer);

  return StatusCode::SUCCESS;
}

StatusCode RPDAnalysisTool::reprocessZdc() {
  if (!m_initialized) {
    ATH_MSG_WARNING("Tool not initialized!");
    return StatusCode::FAILURE;
  }
  xAOD::ZdcModuleContainer const* ZDCModules = nullptr;
  ATH_CHECK(evtStore()->retrieve(ZDCModules, m_ZDCModuleContainerName));
  xAOD::ZdcModuleContainer const* ZDCSums = nullptr;
  ATH_CHECK(evtStore()->retrieve(ZDCSums, m_ZDCSumContainerName));
  return recoZdcModules(*ZDCModules, *ZDCSums);
}

} // namespace ZDC
