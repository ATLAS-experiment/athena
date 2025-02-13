/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include <AsgDataHandles/WriteDecorHandle.h>

#include "ZdcAnalysis/RPDAnalysisTool.h"
#include "ZdcUtils/RPDUtils.h"
#include "xAODEventInfo/EventInfo.h"
#include "ZdcUtils/ZdcEventInfo.h"

namespace ZDC {

RPDAnalysisTool::RPDAnalysisTool(std::string const& name) : asg::AsgTool(name) {
  declareProperty("ZdcModuleContainerName", m_ZDCModuleContainerName = "ZdcModules", "Location of ZDC processed data");
  declareProperty("ZdcSumContainerName", m_ZDCSumContainerName = "ZdcSums", "Location of ZDC processed sums");
  declareProperty("Configuration", m_configuration = "default");
  declareProperty("WriteAux", m_writeAux = true);
  declareProperty("AuxSuffix", m_auxSuffix = "");

  declareProperty("NSamples", m_forceNSamples, "Total number of FADC samples in readout window");
  declareProperty("NBaselineSamples", m_forceNBaselineSamples, "Number of baseline samples; the sample equal to this number is the start of signal region");
  declareProperty("EndSignalSample", m_forceEndSignalSample, "Samples before (not including) this sample are the signal region; 0 or Nsamples goes to end of window");
  declareProperty("Pulse2ndDerivThresh", m_forcePulse2ndDerivThresh, "Second differences less than or equal to this number indicate a pulse");
  declareProperty("PostPulseFracThresh", m_forcePostPulseFracThresh, "If there is a good pulse and post-pulse and size of post-pulse as a fraction of good pulse is less than or equal to this number, ignore post-pulse");
  declareProperty("GoodPulseSampleStart", m_forceGoodPulseSampleStart, "Pulses before this sample are considered pre-pulses");
  declareProperty("GoodPulseSampleStop", m_forceGoodPulseSampleStop, "Pulses after this sample are considered post-pulses");
  declareProperty("NominalBaseline", m_forceNominalBaseline, "The global nominal baseline; used when pileup is detected");
  declareProperty("PileupBaselineSumThresh", m_forcePileupBaselineSumThresh, "Baseline sum (after subtracting nominal baseline) less than this number indicates there is NO pileup");
  declareProperty("PileupBaselineStdDevThresh", m_forcePileupBaselineStdDevThresh, "Baseline standard deviations less than this number indicate there is NO pileup");
  declareProperty("NNegativesAllowed", m_forceNNegativesAllowed, "Maximum number of negative ADC values after baseline and pileup subtraction allowed in signal range");
  declareProperty("ADCOverflow", m_forceADCOverflow, "ADC values greater than or equal to this number are considered overflow");
  declareProperty("SideCCalibFactors", m_forceOutputCalibFactors.at(RPDUtils::sideC), "Multiplicative calibration factors to apply to RPD output, e.g., sum/max ADC, per channel on side C");
  declareProperty("SideACalibFactors", m_forceOutputCalibFactors.at(RPDUtils::sideA), "Multiplicative calibration factors to apply to RPD output, e.g., sum/max ADC, per channel on side A");
}

StatusCode RPDAnalysisTool::initializeKey(std::string const& containerName, SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> & writeHandleKey, std::string const& key) {
  writeHandleKey = containerName + key + m_auxSuffix;
  return writeHandleKey.initialize();
}

StatusCode RPDAnalysisTool::initialize() {
  RPDConfig finalConfig {};
  std::array<std::vector<float>, 2> finalOutputCalibFactors;
  // first initialize reconstruction parameters
  finalConfig.nSamples = 24;
  finalConfig.nBaselineSamples = 7;
  finalConfig.endSignalSample = 23;
  finalConfig.pulse2ndDerivThresh = -18;
  finalConfig.postPulseFracThresh = 0.15;
  finalConfig.goodPulseSampleStart = 8;
  finalConfig.goodPulseSampleStop = 10;
  finalConfig.nominalBaseline = 100;
  finalConfig.pileupBaselineSumThresh = 53;
  finalConfig.pileupBaselineStdDevThresh = 2;
  finalConfig.nNegativesAllowed = 2;
  finalConfig.ADCOverflow = 4095;
  finalOutputCalibFactors.at(RPDUtils::sideC) = std::vector<float>(16, 1.0);
  finalOutputCalibFactors.at(RPDUtils::sideA) = std::vector<float>(16, 1.0);

  // then overwrite individual parameters from configuration if any were provided
  if (m_forceNSamples.has_value()) {
    finalConfig.nSamples = m_forceNSamples.value();
  }
  if (m_forceNBaselineSamples.has_value()) {
    finalConfig.nBaselineSamples = m_forceNBaselineSamples.value();
  }
  if (m_forceEndSignalSample.has_value()) {
    finalConfig.endSignalSample = m_forceEndSignalSample.value();
  }
  if (m_forcePulse2ndDerivThresh.has_value()) {
    finalConfig.pulse2ndDerivThresh = m_forcePulse2ndDerivThresh.value();
  }
  if (m_forcePostPulseFracThresh.has_value()) {
    finalConfig.postPulseFracThresh = m_forcePostPulseFracThresh.value();
  }
  if (m_forceGoodPulseSampleStart.has_value()) {
    finalConfig.goodPulseSampleStart = m_forceGoodPulseSampleStart.value();
  }
  if (m_forceGoodPulseSampleStop.has_value()) {
    finalConfig.goodPulseSampleStop = m_forceGoodPulseSampleStop.value();
  }
  if (m_forceNominalBaseline.has_value()) {
    finalConfig.nominalBaseline = m_forceNominalBaseline.value();
  }
  if (m_forcePileupBaselineSumThresh.has_value()) {
    finalConfig.pileupBaselineSumThresh = m_forcePileupBaselineSumThresh.value();
  }
  if (m_forcePileupBaselineStdDevThresh.has_value()) {
    finalConfig.pileupBaselineStdDevThresh = m_forcePileupBaselineStdDevThresh.value();
  }
  if (m_forceNNegativesAllowed.has_value()) {
    finalConfig.nNegativesAllowed = m_forceNNegativesAllowed.value();
  }
  if (m_forceADCOverflow.has_value()) {
    finalConfig.ADCOverflow = m_forceADCOverflow.value();
  }
  for (auto const side : RPDUtils::sides) {
    if (m_forceOutputCalibFactors.at(side).has_value()) {
      finalOutputCalibFactors.at(side) = m_forceOutputCalibFactors.at(side).value();
    }
  }

  ATH_MSG_DEBUG("RPDAnalysisTool reconstruction parameters:");
  ATH_MSG_DEBUG("config = " << m_configuration);
  ATH_MSG_DEBUG("nSamples = " << finalConfig.nSamples);
  ATH_MSG_DEBUG("nBaselineSamples = " << finalConfig.nBaselineSamples);
  ATH_MSG_DEBUG("endSignalSample = " << finalConfig.endSignalSample);
  ATH_MSG_DEBUG("pulse2ndDerivThresh = " << finalConfig.pulse2ndDerivThresh);
  ATH_MSG_DEBUG("postPulseFracThresh = " << finalConfig.postPulseFracThresh);
  ATH_MSG_DEBUG("goodPulseSampleStart = " << finalConfig.goodPulseSampleStart);
  ATH_MSG_DEBUG("goodPulseSampleStop = " << finalConfig.goodPulseSampleStop);
  ATH_MSG_DEBUG("nominalBaseline = " << finalConfig.nominalBaseline);
  ATH_MSG_DEBUG("pileupBaselineSumThresh = " << finalConfig.pileupBaselineSumThresh);
  ATH_MSG_DEBUG("pileupBaselineStdDevThresh = " << finalConfig.pileupBaselineStdDevThresh);
  ATH_MSG_DEBUG("nNegativesAllowed = " << finalConfig.nNegativesAllowed);
  ATH_MSG_DEBUG("ADCOverflow = " << finalConfig.ADCOverflow);
  ATH_MSG_DEBUG("sideCCalibFactors = " << RPDUtils::vecToString(finalOutputCalibFactors.at(RPDUtils::sideC)));
  ATH_MSG_DEBUG("sideACalibFactors = " << RPDUtils::vecToString(finalOutputCalibFactors.at(RPDUtils::sideA)));

  m_dataAnalyzers.at(RPDUtils::sideC) = std::make_unique<RPDDataAnalyzer>(MakeMessageFunction(), "rpdC", finalConfig, finalOutputCalibFactors.at(RPDUtils::sideC));
  m_dataAnalyzers.at(RPDUtils::sideA) = std::make_unique<RPDDataAnalyzer>(MakeMessageFunction(), "rpdA", finalConfig, finalOutputCalibFactors.at(RPDUtils::sideA));

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
  for (auto const module : moduleContainer) {
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
    auto const& waveform = module->auxdataConst<std::vector<uint16_t>>("g0data");
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
  for (auto const module : moduleContainer) {
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
  for (auto const sum: moduleSumContainer) {
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
  if (!eventInfo.isValid()) {
    return StatusCode::FAILURE;
  }

  if (eventInfo->isEventFlagBitSet(xAOD::EventInfo::ForwardDet, ZdcEventInfo::RPDDECODINGERROR)) {
    ATH_MSG_WARNING("RPD decoding error found - abandoning RPD reco!");
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
