/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ZdcNtuple_ZdcLEDNtuple_H
#define ZdcNtuple_ZdcLEDNtuple_H

#include <AnaAlgorithm/AnaAlgorithm.h>
#include "AsgDataHandles/ReadHandleKey.h"
#include "xAODEventInfo/EventInfo.h"
#include "xAODForward/ZdcModuleContainer.h"
#include <vector>
#include <string>

class ZdcLEDNtuple : public EL::AnaAlgorithm
{
public:
  std::string auxSuffix; // what to add to name the new data, when reprocessing
  bool isLED{};

public:
  SG::ReadHandleKey<xAOD::ZdcModuleContainer> m_zdcModuleContainerName{this, "ZdcModuleContainerName", "ZdcModules", ""};
  SG::ReadHandleKey<xAOD::ZdcModuleContainer> m_zdcSumContainerName{ this, "ZdcSumContainerName", "ZdcSums", "" };
  const xAOD::EventInfo *m_eventInfo{};
  int m_eventCounter{};

  // flags
  bool enableOutputTree{}; // enable output TTree

  // output tree and branches

  TTree *m_outputTree{};

  // evt info
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

  // LED and modules
  unsigned int t_LEDType{};

  static constexpr int nSides = 2;
  static constexpr int nZDC = 4;
  static constexpr int nRPD = 16;
  static constexpr int nSamples = 24;

  static constexpr int ZdcTypeInd = 0; 
  static constexpr int RPDTypeInd = 1; 

  static constexpr int RPDModuleInd = 4; 
  static constexpr int infoSumInd = 0; 


  float t_ZdcModulePresample[nSides][nZDC]{};
  int t_ZdcModuleADCSum[nSides][nZDC]{};
  int t_ZdcModuleMaxADC[nSides][nZDC]{};
  unsigned int t_ZdcModuleMaxSample[nSides][nZDC]{};
  float t_ZdcModuleAvgTime[nSides][nZDC]{};

  uint16_t t_ZdcModuleg0data[nSides][nZDC][nSamples]{};
  uint16_t t_ZdcModuleg1data[nSides][nZDC][nSamples]{};

  uint16_t t_RPDModuleRawdata[nSides][nRPD][nSamples]{};

  std::vector<uint16_t> g0dataVec;
  std::vector<uint16_t> g1dataVec;

  float t_RPDModulePresample[nSides][nRPD]{};
  int t_RPDModuleADCSum[nSides][nRPD]{};
  int t_RPDModuleMaxADC[nSides][nRPD]{};
  unsigned int t_RPDModuleMaxSample[nSides][nRPD]{};
  float t_RPDModuleAvgTime[nSides][nRPD]{};

  ZdcLEDNtuple(const std::string &name, ISvcLocator *pSvcLocator);

  void processEventInfo();
  void processZdcLEDNtupleFromModules();

  virtual StatusCode initialize() override;
  virtual StatusCode execute() override;
  virtual StatusCode finalize() override;
};

#endif