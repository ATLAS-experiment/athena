/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ZDCANALYSIS_RPDANALYSISTOOL_H
#define ZDCANALYSIS_RPDANALYSISTOOL_H

#include "CxxUtils/checker_macros.h"
#include "AsgTools/AsgTool.h"
#include "AsgTools/AsgToolMacros.h"
#include "AsgTools/PropertyWrapper.h"

#include "ZdcAnalysis/IZdcAnalysisTool.h"
#include "ZdcAnalysis/RPDDataAnalyzer.h"
#include "xAODEventInfo/EventInfo.h"

namespace ZDC {

class ATLAS_NOT_THREAD_SAFE RPDAnalysisTool : public virtual IZdcAnalysisTool, public asg::AsgTool {
  ASG_TOOL_CLASS(RPDAnalysisTool, ZDC::IZdcAnalysisTool)
 public:
  explicit RPDAnalysisTool(std::string const& name);
  ~RPDAnalysisTool() override = default;

  // this tool should never be copied or moved
  RPDAnalysisTool(RPDAnalysisTool const&) = delete;
  RPDAnalysisTool& operator=(RPDAnalysisTool const&) = delete;
  RPDAnalysisTool(RPDAnalysisTool &&) = delete;
  RPDAnalysisTool& operator=(RPDAnalysisTool &&) = delete;

  StatusCode initialize() override;

 private:
  StatusCode recoZdcModules(xAOD::ZdcModuleContainer const& moduleContainer, xAOD::ZdcModuleContainer const& moduleSumContainer) override;
  StatusCode reprocessZdc() override;

  // reconstruction steps
  void reset();
  void readAOD(xAOD::ZdcModuleContainer const& moduleContainer);
  void analyze();
  void writeAOD(xAOD::ZdcModuleContainer const& moduleContainer, xAOD::ZdcModuleContainer const& moduleSumContainer) const;

  StatusCode initializeWriteKey(std::string const& containerName, SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> & writeHandleKey, std::string const& key);
  ZDCMsg::MessageFunctionPtr MakeMessageFunction();

  bool m_initialized = false;
  // tool properties to be configured in Python
  Gaudi::Property<std::string> m_configuration{this, "Configuration", "default"};
  Gaudi::Property<bool> m_writeAux{this, "WriteAux", true};
  Gaudi::Property<std::string> m_auxSuffix{this, "AuxSuffix", ""};
  Gaudi::Property<std::string> m_ZDCModuleContainerName{this, "ZdcModuleContainerName", "ZdcModules", "Location of ZDC processed data"};
  Gaudi::Property<std::string> m_ZDCSumContainerName{this, "ZdcSumContainerName", "ZdcSums", "Location of ZDC processed sums"};
  // these are optional and null by default; if their values are set, they will overwrite the bulk configuration from config string
  Gaudi::Property<RPDUtils::OptionalToolProperty<unsigned int>> m_forceNSamples{this, "NSamples", {}, "Total number of FADC samples in readout window"};
  Gaudi::Property<RPDUtils::OptionalToolProperty<unsigned int>> m_forceNBaselineSamples{this, "NBaselineSamples", {}, "Number of baseline samples; the sample equal to this number is the start of signal region"};
  Gaudi::Property<RPDUtils::OptionalToolProperty<unsigned int>> m_forceEndSignalSample{this, "EndSignalSample", {}, "Samples before (not including) this sample are the signal region; 0 or Nsamples goes to end of window"};
  Gaudi::Property<RPDUtils::OptionalToolProperty<float>> m_forcePulse2ndDerivThresh{this, "Pulse2ndDerivThresh", {}, "Second differences less than or equal to this number indicate a pulse"};
  Gaudi::Property<RPDUtils::OptionalToolProperty<float>> m_forcePostPulseFracThresh{this, "PostPulseFracThresh", {}, "If there is a good pulse and post-pulse and size of post-pulse as a fraction of good pulse is less than or equal to this number, ignore post-pulse"};
  Gaudi::Property<RPDUtils::OptionalToolProperty<unsigned int>> m_forceGoodPulseSampleStart{this, "GoodPulseSampleStart", {}, "Pulses before this sample are considered pre-pulses"};
  Gaudi::Property<RPDUtils::OptionalToolProperty<unsigned int>> m_forceGoodPulseSampleStop{this, "GoodPulseSampleStop", {}, "Pulses after this sample are considered post-pulses"};
  Gaudi::Property<RPDUtils::OptionalToolProperty<float>> m_forceNominalBaseline{this, "NominalBaseline", {}, "The global nominal baseline; used when pileup is detected"};
  Gaudi::Property<RPDUtils::OptionalToolProperty<float>> m_forcePileupBaselineSumThresh{this, "PileupBaselineSumThresh", {}, "Baseline sum (after subtracting nominal baseline) less than this number indicates there is NO pileup"};
  Gaudi::Property<RPDUtils::OptionalToolProperty<float>> m_forcePileupBaselineStdDevThresh{this, "PileupBaselineStdDevThresh", {}, "Baseline standard deviations less than this number indicate there is NO pileup"};
  Gaudi::Property<RPDUtils::OptionalToolProperty<unsigned int>> m_forceNNegativesAllowed{this, "NNegativesAllowed", {}, "Maximum number of negative ADC values after baseline and pileup subtraction allowed in signal range"};
  Gaudi::Property<RPDUtils::OptionalToolProperty<unsigned int>> m_forceADCOverflow{this, "ADCOverflow", {}, "ADC values greater than or equal to this number are considered overflow"};
  Gaudi::Property<RPDUtils::OptionalToolProperty<std::vector<float>>> m_forceSideCCalibFactors{this, "SideCCalibFactors", {}, "Multiplicative calibration factors to apply to RPD output, e.g., sum/max ADC, per channel on side C"};
  Gaudi::Property<RPDUtils::OptionalToolProperty<std::vector<float>>> m_forceSideACalibFactors{this, "SideACalibFactors", {}, "Multiplicative calibration factors to apply to RPD output, e.g., sum/max ADC, per channel on side A"};
  std::array<RPDUtils::OptionalToolProperty<std::vector<float>>, 2> m_forceOutputCalibFactors; // multiplicative calibration factors to apply to RPD output, e.g., sum/max ADC, per channel, per side; filled from the properties above in initialize()

  // one for each side
  std::array<std::unique_ptr<RPDDataAnalyzer>, 2> m_dataAnalyzers;

  // read/write handles
  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_chBaselineKey {this, "RPDChannelBaseline", "", "RPD channel baseline"};
  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_chPileupExpFitParamsKey {this, "RPDChannelPileupExpFitParams", "", "RPD channel pileup exponential fit parameters: exp( [0] + [1]*sample )"};
  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_chPileupStretchedExpFitParamsKey {this, "RPDChannelPileupStretchedExpFitParams", "", "RPD channel pileup stretched exponential fit parameters: exp( [0] + [1]*(sample + 4)**0.5 + [2]*(sample + 4)**-0.5 )"};
  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_chPileupExpFitParamErrsKey {this, "RPDChannelPileupExpFitParamErrs", "", "RPD channel pileup exponential fit parameter errors"};
  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_chPileupStretchedExpFitParamErrsKey {this, "RPDChannelPileupStretchedExpFitParamErrs", "", "RPD channel pileup stretched exponential fit parameter errors"};
  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_chPileupExpFitMSEKey {this, "RPDChannelPileupExpFitMSE", "", "RPD Channel pileup exponential fit mean squared error in baseline samples"};
  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_chPileupStretchedExpFitMSEKey {this, "RPDChannelPileupStretchedExpFitMSE", "", "RPD channel pileup stretched exponential fit mean squared error in baseline samples"};
  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_chAmplitudeKey {this, "RPDChannelAmplitude", "", "RPD channel sum ADC (baseline and pileup subtracted)"};
  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_chAmplitudeCalibKey {this, "RPDChannelAmplitudeCalib", "", "RPD channel sum ADC (baseline and pileup subtracted) with output calibration factors applied"};
  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_chMaxADCKey {this, "RPDChannelMaxADC", "", "RPD channel max ADC (baseline and pileup subtracted)"};
  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_chMaxADCCalibKey {this, "RPDChannelMaxADCCalib", "", "RPD channel max ADC (baseline and pileup subtracted) with output calibration factors applied"};
  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_chMaxSampleKey {this, "RPDChannelMaxSample", "", "RPD channel max sample"};
  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_chStatusKey {this, "RPDChannelStatus", "", "RPD channel status"};
  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_chPileupFracKey {this, "RPDChannelPileupFrac", "", "RPD channel pileup as fraction of total (nominal baseline-subtracted) sum ADC"};

  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_sideStatusKey {this, "ZdcSumRPDStatus", "", "RPD side level status"};

  SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey {this, "EventInfoKey", "EventInfo", "Location of the event info."};
};

} // namespace ZDC

#endif
