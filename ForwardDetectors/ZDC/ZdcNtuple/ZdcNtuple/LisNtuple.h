/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ZdcNtuple_LisNtuple_H
#define ZdcNtuple_LisNtuple_H

#include <AnaAlgorithm/AnaAlgorithm.h>
#include "AsgDataHandles/ReadHandleKey.h"
#include "xAODEventInfo/EventInfo.h"
#include "xAODForward/ZdcModuleContainer.h"
#include <string>
#include <ZdcConditions/ZdcInjPulserAmpMap.h>

class LisNtuple : public EL::AnaAlgorithm
{
public:

  unsigned int m_lastRunNumber{0};
  ZdcInjPulserAmpMap::Token m_injMapRunToken{};


  SG::ReadHandleKey<xAOD::ZdcModuleContainer> m_zdcModuleContainerName{this, "ZdcModuleContainerName", "ZdcModules", ""};
  SG::ReadHandleKey<xAOD::ZdcModuleContainer> m_zdcSumContainerName{this, "ZdcSumContainerName", "ZdcSums", ""};
  const xAOD::EventInfo *m_eventInfo{};
  int m_eventCounter{};

  // flags
  bool m_enableOutputTree{}; // enable output TTree
  bool m_lisInj{}; // LIS injected-pulse run
  bool m_lisLED{}; // LIS LED run
  std::string m_auxSuffix{}; // suffix for aux data names when reprocessing

  // output tree
  TTree *m_outputTree{};

  // inj map
  std::unique_ptr<ZdcInjPulserAmpMap> m_zdcInjPulserAmpMap;

  // evt info
  float t_vInj{};
  uint32_t t_runNumber{};
  uint32_t t_eventNumber{};
  uint32_t t_lumiBlock{};
  uint32_t t_bcid{};
  uint8_t t_bunchGroup{};
  uint32_t t_extendedLevel1ID{};
  uint32_t t_timeStamp{};
  uint32_t t_timeStampNSOffset{};
  float t_avgIntPerCrossing{};
  float t_actIntPerCrossing{};

  // LED type (for LED events)
  unsigned int t_LEDType{};

  // LIS module constants
  static constexpr int nLISChannels = 8;  // LIS channels per side
  static constexpr int nSamples = 24;

  static constexpr int LISTypeInd = 2;    // LIS type index
  static constexpr int LISModuleInd = 5;  // LIS module index
  static constexpr int infoSumInd = 0;    // side index for event-level info

  // LIS processed quantities
  float t_LISPresample[nLISChannels]{};
  int t_LISADCSum[nLISChannels]{};
  int t_LISMaxADC[nLISChannels]{};
  unsigned int t_LISMaxSample[nLISChannels]{};
  float t_LISAvgTime[nLISChannels]{};

  // LIS raw waveform data
  uint16_t t_LISRawdata[nLISChannels][nSamples]{};

  LisNtuple(const std::string &name, ISvcLocator *pSvcLocator);

  void processEventInfo();
  void processLisNtupleFromModules();

  virtual StatusCode initialize() override;
  virtual StatusCode execute() override;
  virtual StatusCode finalize() override;

  void processVInjInfo();
};

#endif
