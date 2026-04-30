/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include <TSystem.h>
#include <TFile.h>
#include "AthContainers/ConstAccessor.h"
#include "xAODRootAccess/TEvent.h"
#include <ZdcConditions/ZdcInjPulserAmpMap.h>
#include <ZdcNtuple/LisNtuple.h>

LisNtuple::LisNtuple(const std::string &name, ISvcLocator *pSvcLocator)
    : EL::AnaAlgorithm(name, pSvcLocator)
{
  declareProperty("enableOutputTree", m_enableOutputTree = true, "Enable output tree");
  declareProperty("auxSuffix", m_auxSuffix = "", "Suffix for aux data names");
  declareProperty("lisInj", m_lisInj = false, "LIS injected-pulse run");
  declareProperty("lisLED", m_lisLED = false, "LIS LED run");
  declareProperty("enableTrigger",  enableTrigger = true, "use trigger info");

  m_eventCounter = 0;
}

StatusCode LisNtuple::initialize()
{
  ANA_MSG_DEBUG("Initializing LisNtuple");

  ANA_CHECK(m_zdcModuleContainerName.initialize());
  ANA_CHECK(m_zdcSumContainerName.initialize());

  if (m_enableOutputTree)
  {
    ANA_CHECK(book(TTree("lisTree", "LIS Tree")));
    m_outputTree = tree("lisTree");

    // Event info branches
    m_outputTree->Branch("bcid", &t_bcid, "bcid/i");
    m_outputTree->Branch("runNumber", &t_runNumber, "runNumber/i");
    m_outputTree->Branch("eventNumber", &t_eventNumber, "eventNumber/i");
    m_outputTree->Branch("lumiBlock", &t_lumiBlock, "lumiBlock/i");
    m_outputTree->Branch("bunchGroup", &t_bunchGroup, "bunchGroup/b");
    m_outputTree->Branch("extendedLevel1ID", &t_extendedLevel1ID, "extendedLevel1ID/i");
    m_outputTree->Branch("timeStamp", &t_timeStamp, "timeStamp/i");
    m_outputTree->Branch("timeStampNSOffset", &t_timeStampNSOffset, "timeStampNSOffset/i");

    m_outputTree->Branch("avgIntPerCrossing", &t_avgIntPerCrossing, "avgIntPerCrossing/F");
    m_outputTree->Branch("actIntPerCrossing", &t_actIntPerCrossing, "actIntPerCrossing/F");
    m_outputTree->Branch("tbp", &t_tbp, "tbp[16]/i");
    m_outputTree->Branch("tav", &t_tav, "tav[16]/i");

    if (m_lisLED)
      {m_outputTree->Branch("LEDType", &t_LEDType, "LEDType/i");}
    if (m_lisInj)
      {m_outputTree->Branch("vInj",&t_vInj,"vInj/F");}

    // LIS processed data branches
    m_outputTree->Branch("LISPresample", &t_LISPresample, Form("LISPresample[%d]/F", nLISChannels));
    m_outputTree->Branch("LISADCSum", &t_LISADCSum, Form("LISADCSum[%d]/I", nLISChannels));
    m_outputTree->Branch("LISMaxADC", &t_LISMaxADC, Form("LISMaxADC[%d]/I", nLISChannels));
    m_outputTree->Branch("LISMaxSample", &t_LISMaxSample, Form("LISMaxSample[%d]/i", nLISChannels));
    m_outputTree->Branch("LISAvgTime", &t_LISAvgTime, Form("LISAvgTime[%d]/F", nLISChannels));
    m_outputTree->Branch("LISModuleStatus", &t_LISModuleStatus, Form("LISModuleStatus[%d]/i", nLISChannels));
    // LIS raw waveform data
    m_outputTree->Branch("LISRawdata", &t_LISRawdata, Form("LISRawdata[%d][%d]/s", nLISChannels, nSamples));

    if (m_lisInj)
      {
        m_zdcInjPulserAmpMap = std::make_unique<ZdcInjPulserAmpMap>();
        ATH_MSG_INFO( "Using JSON file for injector-pulse voltage at path " << m_zdcInjPulserAmpMap->getFilePath() );
      }
    
    }

  return StatusCode::SUCCESS;
}

StatusCode LisNtuple::execute()
{
  if (!evtStore())
  {
    ANA_MSG_INFO("*** No event found! ***");
    return StatusCode::SUCCESS;
  }

  ANA_CHECK(evtStore()->retrieve(m_eventInfo, "EventInfo"));
  processEventInfo();

  if (m_lisInj){
  	processVInjInfo();
  }

  bool passTrigger = true;
  m_trigDecision = 0;
  if (enableTrigger)
  {
    ANA_CHECK(evtStore()->retrieve( m_trigDecision, "xTrigDecision"));
    passTrigger = processTriggerDecision();
  }

  if(!passTrigger){
    // The trigger chains will be implemented in the future, for now just fill the tree with all events
    //return StatusCode::FAILURE;
  }

  processLisNtupleFromModules();

  if (m_enableOutputTree)
  {
    tree("lisTree")->Fill();
  }

  return StatusCode::SUCCESS;
}

void LisNtuple::processLisNtupleFromModules()
{
  // iside 0 is side C, iside 1 is side A
  SG::ReadHandle<xAOD::ZdcModuleContainer> zdcModules(m_zdcModuleContainerName);
  SG::ReadHandle<xAOD::ZdcModuleContainer> zdcSums(m_zdcSumContainerName);

  ANA_MSG_DEBUG("Processing LIS modules");

  // Initialize arrays to zero
  
    for (int ichan = 0; ichan < nLISChannels; ichan++)
    {
      t_LISPresample[ichan] = 0;
      t_LISADCSum[ichan] = 0;
      t_LISMaxADC[ichan] = 0;
      t_LISMaxSample[ichan] = 0;
      t_LISAvgTime[ichan] = 0;
      t_LISModuleStatus[ichan] = 0;

      for (int isam = 0; isam < nSamples; isam++)
      {
        t_LISRawdata[ichan][isam] = 0;
      }
    }
  

  // Default LED type
  t_LEDType = 999;

  ANA_MSG_DEBUG("Accessing ZdcModules for LIS data");

  static const SG::ConstAccessor<unsigned int> ZdcLEDTypeAccessor("ZdcLEDType" + m_auxSuffix);
  static const SG::ConstAccessor<unsigned int> LEDTypeAccessor("LEDType" + m_auxSuffix);
  static const SG::ConstAccessor<float> LISPresampleAccessor("LISPresample" + m_auxSuffix);
  static const SG::ConstAccessor<int> LISADCSumAccessor("LISADCSum" + m_auxSuffix);
  static const SG::ConstAccessor<int> LISMaxADCAccessor("LISMaxADC" + m_auxSuffix);
  static const SG::ConstAccessor<unsigned int> LISMaxSampleAccessor("LISMaxSample" + m_auxSuffix);
  static const SG::ConstAccessor<float> LISAvgTimeAccessor("LISAvgTime" + m_auxSuffix);
  static const SG::ConstAccessor<unsigned int> LISModuleStatusAccessor("LISModuleStatus" + m_auxSuffix);
  static const SG::ConstAccessor<std::vector<uint16_t>> g0dataAccessor("g0data" + m_auxSuffix);
  static const SG::ConstAccessor<std::vector<uint16_t>> g1dataAccessor("g1data" + m_auxSuffix);

  if (zdcModules.ptr())
  {
    for (const auto zdcMod : *zdcModules)
    {
      // Get LED type from sum container
      if (zdcSums.ptr())
      {
        for (const auto zdcSum : *zdcSums)
        {
          if (zdcSum->zdcSide() == infoSumInd)
          {
            if (ZdcLEDTypeAccessor.isAvailable(*zdcSum))
            {
              t_LEDType = ZdcLEDTypeAccessor(*zdcSum);
              break;
            }
            // Fallback to LEDType if ZdcLEDType not available
            else if (LEDTypeAccessor.isAvailable(*zdcSum))
            {
              t_LEDType = LEDTypeAccessor(*zdcSum);
              break;
            }
          }
        }
      }

      // Skip side 0 (info container)
      if (zdcMod->zdcSide() == 0)
        continue;

      // Check if this is a LIS module (type=2, module=5)
      if (zdcMod->zdcType() == LISTypeInd && zdcMod->zdcModule() == LISModuleInd)
      {

        if(zdcMod->zdcSide() > 0){
            ANA_MSG_WARNING("No LIS on side A, whats that?");
        }
       
        int ichan = zdcMod->zdcChannel();

        // Bounds check
        if (ichan < 0 || ichan >= nLISChannels)
        {
          ANA_MSG_WARNING("LIS channel " << ichan << " out of range");
          continue;
        }

        ANA_MSG_VERBOSE("LIS Module side " << zdcMod->zdcSide() << " channel " << ichan);

        // Check for LIS aux data availability
        if (!LISPresampleAccessor.isAvailable(*zdcMod))
        {
          ANA_MSG_WARNING("Missing LIS aux data for side " << zdcMod->zdcSide() << " channel " << ichan);
          continue;
        }

        // Read processed LIS data
        t_LISPresample[ichan] = LISPresampleAccessor(*zdcMod);
        t_LISADCSum[ichan] = LISADCSumAccessor(*zdcMod);
        t_LISMaxADC[ichan] = LISMaxADCAccessor(*zdcMod);
        t_LISMaxSample[ichan] = LISMaxSampleAccessor(*zdcMod);
        t_LISAvgTime[ichan] = LISAvgTimeAccessor(*zdcMod);
        t_LISModuleStatus[ichan] = LISModuleStatusAccessor(*zdcMod);
        // Read raw waveform data
        if(ichan > 3){
        if (g1dataAccessor.isAvailable(*zdcMod))
        {
          const std::vector<uint16_t> &g1dataVec = g1dataAccessor(*zdcMod);
          for (int isam = 0; isam < nSamples && isam < static_cast<int>(g1dataVec.size()); isam++)
          {
            t_LISRawdata[ichan][isam] = g1dataVec.at(isam);
          }
        }
        }
        else{
        if (g0dataAccessor.isAvailable(*zdcMod))
        {
          const std::vector<uint16_t> &g0dataVec = g0dataAccessor(*zdcMod);
          for (int isam = 0; isam < nSamples && isam < static_cast<int>(g0dataVec.size()); isam++)
          {
            t_LISRawdata[ichan][isam] = g0dataVec.at(isam);
          }
        }
        }
        
      }
    }
  }
}

void LisNtuple::processEventInfo()
{
  ANA_MSG_DEBUG("Processing event info");

  // Get BCID from ZdcSums rodBCID decoration (from LUCROD)
  static const SG::Accessor<std::vector<uint16_t>> rodBCIDAcc("rodBCID");
  SG::ReadHandle<xAOD::ZdcModuleContainer> zdcSums(m_zdcSumContainerName);
  
  t_bcid = m_eventInfo->bcid(); // Default from EventInfo
  
  if (zdcSums.ptr()) {
    for (const auto zdcSum : *zdcSums) {
      if (zdcSum->zdcSide() == infoSumInd) {
        if (rodBCIDAcc.isAvailable(*zdcSum)) {
          const std::vector<uint16_t>& rodBCID = rodBCIDAcc(*zdcSum);
          if (!rodBCID.empty()) {
            t_bcid = rodBCID[0]; // Use BCID from LUCROD
            ANA_MSG_DEBUG("Using BCID from rodBCID: " << t_bcid);
          }
        }
        break;
      }
    }
  }
  
  t_runNumber = m_eventInfo->runNumber();
  t_eventNumber = m_eventInfo->eventNumber();
  t_lumiBlock = m_eventInfo->lumiBlock();
  t_bunchGroup = -1;
  t_extendedLevel1ID = m_eventInfo->extendedLevel1ID();
  t_timeStamp = m_eventInfo->timeStamp();
  t_timeStampNSOffset = m_eventInfo->timeStampNSOffset();
  t_avgIntPerCrossing = m_eventInfo->averageInteractionsPerCrossing();
  t_actIntPerCrossing = m_eventInfo->actualInteractionsPerCrossing();

  if (!(m_eventCounter++ % 1000))
  {
    ANA_MSG_INFO("Event# " << m_eventCounter << " Run " << m_eventInfo->runNumber() 
                 << " Event " << m_eventInfo->eventNumber() << " LB " << m_eventInfo->lumiBlock());
  }
}

StatusCode LisNtuple::finalize()
{
  return StatusCode::SUCCESS;
}

bool LisNtuple::processTriggerDecision()  // reimpleneted from ZdcNtuple.cxx
{
  ANA_MSG_DEBUG ("Processing trigger");

  bool passTrigger = false;

  t_trigger = 0;
  t_trigger_TBP = 0;

  for (int i = 0; i < 16; i++)
  {
    t_tav[i] = 0;
    t_tbp[i] = 0;
  }

  if (m_trigDecision)
    {
      for (int i = 0; i < 16; i++)
	    {
	      t_tbp[i] = m_trigDecision->tbp().at(i);
	      t_tav[i] = m_trigDecision->tav().at(i);	  
	      ANA_MSG_DEBUG( "TD: " << i << " tbp: " << std::hex << t_tbp[i] << "\t" << t_tav[i] );
	    }
      t_bunchGroup = m_trigDecision->bgCode();
    }

  return passTrigger;
}

void LisNtuple::processVInjInfo(){
	// Check for new run number
  //
  if (t_runNumber != m_lastRunNumber) {
    //
    // Get access to the injector pulse steps for this run
    //
    m_injMapRunToken = m_zdcInjPulserAmpMap->lookupRun(t_runNumber, true);
    if (!m_injMapRunToken.isValid()) {
      ANA_MSG_ERROR("Unable to obtain injector pulse steps for run " << t_runNumber);
    }
    else {
      unsigned int startLB = m_zdcInjPulserAmpMap->getFirstLumiBlock(m_injMapRunToken);
      unsigned int nsteps = m_zdcInjPulserAmpMap->getNumSteps(m_injMapRunToken);
      ANA_MSG_DEBUG("Successfully obtained injector pulse steps for run " << t_runNumber
		    << ", first LB = " << startLB << ", number of steps = " << nsteps);
    }

    // update the last run number to be the current run number
    m_lastRunNumber = t_runNumber;
  }
  
  t_vInj = m_zdcInjPulserAmpMap->getPulserAmplitude(m_injMapRunToken, t_lumiBlock);
}
