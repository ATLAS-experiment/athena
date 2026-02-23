/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ZDCANALYSIS_LISANALYSISTOOL_H
#define ZDCANALYSIS_LISANALYSISTOOL_H

#include "CxxUtils/checker_macros.h"
#include "AsgTools/AsgTool.h"
#include "AsgTools/AsgToolMacros.h"
#include "AsgTools/PropertyWrapper.h"
#include "AsgDataHandles/ReadHandleKey.h"
#include "AsgDataHandles/ReadDecorHandleKey.h"
#include "AsgDataHandles/WriteDecorHandleKey.h"

#include "ZdcAnalysis/IZdcAnalysisTool.h"
#include "xAODEventInfo/EventInfo.h"
#include "xAODForward/ZdcModuleContainer.h"

namespace ZDC {

// Results class to hold processed LIS waveform quantities
class LISModuleResults {
  unsigned int m_presampleADC{};
  int m_ADCsum{};
  int m_maxADC{};
  unsigned int m_maxSample{};
  float m_avgTime{};

public:
  LISModuleResults(float presampleADC, int ADCsum, int maxADC, unsigned int maxSample, float avgTime) :
    m_presampleADC(presampleADC),
    m_ADCsum(ADCsum),
    m_maxADC(maxADC),
    m_maxSample(maxSample),
    m_avgTime(avgTime)
  {}

  LISModuleResults() = default;

  unsigned int getPresampleADC() const { return m_presampleADC; }
  int getADCSum() const { return m_ADCsum; }
  int getMaxADC() const { return m_maxADC; }
  unsigned int getMaxSample() const { return m_maxSample; }
  float getAvgTime() const { return m_avgTime; }
};

class ATLAS_NOT_THREAD_SAFE LISAnalysisTool : public virtual IZdcAnalysisTool, public asg::AsgTool {
  ASG_TOOL_CLASS(LISAnalysisTool, ZDC::IZdcAnalysisTool)

public:
  explicit LISAnalysisTool(std::string const& name);
  virtual ~LISAnalysisTool() override = default;

  StatusCode initialize() override;
  StatusCode recoZdcModules(xAOD::ZdcModuleContainer const& moduleContainer, xAOD::ZdcModuleContainer const& moduleSumContainer) override;
  StatusCode reprocessZdc() override;

private:
  // Configuration initialization methods
  void initialize_default();

  // Processing methods
  LISModuleResults processLISModule(const xAOD::ZdcModule& module);
  LISModuleResults processModuleData(int side, int channel, 
                                     const std::vector<unsigned short>& data,
                                     unsigned int startSample, unsigned int endSample);

  bool m_init{false};
  std::string m_name;

  // Job properties
  Gaudi::Property<std::string> m_configuration{this, "Configuration", "default", "Which config to use"};
  Gaudi::Property<bool> m_writeAux{this, "WriteAux", true, "Write auxiliary data"};
  Gaudi::Property<std::string> m_auxSuffix{this, "AuxSuffix", "", "Suffix for aux data names"};
  Gaudi::Property<unsigned int> m_nBaselineStart{this, "BaselineStart", 0, "Start index for baseline calculation"};
  Gaudi::Property<unsigned int> m_nBaselineEnd{this, "BaselineEnd", 5, "End index for baseline calculation"};

  std::vector<unsigned int> m_LEDCalreqIdx;
  std::vector<unsigned int> m_LEDBCID;
  
  const std::vector<std::string> m_LEDNames = {"Blue1", "Green", "Blue2"};
  const std::vector<std::string> m_calreqNames = {"CalReq1", "CalReq2", "CalReq3"};

  // Configuration settings
  unsigned int m_numSamples{24};
  unsigned int m_preSample{0};
  float m_deltaTSample{3.125};  // ns per sample
  unsigned int m_sampleAnaStart{5};
  unsigned int m_sampleAnaEnd{23};
  unsigned int m_nBaselineSamples{5}; // default to 5 samples for baseline calculation

  // Module container names
  Gaudi::Property<std::string> m_zdcModuleContainerName{this, "ZdcModuleContainerName", "ZdcModules", "Location of ZDC processed data"};
  Gaudi::Property<std::string> m_zdcSumContainerName{this, "ZdcSumContainerName", "ZdcSums", "Location of ZDC processed sums"};
  const xAOD::ZdcModuleContainer* m_zdcModules{nullptr};
  const xAOD::ZdcModuleContainer* m_zdcSums{nullptr};

  // Read handles
  SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey{this, "EventInfoKey", "EventInfo", "Location of the event info"};
  SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_eventTypeKey{this, "ZdcEventTypeKey", "", "ZDC Event type"};
  SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_robBCIDKey {this, "ROBBCIDKey", "", "BCID from LUCROD ROB headers"};
  SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_DAQModeKey{this, "ZdcDAQModeKey", "", "ZDC DAQ mode"};

  // Write decoration handles for LIS-specific outputs
  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_ZdcLEDType{this, "ZdcLEDType", "", "ZDC LED Type (0-Blue1, 1-Green, 2-Blue2}"};
  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_LISPresampleADC{this, "LISPresampleADC", "", "LIS presample ADC"};
  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_LISADCSum{this, "LISADCSum", "", "LIS pulse FADC sum"};
  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_LISMaxADC{this, "LISMaxADC", "", "LIS pulse max FADC value"};
  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_LISMaxSample{this, "LISMaxSample", "", "LIS max FADC sample"};
  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_LISAvgTime{this, "LISAvgTime", "", "LIS average time"};

  bool m_doFADCCorr{};
  double getAmplitudeCorrection(int iside, int imod, bool highGain, float fitAmp);
  void setFADCCorrections(unsigned int runNumber);


};

} // namespace ZDC

#endif // ZDCANALYSIS_LISANALYSISTOOL_H
