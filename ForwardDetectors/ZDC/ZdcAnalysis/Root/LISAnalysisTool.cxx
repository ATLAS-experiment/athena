/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include <AsgDataHandles/ReadHandle.h>
#include <AsgDataHandles/ReadDecorHandle.h>
#include <AsgDataHandles/WriteHandle.h>
#include <AsgDataHandles/WriteDecorHandle.h>
#include "ZdcAnalysis/LISAnalysisTool.h"
#include "AthContainers/ConstAccessor.h"
#include "xAODEventInfo/EventInfo.h"
#include "ZdcUtils/ZdcEventInfo.h"

namespace ZDC {

LISAnalysisTool::LISAnalysisTool(std::string const& name) : 
  asg::AsgTool(name),
  m_name(name)
{
#ifndef XAOD_STANDALONE
  declareInterface<IZdcAnalysisTool>(this);
#endif
}

StatusCode LISAnalysisTool::initialize() {
  ATH_MSG_INFO("Initializing LISAnalysisTool with configuration: " << m_configuration);
  ATH_MSG_INFO("Initializing LISAnalysisTool with BaselineStart: " << m_nBaselineStart << ", BaselineEnd: " << m_nBaselineEnd);
  
  initialize_default();

  ATH_MSG_INFO("LIS Configuration:");
  ATH_MSG_INFO("  NumSamples: " << m_numSamples);
  ATH_MSG_INFO("  Presample: " << m_preSample);
  ATH_MSG_INFO("  DeltaTSample: " << m_deltaTSample << " ns");
  ATH_MSG_INFO("  SampleAnaStart: " << m_sampleAnaStart);
  ATH_MSG_INFO("  SampleAnaEnd: " << m_sampleAnaEnd);

  // If an aux suffix is provided, prepend it with "_"
  std::string auxSuffix = m_auxSuffix;
  if (auxSuffix != "") auxSuffix = "_" + auxSuffix;

  // Initialize eventInfo access
  ATH_CHECK(m_eventInfoKey.initialize());

  // Initialize keys for reading ZDC event-level aux decor information
  m_eventTypeKey = m_zdcSumContainerName + ".EventType";
  ATH_CHECK(m_eventTypeKey.initialize());

  m_DAQModeKey = m_zdcSumContainerName + ".DAQMode";
  ATH_CHECK(m_DAQModeKey.initialize());

  m_robBCIDKey = m_zdcSumContainerName + ".rodBCID";
  ATH_CHECK(m_robBCIDKey.initialize());

  // Initialize WriteDecor handles
  if (m_writeAux) {
    m_ZdcLEDType = m_zdcSumContainerName + ".ZdcLEDType" + auxSuffix;
    ATH_CHECK(m_ZdcLEDType.initialize());

    m_LISPresampleADC = m_zdcModuleContainerName + ".LISPresample" + auxSuffix;
    ATH_CHECK(m_LISPresampleADC.initialize());

    m_LISADCSum = m_zdcModuleContainerName + ".LISADCSum" + auxSuffix;
    ATH_CHECK(m_LISADCSum.initialize());

    m_LISMaxADC = m_zdcModuleContainerName + ".LISMaxADC" + auxSuffix;
    ATH_CHECK(m_LISMaxADC.initialize());

    m_LISMaxSample = m_zdcModuleContainerName + ".LISMaxSample" + auxSuffix;
    ATH_CHECK(m_LISMaxSample.initialize());

    m_LISAvgTime = m_zdcModuleContainerName + ".LISAvgTime" + auxSuffix;
    ATH_CHECK(m_LISAvgTime.initialize());

  }

  m_init = true;
  ATH_MSG_INFO("LISAnalysisTool initialization complete");
  
  return StatusCode::SUCCESS;
}

void LISAnalysisTool::initialize_default() {
  // PbPb 2025 configuration
  m_numSamples = 24;
  m_preSample = 0;
  m_deltaTSample = 3.125;
  m_sampleAnaStart = 5;
  m_sampleAnaEnd = 23;
  m_nBaselineSamples = m_nBaselineEnd - m_nBaselineStart;

  m_LEDBCID = {3476, 3479, 3482};
  m_LEDCalreqIdx = {0 ,1 ,2};}

LISModuleResults LISAnalysisTool::processModuleData(
    int side, int channel,
    const std::vector<unsigned short>& data,
    unsigned int startSample, unsigned int endSample)
{
  ATH_MSG_DEBUG("Processing LIS data for side " << side << ", channel " << channel);

  int ADCSum = 0;
  int maxADCsub = -999;
  unsigned int maxSample = 0;
  float avgTime = 0.f;

  if (data.empty()) {
    ATH_MSG_DEBUG("Empty waveform data");
    return LISModuleResults();
  }

  if (startSample >= data.size() || endSample >= data.size()) {
    ATH_MSG_WARNING("Start or end sample number greater than number of samples");
    return LISModuleResults();
  }

  // Calculate presample (baseline) from first few samples
  float preFADC = 0;
  unsigned int nBaseline = std::min(m_nBaselineSamples, static_cast<unsigned int>(data.size()));
  for (unsigned int i = m_nBaselineStart; i < m_nBaselineEnd; ++i) {
    preFADC += data[i];
  }
  preFADC /= nBaseline;

  // Process samples in analysis window
  for (unsigned int sample = startSample; sample <= endSample; sample++) {
    int FADCsub = static_cast<int>(data[sample]) - static_cast<int>(std::round(preFADC));

    float time = (sample + 0.5f) * m_deltaTSample;
    ADCSum += FADCsub;

    if (FADCsub > maxADCsub) {
      maxADCsub = FADCsub;
      maxSample = sample;
    }

    avgTime += time * FADCsub;
  }

  // Calculate average time
  if (ADCSum != 0) {
    avgTime /= ADCSum;
  } else {
    avgTime = 0.f;
  }

  ATH_MSG_DEBUG("  Presample: " << preFADC << ", ADCSum: " << ADCSum 
               << ", MaxADC: " << maxADCsub << ", MaxSample: " << maxSample
               << ", AvgTime: " << avgTime);

  return LISModuleResults(preFADC, ADCSum, maxADCsub, maxSample, avgTime);
}

LISModuleResults LISAnalysisTool::processLISModule(const xAOD::ZdcModule& module) {
  ATH_MSG_DEBUG("Processing LIS module: side=" << module.zdcSide() 
               << ", module=" << module.zdcModule()
               << ", channel=" << module.zdcChannel());

  int LISModuleGain = 0; 

  if(module.zdcChannel() > 3){
    LISModuleGain = 1; // gain 1 for channels 4-7
  }
  else{
    LISModuleGain = 0; // gain 0 for channels 0-3
  }

   ATH_MSG_DEBUG("LIS module gain: " << LISModuleGain);

  // Determine which gain data to use
  static SG::ConstAccessor<std::vector<uint16_t>> g0dataAccessor("g0data");
  static SG::ConstAccessor<std::vector<uint16_t>> g1dataAccessor("g1data");
  const SG::ConstAccessor<std::vector<uint16_t>> &gainDataAccessor = (LISModuleGain == 0) ? g0dataAccessor : g1dataAccessor;

  // Get waveform data
  if (!gainDataAccessor.isAvailable(module)) {
    ATH_MSG_DEBUG("No gain data available for this module");
    return LISModuleResults();
  }
  
  const std::vector<uint16_t> &waveform = gainDataAccessor(module);
  if (waveform.empty()) {
    ATH_MSG_DEBUG("Empty waveform");
    return LISModuleResults();
  }

  return processModuleData(module.zdcSide(), module.zdcChannel(), 
                          waveform, m_sampleAnaStart, m_sampleAnaEnd);
}

StatusCode LISAnalysisTool::recoZdcModules(
    xAOD::ZdcModuleContainer const& moduleContainer,
    xAOD::ZdcModuleContainer const& moduleSumContainer) {
  
  ATH_MSG_DEBUG("LISAnalysisTool::recoZdcModules processing event");
  
  if (!m_init) {
    ATH_MSG_WARNING("Tool not initialized!");
    return StatusCode::FAILURE;
  }

  if (moduleContainer.empty()) {
    return StatusCode::SUCCESS;
  }
  // Check for decoding errors
  SG::ReadHandle<xAOD::EventInfo> eventInfo(m_eventInfoKey);
  if (!eventInfo.isValid()) {
    ATH_MSG_WARNING("EventInfo not valid");
    return StatusCode::FAILURE;
  }

  bool lisErr = eventInfo->isEventFlagBitSet(xAOD::EventInfo::ForwardDet, ZdcEventInfo::LISDECODINGERROR);
  if (lisErr) {
    ATH_MSG_WARNING("LIS decoding error found - abandoning LISAnalysisTool!");
    return StatusCode::SUCCESS;
  }

  SG::ReadDecorHandle<xAOD::ZdcModuleContainer, unsigned int> eventTypeHandle(m_eventTypeKey);
  SG::ReadDecorHandle<xAOD::ZdcModuleContainer, std::vector<uint16_t>> rodBCIDHandle(m_robBCIDKey);

  // Loop over the sum container to find event-level info (side == 0)
  //
  bool haveZdcEventInfo = false;
  unsigned int eventType = ZdcEventInfo::ZdcEventUnknown;
  unsigned int bcid = 0;

  const xAOD::ZdcModule* moduleSumEventInfo_ptr = 0;

  for (auto modSum : moduleSumContainer) {
    //
    // Module sum object with side == 0 contains event-level information
    //
    if (modSum->zdcSide() == 0) {
      //
      // Add the event type and bcid as aux decors
      //
      ATH_MSG_DEBUG("Found global sum");
      eventType = eventTypeHandle(*modSum);
      haveZdcEventInfo = true;
      moduleSumEventInfo_ptr = modSum;
    }
  }

    if (!haveZdcEventInfo) {
    ATH_MSG_ERROR("Zdc event data not available (moduleSum with side = 0)");
    return StatusCode::FAILURE;
  }

  // Get the BCID from the rodBCID vector
  const std::vector<uint16_t>& rodBCID = rodBCIDHandle(*moduleSumEventInfo_ptr);
  if (!rodBCID.empty()) {
    bcid = rodBCID[0]; // Use first BCID from LUCROD
    ATH_MSG_DEBUG("Retrieved BCID from rodBCID: " << bcid);
  } else {
    ATH_MSG_WARNING("rodBCID vector is empty, using EventInfo BCID");
    bcid = eventInfo->bcid();
  }
   
  // Determine the LED type
  //
  unsigned int evtLEDType = ZdcEventInfo::LEDNone;

  for (unsigned int idxLED = 0; idxLED < ZdcEventInfo::NumLEDs; idxLED++) {
    //
    //  Does the BCID match one of those associated with the LEDs?
    //
    if (m_LEDBCID[idxLED] == bcid) {
    
      evtLEDType = idxLED;
      break;
    }
  }

  if (evtLEDType == ZdcEventInfo::LEDNone && eventType == ZdcEventInfo::ZdcEventLED) {
    //
    // Thie BCID does not appear to be associated with one of the LEDs, print warning and quit processing
    //
    ATH_MSG_WARNING("LED event: Unexpected BCID found in data: bcid = " << bcid  << m_configuration);
    return StatusCode::SUCCESS;
  }
  else {
    ATH_MSG_DEBUG("Event with BCID = " << bcid << " has LED type " << evtLEDType);
  }


  // Create write decoration handles
  SG::WriteDecorHandle<xAOD::ZdcModuleContainer,unsigned int> LEDTypeHandle(m_ZdcLEDType);
  SG::WriteDecorHandle<xAOD::ZdcModuleContainer, float> presampleHandle(m_LISPresampleADC);
  SG::WriteDecorHandle<xAOD::ZdcModuleContainer, int> adcSumHandle(m_LISADCSum);
  SG::WriteDecorHandle<xAOD::ZdcModuleContainer, int> maxADCHandle(m_LISMaxADC);
  SG::WriteDecorHandle<xAOD::ZdcModuleContainer, unsigned int> maxSampleHandle(m_LISMaxSample);
  SG::WriteDecorHandle<xAOD::ZdcModuleContainer, float> avgTimeHandle(m_LISAvgTime);

  ATH_MSG_DEBUG("Processing " << moduleContainer.size() << " modules");

  // Loop over modules and process LIS channels (module=5, type=2)
  for (const auto* zdcModule : moduleContainer) {
      
      LISModuleResults results = processLISModule(*zdcModule);



      ATH_MSG_DEBUG("Writing aux decors to LIS module: side=" << zdcModule->zdcSide() 
                   << ", channel=" << zdcModule->zdcChannel());

      // Write decorations
      presampleHandle(*zdcModule) = results.getPresampleADC();
      adcSumHandle(*zdcModule) = results.getADCSum();
      maxADCHandle(*zdcModule) = results.getMaxADC();
      maxSampleHandle(*zdcModule) = results.getMaxSample();
      avgTimeHandle(*zdcModule) = results.getAvgTime();
    
  }


  if(eventType == ZdcEventInfo::ZdcEventLED){
  // Write the LED type to the moduleSum container keep event-level data
  //
  LEDTypeHandle(*moduleSumEventInfo_ptr) = evtLEDType;
  }

  ATH_MSG_DEBUG("Finished event processing");

  return StatusCode::SUCCESS;
}

StatusCode LISAnalysisTool::reprocessZdc() {
  if (!m_init) {
    ATH_MSG_WARNING("Tool not initialized!");
    return StatusCode::FAILURE;
  }

  ATH_MSG_DEBUG("Trying to retrieve " << m_zdcModuleContainerName);

  m_zdcModules = nullptr;
  ATH_CHECK(evtStore()->retrieve(m_zdcModules, m_zdcModuleContainerName));

  m_zdcSums = nullptr;
  ATH_CHECK(evtStore()->retrieve(m_zdcSums, m_zdcSumContainerName));

  ATH_CHECK(recoZdcModules(*m_zdcModules, *m_zdcSums));

  return StatusCode::SUCCESS;
}

} // namespace ZDC
