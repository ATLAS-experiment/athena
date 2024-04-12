/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ZDCANALYSIS_ZDCLEDANALYSISTOOL_H
#define ZDCANALYSIS_ZDCLEDANALYSISTOOL_H

#include "AsgTools/AsgTool.h"
#include "AsgTools/PropertyWrapper.h"
#include "AsgDataHandles/ReadHandleKey.h"
#include "AsgDataHandles/ReadDecorHandleKey.h"
#include "AsgDataHandles/WriteDecorHandleKey.h"
#include "CxxUtils/checker_macros.h"

#include "xAODForward/ZdcModuleContainer.h"
#include "xAODTrigL1Calo/TriggerTowerContainer.h"
#include "xAODEventInfo/EventInfo.h"

#include "ZdcUtils/ZdcEventInfo.h"
#include "ZdcAnalysis/IZdcAnalysisTool.h"
#include "ZdcAnalysis/ZDCMsg.h"

namespace ZDC
{

class ZDCLEDModuleResults
{
  unsigned int m_presampleADC;
  int m_ADCsum;
  int m_maxADC;
  unsigned int m_maxSample;
  float m_avgTime;

public:
  ZDCLEDModuleResults(unsigned int presampleADC, unsigned int ADCsum, unsigned int maxADC, unsigned int maxSample, float avgTime) :
    m_presampleADC(presampleADC),
    m_ADCsum(ADCsum),
    m_maxADC(maxADC),
    m_maxSample(maxSample),
    m_avgTime(avgTime)
    {}

  ZDCLEDModuleResults() :
    m_presampleADC(0),
    m_ADCsum(0),
    m_maxADC(0),
    m_maxSample(0),
    m_avgTime(0)
    {}

  unsigned int getPresampleADC() const {return m_presampleADC;}
  int getADCSum() const {return m_ADCsum;}
  int getMaxADC() const {return m_maxADC;}
  unsigned int getMaxSample() const {return m_maxSample;}
  float getAvgTime() const {return m_avgTime;}
    
};
  
class ATLAS_NOT_THREAD_SAFE ZdcLEDAnalysisTool : public virtual IZdcAnalysisTool, public asg::AsgTool
{

  ASG_TOOL_CLASS(ZdcLEDAnalysisTool, ZDC::IZdcAnalysisTool)

public:
  ZdcLEDAnalysisTool(const std::string& name);
  virtual ~ZdcLEDAnalysisTool() override;

  //interface from AsgTool
  StatusCode initialize() override;

  StatusCode recoZdcModules(const xAOD::ZdcModuleContainer& moduleContainer, const xAOD::ZdcModuleContainer& moduleSumContainer) override;
  StatusCode reprocessZdc() override;

  ZDCMsg::MessageFunctionPtr MakeMessageFunction()
  {
    std::function<bool(int, std::string)> msgFunction = [this](int messageZdcLevel, const std::string& message)-> bool
    {
      MSG::Level messageAthenaLevel = static_cast<MSG::Level>(messageZdcLevel);
      bool passesStreamOutputLevel = messageAthenaLevel >= this->msg().level();
      if (passesStreamOutputLevel) {
        this->msg(messageAthenaLevel) << message << endmsg;
      }
      return passesStreamOutputLevel;
    };

    return ZDCMsg::MessageFunctionPtr(new ZDCMsg::MessageFunction(msgFunction));
  }

private:

  // Provide methods
  //
  void initialize_ppPbPb2023();
  void initialize_ppALFA2023();
  void initialize_zdcStandalone();
  
  ZDCLEDModuleResults processZDCModule(const xAOD::ZdcModule& module);
  ZDCLEDModuleResults processRPDModule(const xAOD::ZdcModule& module);
  ZDCLEDModuleResults processModuleData(const std::vector<unsigned short>& data,
					unsigned int startSample, unsigned int endSample, float gainScale);
    
  bool m_init{false};
  
  // Job properties
  //
  std::string m_name;
  Gaudi::Property<std::string> m_configuration{this, "Configuration", "ppPbPb2023", "Which config files to use"};
  bool m_writeAux{false};

  Gaudi::Property<std::string> m_auxSuffix{this, "AuxSuffix", "", "Append this tag onto end of AuxData"};

  // Configuration settings based on mConfiguration
  //
  Gaudi::Property<bool> m_doRPD{this, "doRPD", true, "Process RPD Data?"};
  Gaudi::Property<bool> m_doZDC{this, "doZDC", true, "Process ZDC Data?"};

  // Configuration for LED identification
  //
  std::vector<unsigned int> m_LEDCalreqIdx;
  std::vector<unsigned int> m_LEDBCID;
  
  const std::vector<std::string> m_LEDNames = {"Blue1", "Green", "Blue2"};
  const std::vector<std::string> m_calreqNames = {"CalReq1", "CalReq2", "CalReq3"};

  unsigned int m_HGADCOverflow{4095};
  unsigned int m_numSamples{24};
  unsigned int m_preSample{0};
  float m_deltaTSample{3.125};
  unsigned int m_sampleAnaStartZDC{0};
  unsigned int m_sampleAnaEndZDC{23};
  unsigned int m_sampleAnaStartRPD{0};
  unsigned int m_sampleAnaEndRPD{23};

  float m_ZdcLowGainScale{10};

  // Module container names
  //
  Gaudi::Property<std::string> m_zdcModuleContainerName {this, "ZdcModuleContainerName", "ZdcModules", "Location of ZDC processed data"};
  const xAOD::ZdcModuleContainer* m_zdcModules {nullptr};
  Gaudi::Property<std::string> m_zdcSumContainerName {this, "ZdcSumContainerName", "ZdcSums", "Location of ZDC processed sums"};
  const xAOD::ZdcModuleContainer* m_zdcSums {nullptr};

  // Storegate keys
  //
  SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey {
    this, "EventInfoKey", "EventInfo", "Location of the event info."};

  SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_eventTypeKey {
    this, "ZdcEventTypeKey", "", "ZDC Event type"};

  SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_robBCIDKey {
    this, "ROBBCIDKey", "", "BCID from LUCROD ROB headers"};

  SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_DAQModeKey {
    this, "ZdcDAQModeKey", "", "ZDC DAQ mode"};

  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_ZdcLEDType{this, "ZdcLEDType", "", "ZDC LED Type (0-Blue1, 1-Green, 2-Blue2}"};
  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_ZdcLEDPresampleADC{this, "ZdcLEDPresampleADC", "", "ZDC LED presample"};
  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_ZdcLEDADCSum{this, "ZdcLEDADCSum", "", "ZDC LED pulse FADC sum"};
  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_ZdcLEDMaxADC{this, "ZdcLEDMaxADC", "", "ZDC LED pulse max FADC value"};
  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_ZdcLEDMaxSample{this, "ZdcLEDMaxSample", "", "ZDC LED max FADC sample"};
  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_ZdcLEDAvgTime{this, "ZdcLEDAvgTime", "", "ZDC LED average time"};
 
};

} // namespace ZDC

#endif




