/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ZDCANALYSIS_ZDCANALYSISTOOL_H
#define ZDCANALYSIS_ZDCANALYSISTOOL_H

#include "AsgTools/AsgTool.h"
#include "AsgDataHandles/ReadHandleKey.h"
#include "AsgDataHandles/WriteDecorHandleKey.h"

#include "xAODForward/ZdcModuleContainer.h"

#include "ZdcAnalysis/ZDCDataAnalyzer.h"
#include "ZdcAnalysis/ZDCTriggerEfficiency.h"
#include "ZdcAnalysis/IZdcAnalysisTool.h"
#include "ZdcAnalysis/ZDCMsg.h"

#include "TF1.h"

#include "xAODEventInfo/EventInfo.h"
#include "CxxUtils/checker_macros.h"


namespace ZDC
{

class ATLAS_NOT_THREAD_SAFE ZdcAnalysisTool : public virtual IZdcAnalysisTool, public asg::AsgTool
{

  ASG_TOOL_CLASS(ZdcAnalysisTool, ZDC::IZdcAnalysisTool)

public:
  ZdcAnalysisTool(const std::string& name);
  virtual ~ZdcAnalysisTool() override;
  void initializeDecorations();

  //interface from AsgTool
  StatusCode initialize() override;
  void initialize80MHz();
  void initialize40MHz();
  void initializeTriggerEffs(unsigned int runNumber);

  StatusCode recoZdcModules(const xAOD::ZdcModuleContainer& moduleContainer, const xAOD::ZdcModuleContainer& moduleSumContainer) override;
  StatusCode reprocessZdc() override;

  // methods for processing, used for decoration
  static bool sigprocMaxFinder(const std::vector<unsigned short>& adc, float deltaT, float& amp, float& time, float& qual);
  bool sigprocSincInterp(const std::vector<unsigned short>& adc, float deltaT, float& amp, float& time, float& qual);

  void setEnergyCalibrations(unsigned int runNumber);
  void setTimeCalibrations(unsigned int runNumber);
  void setFADCCorrections(unsigned int runNumber = 0);
  
  float getModuleSum(int side);

  float getCalibModuleSum(int side);
  float getCalibModuleSumErr(int side);

  float getUncalibModuleSum(int side);
  float getUncalibModuleSumErr(int side);

  float getAverageTime(int side);
  bool  sideFailed(int side);
  unsigned int getModuleMask();

  double getTriggerEfficiency(int side);
  double getTriggerEfficiencyUncertainty(int side);

  const ZDCDataAnalyzer* getDataAnalyzer() {return m_zdcDataAnalyzer.get();}

  static void SetDebugLevel(int debugLevel = 0)
  {
    s_debugLevel = debugLevel;
  }

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

  void Dump_setting() {
    if (s_debugLevel > 2) {
      ATH_MSG_INFO("========================================================================================================================");
      for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 4; j++) {
          ATH_MSG_INFO("-------------------------------------------------------------------------------------------------------------------");
          ATH_MSG_INFO("Side: " << i << ", Module: " << j);
          m_zdcDataAnalyzer->GetPulseAnalyzer(i, j)->dumpSetting();
        }
      }
      ATH_MSG_INFO("========================================================================================================================");
    }
  }

private:
  // Private methods
  //
  std::unique_ptr<ZDCDataAnalyzer> initializeDefault();
  std::unique_ptr<ZDCDataAnalyzer> initializePbPb2015G4();
  std::unique_ptr<ZDCDataAnalyzer> initializepPb2016();
  std::unique_ptr<ZDCDataAnalyzer> initializePbPb2018();
  std::unique_ptr<ZDCDataAnalyzer> initializeLHCf2022();
  std::unique_ptr<ZDCDataAnalyzer> initializepp2023();
  std::unique_ptr<ZDCDataAnalyzer> initializePbPb2023();
  std::unique_ptr<ZDCDataAnalyzer> initializepp2024();
  std::unique_ptr<ZDCDataAnalyzer> initializePbPb2024();
  std::unique_ptr<ZDCDataAnalyzer> initializeInjectorpp2024();
  std::unique_ptr<ZDCDataAnalyzer> initializeInjectorPbPb2024();
  std::unique_ptr<ZDCDataAnalyzer> initializeMonteCarloPbPb2023();

  StatusCode configureNewRun(unsigned int runNumber);

  // Data members
  //
  std::string m_name;
  bool m_init;
  std::string m_configuration;
  std::string m_zdcAnalysisConfigPath;
  std::string m_zdcEnergyCalibFileName;
  std::string m_zdcTimeCalibFileName;
  std::string m_zdcTriggerEffParamsFileName;

  bool m_writeAux;
  std::string m_auxSuffix;

  bool m_eventReady;
  unsigned int m_runNumber;
  unsigned int m_lumiBlock;

  // internal functions
  std::unique_ptr<TF1> m_tf1SincInterp;

  SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey {
    this, "EventInfoKey", "EventInfo",
      "Location of the event info."};

  std::string m_zdcModuleContainerName;
  const xAOD::ZdcModuleContainer* m_zdcModules {nullptr};
  std::string m_zdcSumContainerName;
  const xAOD::ZdcModuleContainer* m_zdcSums {nullptr};
  bool m_flipEMDelay;
  unsigned int m_lowGainMode;
  bool m_combineDelay{false};
  bool m_doCalib{false};
  bool m_doTrigEff{false};
  bool m_doTimeCalib{false};
  bool m_doFADCCorr{false};
  bool m_doNonLinCorr{false};
  bool m_doFADCCorrPerSample{false};
  int m_forceCalibRun;
  int m_forceCalibLB;
  std::string m_calibVersion;
  
  //  Parameters that control the pulse fitting analysis
  //
  unsigned int m_numSample;
  float m_deltaTSample;
  unsigned int m_presample;
  unsigned int m_peakSample;
  float m_Peak2ndDerivThresh;
  float m_t0;
  float m_delayDeltaT;
  float m_tau1;
  float m_tau2;
  bool m_fixTau1;
  bool m_fixTau2;
  float m_deltaTCut;
  float m_ChisqRatioCut;
  
  int m_LHCRun;

  // The objects that carry out the analysis
  //
  std::shared_ptr<ZDCDataAnalyzer> m_zdcDataAnalyzer;
  std::shared_ptr<ZDCDataAnalyzer> m_zdcDataAnalyzer_40MHz;
  std::shared_ptr<ZDCDataAnalyzer> m_zdcDataAnalyzer_80MHz;
  
  ZDCDataAnalyzer::ZDCModuleIntArray m_peak2ndDerivMinSamples{};
  ZDCDataAnalyzer::ZDCModuleFloatArray m_peak2ndDerivMinThresholdsHG{};
  ZDCDataAnalyzer::ZDCModuleFloatArray m_peak2ndDerivMinThresholdsLG{};


  std::shared_ptr<ZDCTriggerEfficiency> m_zdcTriggerEfficiency;

  static std::atomic<int> s_debugLevel;

  // decoration list for ZDC modules
  // ZDC
  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_zdcModuleAmplitude{this, "ZdcModuleAmplitude", "", "ZDC module amplitude"};
  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_zdcModuleCalibEnergy{this, "ZdcModuleCalibEnergy", "", "ZDC module calibrated energy"};
  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_zdcModuleCalibTime{this, "ZdcModuleCalibTime", "", "ZDC module calibrated time"};
  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_zdcModuleStatus{this, "ZdcModuleStatus", "", "ZDC module fit status"};
  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_zdcModuleTime{this, "ZdcModuleTime", "", "ZDC module time"};
  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_zdcModuleChisq{this, "ZdcModuleChisq", "", "ZDC module fit chisq"};
  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_zdcModuleAmpNoNonLin{this, "ZdcModuleAmpNoNonLin", "", "ZDC module amplitude with gain factor applied but no nonlinear correction"};
  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_zdcModuleFitAmp{this, "ZdcModuleFitAmp", "", "ZDC module fit amp"};
  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_zdcModuleFitAmpError{this, "ZdcModuleFitAmpError", "", "ZDC module fit amp error"};  
  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_zdcModuleFitT0{this, "ZdcModuleFitT0", "", "ZDC module fit t0"};
  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_zdcModuleBkgdMaxFraction{this, "ZdcModuleBkgdMaxFraction", "", "ZDC module background max fraction"};
  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_zdcModulePreSampleAmp{this, "ZdcModulePreSampleAmp", "", "ZDC module presample amplitude"};
  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_zdcModulePresample{this, "ZdcModulePresample", "", "ZDC module presample"};
  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_zdcModuleMinDeriv2nd{this, "ZdcModuleMinDeriv2nd", "", "ZDC module min 2nd derivative"};
  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_zdcModuleMaxADC{this, "ZdcModuleMaxADC", "", "ZDC module max ADC, minus pre-sample"};
  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_zdcModuleMaxADCHG{this, "ZdcModuleMaxADCHG", "", "ZDC module HG max ADC, unsubtracted"};
  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_zdcModuleMaxADCLG{this, "ZdcModuleMaxADCLG", "", "ZDC module LG max ADC, unsubtracted"};

  // LG refit data
  // 
  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_zdcModuleFitAmpLGRefit{this, "ZdcModuleFitAmpLGRefit", "", "ZDC module fit amp LG refit, with no gain factor or nonlinear correction applied"};
  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_zdcModuleAmpLGRefit{this, "ZdcModuleAmpLGRefit", "", "ZDC module amp LG refit, with gain factor applied but no nonlinear correction"};
  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_zdcModuleAmpCorrLGRefit{this, "ZdcModuleAmpCorrLGRefit", "", "ZDC module amp LG refit, with both gain factor and nonlinear correction applied"};
  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_zdcModuleT0LGRefit{this, "ZdcModuleT0LGRefit", "", "ZDC module fit t0 LG refit"};
  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_zdcModuleT0SubLGRefit{this, "ZdcModuleT0SubLGRefit", "", "ZDC module subtracted t0 LG refit"};
  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_zdcModuleChisqLGRefit{this, "ZdcModuleChisqLGRefit", "", "ZDC module LG refit chi square"};

  // decoration list for sums
  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_zdcSumUncalibSum{this, "ZdcSumUncalibSum", "", "ZDC side uncalibrated sum"};
  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_zdcSumUncalibSumErr{this, "ZdcSumUncalibSumErr", "", "ZDC side uncalibrated sum error"};
  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_zdcSumCalibEnergy{this, "ZdcSumCalibEnergy", "", "ZDC side calibrated energy"};
  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_zdcSumCalibEnergyErr{this, "ZdcSumCalibEnergyErr", "", "ZDC side calibrated energy error"};
  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_zdcSumFinalEnergy{this, "ZdcSumFinalEnergy", "", "ZDC side final energy"};
  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_zdcSumAverageTime{this, "ZdcSumAverageTime", "", "ZDC side average time"};
  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_zdcSumStatus{this, "ZdcSumStatus", "", "ZDC side status"};
  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_zdcSumModuleMask{this, "ZdcSumModuleMask", "", "ZDC side module mask"};

};

} // namespace ZDC

#endif



