/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include <TSystem.h>
#include <TFile.h>
#include "xAODRootAccess/tools/Message.h"
#include "xAODRootAccess/Init.h"
#include "xAODRootAccess/TEvent.h"
#include "xAODCore/ShallowCopy.h"
#include "AthContainers/ConstAccessor.h"

#include <ZdcNtuple/ZdcLEDNtuple.h>

ZdcLEDNtuple ::ZdcLEDNtuple(const std::string &name, ISvcLocator *pSvcLocator)
    : EL::AnaAlgorithm(name, pSvcLocator)
{
 

  declareProperty("enableOutputTree", enableOutputTree = true, "Enable output tree");
  declareProperty("auxSuffix", auxSuffix = "", "comment");

  m_eventCounter = 0;
}

StatusCode ZdcLEDNtuple ::initialize()
{
  
  ANA_MSG_DEBUG("Howdy from Initialize!");

  ANA_CHECK(m_zdcModuleContainerName.initialize());
  ANA_CHECK(m_zdcSumContainerName.initialize());

  if (enableOutputTree)
  {

    ANA_CHECK(book(TTree("zdcLEDTree", "ZDC LED Tree")));
    m_outputTree = tree("zdcLEDTree");

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

    m_outputTree->Branch("LEDType", &t_LEDType, "LEDType/i");

    m_outputTree->Branch("ZdcModulePresample", &t_ZdcModulePresample, Form("ZdcModulePresample[%d][%d]/F", nSides, nZDC));
    m_outputTree->Branch("ZdcModuleADCSum", &t_ZdcModuleADCSum, Form("ZdcModuleADCSum[%d][%d]/I", nSides, nZDC));
    m_outputTree->Branch("ZdcModuleMaxADC", &t_ZdcModuleMaxADC, Form("ZdcModuleMaxADC[%d][%d]/I", nSides, nZDC));
    m_outputTree->Branch("ZdcModuleMaxSample", &t_ZdcModuleMaxSample, Form("ZdcModuleMaxSample[%d][%d]/i", nSides, nZDC));
    m_outputTree->Branch("ZdcModuleAvgTime", &t_ZdcModuleAvgTime, Form("ZdcModuleAvgTime[%d][%d]/F", nSides, nZDC));
    m_outputTree->Branch("ZdcModuleRawsLowGain", &t_ZdcModuleg0data, Form("ZdcModuleRawsLowGain[%d][%d][%d]/s", nSides, nZDC, nSamples));
    m_outputTree->Branch("ZdcModuleRawsHighGain", &t_ZdcModuleg1data, Form("ZdcModuleRawsHighGain[%d][%d][%d]/s", nSides, nZDC, nSamples));
    m_outputTree->Branch("RPDModuleRaws", &t_RPDModuleRawdata, Form("RPDModuleRaws[%d][%d][%d]/s", nSides, nRPD, nSamples));
    m_outputTree->Branch("RPDModulePresample", &t_RPDModulePresample, Form("RPDModulePresample[%d][%d]/F", nSides, nRPD));
    m_outputTree->Branch("RPDModuleADCSum", &t_RPDModuleADCSum, Form("RPDModuleADCSum[%d][%d]/I", nSides, nRPD));
    m_outputTree->Branch("RPDModuleMaxADC", &t_RPDModuleMaxADC, Form("RPDModuleMaxADC[%d][%d]/I", nSides, nRPD));
    m_outputTree->Branch("RPDModuleMaxSample", &t_RPDModuleMaxSample, Form("RPDModuleMaxSample[%d][%d]/i", nSides, nRPD));
    m_outputTree->Branch("RPDModuleAvgTime", &t_RPDModuleAvgTime, Form("RPDModuleAvgTime[%d][%d]/F", nSides, nRPD));
  }

  return StatusCode::SUCCESS;
}

StatusCode ZdcLEDNtuple ::execute()
{
  if (!evtStore())
  {
    ANA_MSG_INFO("*** No event found! ***");
    return StatusCode::SUCCESS;
  }

  ANA_CHECK(evtStore()->retrieve(m_eventInfo, "EventInfo"));
  processEventInfo();
  processZdcLEDNtupleFromModules();

  if (enableOutputTree)
  {
    if (t_LEDType == 0 || t_LEDType == 1 || t_LEDType == 2 )
    {
      tree("zdcLEDTree")->Fill();
    }
  }

  return StatusCode::SUCCESS;
}

void ZdcLEDNtuple::processZdcLEDNtupleFromModules()
{
  // iside 0 is side C, iside 1 is side A
  SG::ReadHandle<xAOD::ZdcModuleContainer> zdcModules(m_zdcModuleContainerName);
  SG::ReadHandle<xAOD::ZdcModuleContainer> zdcSums(m_zdcSumContainerName);

  ANA_MSG_DEBUG("copying already processed info!");

  for (size_t iside = 0; iside < nSides; iside++)
  {
    for (int imod = 0; imod < nZDC; imod++)
    {
      t_ZdcModulePresample[iside][imod] = 0;
      t_ZdcModuleADCSum[iside][imod] = 0;
      t_ZdcModuleMaxADC[iside][imod] = 0;
      t_ZdcModuleMaxSample[iside][imod] = 0;
      t_ZdcModuleAvgTime[iside][imod] = 0;

      for (int isam = 0; isam < nSamples; isam++)
      {
        t_ZdcModuleg0data[iside][imod][isam] = 0;
        t_ZdcModuleg1data[iside][imod][isam] = 0;
      }
    }
    for (int imod = 0; imod < nRPD; imod++)
    {
      t_RPDModulePresample[iside][imod] = 0;
      t_RPDModuleADCSum[iside][imod] = 0;
      t_RPDModuleMaxADC[iside][imod] = 0;
      t_RPDModuleMaxSample[iside][imod] = 0;
      t_RPDModuleAvgTime[iside][imod] = 0;
    }
  }

  ANA_MSG_DEBUG("accessing ZdcModules");

  SG::ConstAccessor<int> ADCSumAcc("ADCSum" + auxSuffix);
  SG::ConstAccessor<unsigned int> LEDTypeAcc("LEDType" + auxSuffix);
  SG::ConstAccessor<float> PresampleAcc("Presample"+auxSuffix);
  SG::ConstAccessor<int> MaxADCAcc("MaxADC" + auxSuffix);
  SG::ConstAccessor<unsigned int> MaxSampleAcc("MaxSample" + auxSuffix);
  SG::ConstAccessor<float> AvgTimeAcc("AvgTime" + auxSuffix);
  SG::ConstAccessor<std::vector<uint16_t> > g0dataAcc("g0data" + auxSuffix);
  SG::ConstAccessor<std::vector<uint16_t> > g1dataAcc("g1data" + auxSuffix);

  if (zdcModules.ptr())
  {

    for (const auto zdcMod : *zdcModules)
    {
      if (ADCSumAcc.isAvailable(*zdcMod))
      {
        if (zdcSums.ptr())
        {
          for (const auto zdcSum : *zdcSums)
          {
            if (zdcSum->zdcSide() == infoSumInd)
            {
              if (LEDTypeAcc.isAvailable(*zdcSum))
              {
                t_LEDType = LEDTypeAcc(*zdcSum);
                break;
              }
            }
          }
        }
      }
      else
      {
        t_LEDType = 999;
        break;
      }
      if (zdcMod->zdcSide() != 0)
      { // side 0 is info
        int iside = 0;
        if (zdcMod->zdcSide() > 0)
          iside = 1;

        ANA_MSG_VERBOSE("Module " << zdcMod->zdcSide() << " " << zdcMod->zdcModule());
        if (zdcMod->zdcType() == ZdcTypeInd)
        {
          int imod = zdcMod->zdcModule();
	  if (!PresampleAcc.isAvailable(*zdcMod))
	    {
	      ANA_MSG_WARNING("Missing ZDC aux data");
	      continue;
	    }
          t_ZdcModulePresample[iside][imod] = PresampleAcc(*zdcMod);
          t_ZdcModuleADCSum[iside][imod] = ADCSumAcc(*zdcMod);
          t_ZdcModuleMaxADC[iside][imod] = MaxADCAcc(*zdcMod);
          t_ZdcModuleMaxSample[iside][imod] = MaxSampleAcc(*zdcMod);
          t_ZdcModuleAvgTime[iside][imod] = AvgTimeAcc(*zdcMod);

          g0dataVec = g0dataAcc(*zdcMod);
          g1dataVec = g1dataAcc(*zdcMod);

          for (int isam = 0; isam < nSamples; isam++)
          {
            t_ZdcModuleg0data[iside][imod][isam] = g0dataVec.at(isam);
            t_ZdcModuleg1data[iside][imod][isam] = g1dataVec.at(isam);
          }
        }
        if ((zdcMod->zdcType() == RPDTypeInd) && (zdcMod->zdcModule() == RPDModuleInd))
        {
          int imod = zdcMod->zdcChannel();
	  if (!PresampleAcc.isAvailable(*zdcMod))
	    {
	      ANA_MSG_WARNING("Missing RPD aux data");
	      continue;
	    }
          t_RPDModulePresample[iside][imod] = PresampleAcc(*zdcMod);
          t_RPDModuleADCSum[iside][imod] = ADCSumAcc(*zdcMod);
          t_RPDModuleMaxADC[iside][imod] = MaxADCAcc(*zdcMod);
          t_RPDModuleMaxSample[iside][imod] = MaxSampleAcc(*zdcMod);
          t_RPDModuleAvgTime[iside][imod] = AvgTimeAcc(*zdcMod);

          g0dataVec = g0dataAcc(*zdcMod);

          for (int isam = 0; isam < nSamples; isam++)
          {
            t_RPDModuleRawdata[iside][imod][isam] = g0dataVec.at(isam);
          }
        }
      }
    }
  }
}

void ZdcLEDNtuple::processEventInfo()
{
  ANA_MSG_DEBUG("processing event info");

  t_bcid = m_eventInfo->bcid();
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
    ANA_MSG_INFO("Event# " << m_eventCounter << "Run " << m_eventInfo->runNumber() << " Event " << m_eventInfo->eventNumber() << " LB " << m_eventInfo->lumiBlock());
  }
}

StatusCode ZdcLEDNtuple ::finalize()
{
   return StatusCode::SUCCESS;
}
