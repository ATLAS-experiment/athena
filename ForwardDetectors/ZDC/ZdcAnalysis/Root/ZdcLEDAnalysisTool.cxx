/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include <sstream>
#include <memory>

#include "ZdcUtils/ZdcEventInfo.h"
#include "ZdcAnalysis/ZdcLEDAnalysisTool.h"
#include "xAODEventInfo/EventInfo.h"
#include <xAODForward/ZdcModuleAuxContainer.h>
#include <AsgDataHandles/ReadHandle.h>
#include "AsgDataHandles/ReadDecorHandle.h"
#include <AsgDataHandles/WriteHandle.h>
#include <AsgDataHandles/WriteDecorHandle.h>
#include "AthContainers/ConstAccessor.h"
#include "PathResolver/PathResolver.h"
#include <TFile.h>

namespace ZDC
{
  ZdcLEDAnalysisTool::ZdcLEDAnalysisTool(const std::string& name) :
    asg::AsgTool(name),
    m_name(name)
{

  
#ifndef XAOD_STANDALONE
  declareInterface<IZdcAnalysisTool>(this);
#endif
    declareProperty("ForceCalibRun", m_forceCalibRun = -1); 
    declareProperty("DoFADCCorr", m_doFADCCorr = false); 
}

ZdcLEDAnalysisTool::~ZdcLEDAnalysisTool()
{
  ATH_MSG_DEBUG("Deleting ZdcLEDAnalysisTool named " << m_name);
}

StatusCode ZdcLEDAnalysisTool::initialize()
{
  // Use configuration to direct initialization. 
  //
  if (m_configuration == "ppPbPb2023") {
    initialize_ppPbPb2023();
  }
  else if (m_configuration == "ppALFA2023") {
    initialize_ppALFA2023();
  }
  else if (m_configuration == "zdcStandalone") {
    initialize_zdcStandalone();
  }
  else {
    ATH_MSG_ERROR("Unknown configuration: "  << m_configuration);
    return StatusCode::FAILURE;
  }

  // Check for valid configuration
  //
  if (m_LEDBCID.size() != ZdcEventInfo::NumLEDs || m_LEDCalreqIdx.size() != ZdcEventInfo::NumLEDs) {
    ATH_MSG_ERROR("Invalid initialization of tool for configuration " <<  m_configuration);
    return StatusCode::FAILURE;
  }
  
  ATH_MSG_INFO("doZDC: " << (m_doZDC ? "true" : "false"));
  ATH_MSG_INFO("doRPD: " << (m_doRPD ? "true" : "false"));
  ATH_MSG_INFO("Configuration: " << m_configuration);
  ATH_MSG_DEBUG("AuxSuffix: " << m_auxSuffix);
  ATH_MSG_DEBUG("NumSamples: " << m_numSamples);
  ATH_MSG_DEBUG("Presample: " << m_preSample);
  ATH_MSG_DEBUG("DeltaTSample: " << m_deltaTSample);

  if (m_writeAux && m_auxSuffix != "") {
    ATH_MSG_DEBUG("suffix string = " << m_auxSuffix);
  }

  std::ostringstream BCIDList;
  BCIDList << "LED BCIDs:";

  for (unsigned int idxLED = 0; idxLED < ZdcEventInfo::NumLEDs; idxLED++) {
    BCIDList << m_LEDNames[idxLED] << " - BCID " << m_LEDBCID[idxLED];
    if (idxLED < ZdcEventInfo::NumLEDs - 1) BCIDList << ", ";
  }

  ATH_MSG_DEBUG(BCIDList.str());

  
  // If an aux suffix is provided, prepend it with "_" so we don't have to do so at each use
  //
  if (m_auxSuffix != "") m_auxSuffix = "_" + m_auxSuffix;

  // initialize eventInfo access
  //
  ATH_CHECK( m_eventInfoKey.initialize());
  
  // initialize keys for reading ZDC event-level aux decor information
  //
  m_eventTypeKey = m_zdcSumContainerName + ".EventType";
  ATH_CHECK(m_eventTypeKey.initialize());

  m_DAQModeKey = m_zdcSumContainerName + ".DAQMode";
  ATH_CHECK(m_DAQModeKey.initialize());

  m_robBCIDKey = m_zdcSumContainerName + ".rodBCID";
  ATH_CHECK(m_robBCIDKey.initialize());

  // Initialize writeDecor handles
  //
  m_ZdcLEDPresampleADC = m_zdcModuleContainerName + ".Presample" + m_auxSuffix;
  ATH_CHECK( m_ZdcLEDPresampleADC.initialize());

  m_ZdcLEDADCSum = m_zdcModuleContainerName + ".ADCSum" + m_auxSuffix;
  ATH_CHECK( m_ZdcLEDADCSum.initialize());

  m_ZdcLEDMaxADC = m_zdcModuleContainerName + ".MaxADC" + m_auxSuffix;
  ATH_CHECK( m_ZdcLEDMaxADC.initialize());
  
  m_ZdcLEDMaxSample = m_zdcModuleContainerName + ".MaxSample" + m_auxSuffix;
  ATH_CHECK( m_ZdcLEDMaxSample.initialize());
  
  m_ZdcLEDAvgTime = m_zdcModuleContainerName + ".AvgTime" + m_auxSuffix;
  ATH_CHECK( m_ZdcLEDAvgTime.initialize());
      
  // The LED type gets writting to the module 0 sum container
  //
  m_ZdcLEDType = m_zdcSumContainerName + ".LEDType" + m_auxSuffix;
  ATH_CHECK( m_ZdcLEDType.initialize());

  // prepare FADC correction per sample
  
  m_init = true;
  return StatusCode::SUCCESS;
}

void ZdcLEDAnalysisTool::initialize_zdcStandalone()
{
  // Use the defaults for now except for sampleAnaStart values and the BCIDs
  //
  m_sampleAnaStartZDC = 5;
  m_sampleAnaStartRPD = 5;

  m_LEDBCID = {1152, 1154, 1156};
  m_LEDCalreqIdx = {2, 1, 0};
}

void ZdcLEDAnalysisTool::initialize_ppPbPb2023()
{
  // Use the defaults for now except for sampleAnaStart values and the BCIDs
  //
  m_sampleAnaStartZDC = 5;
  m_sampleAnaStartRPD = 5;

  m_LEDBCID = {3476, 3479, 3482};
  m_LEDCalreqIdx = {0, 1, 2};
}

void ZdcLEDAnalysisTool::initialize_ppALFA2023()
{
  // Use the defaults for now except for sampleAnaStart values and the BCIDs
  m_sampleAnaStartZDC = 5;
  m_sampleAnaStartRPD = 5;

  m_LEDBCID = {3479, 3482, 3485};
  m_LEDCalreqIdx = {0 ,1 ,2};
}
  
StatusCode ZdcLEDAnalysisTool::recoZdcModules(const xAOD::ZdcModuleContainer& moduleContainer, 
					      const xAOD::ZdcModuleContainer& moduleSumContainer)
{
  if (!m_init) {
    ATH_MSG_WARNING("Tool not initialized!");
    return StatusCode::FAILURE;
  }

  if (moduleContainer.size()==0) return StatusCode::SUCCESS; // if no modules, do nothing

  SG::ReadHandle<xAOD::EventInfo> eventInfo(m_eventInfoKey);
  if (!eventInfo.isValid()) return StatusCode::FAILURE;
  
  bool zdcErr = eventInfo->isEventFlagBitSet(xAOD::EventInfo::ForwardDet, ZdcEventInfo::ZDCDECODINGERROR );
  bool rpdErr = eventInfo->isEventFlagBitSet(xAOD::EventInfo::ForwardDet, ZdcEventInfo::RPDDECODINGERROR );
  if (zdcErr||rpdErr)
    {
      ATH_MSG_WARNING("ZDC or RPD decoding error found - abandoning ZdcLEDAnalysisTool!");
      return StatusCode::SUCCESS;
    }

  unsigned int thisRunNumber = eventInfo->runNumber();
  if (thisRunNumber != m_runNumber) {
    ATH_MSG_INFO("ZDC LED analysis tool will be configured for run " << thisRunNumber << " m_doFADCCorr = " << m_doFADCCorr);
    if (m_doFADCCorr)
      {
	unsigned int calibRunNumber = thisRunNumber;
	if (m_forceCalibRun > -1) calibRunNumber = m_forceCalibRun;
	ATH_MSG_INFO("FADC corrections will be configured for run " << calibRunNumber);
	setFADCCorrections(calibRunNumber);
      }
    m_runNumber = thisRunNumber;
  }
  SG::ReadDecorHandle<xAOD::ZdcModuleContainer, unsigned int> eventTypeHandle(m_eventTypeKey);
  SG::ReadDecorHandle<xAOD::ZdcModuleContainer, unsigned int> DAQModeHandle(m_DAQModeKey);

  // Loop over the sum container to find event-level info (side == 0)
  //
  bool haveZdcEventInfo = false;
  unsigned int eventType = ZdcEventInfo::ZdcEventUnknown;
  unsigned int bcid = eventInfo->bcid();
  unsigned int DAQMode = ZdcEventInfo::DAQModeUndef;

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
      DAQMode = DAQModeHandle(*modSum);
      haveZdcEventInfo = true;
      moduleSumEventInfo_ptr = modSum;
    }
  }
  if (!haveZdcEventInfo) {
    ATH_MSG_ERROR("Zdc event data not available (moduleSum with side = 0)");
    return StatusCode::FAILURE;
  }

  //
  // only do something on LED calibration events
  //
  if (eventType != ZdcEventInfo::ZdcEventLED) return StatusCode::SUCCESS;

  // In standalone mode, we have to read the BCID from the rob header
  //
  if (DAQMode == ZdcEventInfo::Standalone) {  
    SG::ReadDecorHandle<xAOD::ZdcModuleContainer, std::vector<uint16_t> > robBCIDHandle(m_robBCIDKey);
    if (!robBCIDHandle.isValid()) return StatusCode::FAILURE;

    const std::vector<uint16_t>& robBCIDvec = robBCIDHandle(*moduleSumEventInfo_ptr);
    if (robBCIDHandle->size() == 0) return StatusCode::FAILURE;
    
    unsigned int checkBCID = robBCIDvec[0];
    for (unsigned int bcid : robBCIDvec) {
      if (bcid != checkBCID) {
	ATH_MSG_ERROR("Inconsistent BCIDs in rob header, cannot continue in standalone mode");
	return StatusCode::FAILURE;
      }
    }

    bcid = checkBCID;
  }

  // Determine the LED type
  //
  unsigned int evtLEDType = ZdcEventInfo::LEDNone;

  for (unsigned int idxLED = 0; idxLED < ZdcEventInfo::NumLEDs; idxLED++) {
    //
    //  Does the BCID match one of those associated with the LEDs?
    //
    if (m_LEDBCID[idxLED] == bcid) {
      if (DAQMode != ZdcEventInfo::Standalone) {
	//
	// Also check the calreq trigger (to be implemented)
	//
	if (false) continue;
      }
      
      evtLEDType = idxLED;
      break;
    }
  }

  if (evtLEDType == ZdcEventInfo::LEDNone) {
    //
    // Thie BCID does not appear to be associated with one of the LEDs, print warning and quit processing
    //
    ATH_MSG_WARNING("Unexpected BCID found in data: bcid = " << bcid  << m_configuration);
    return StatusCode::SUCCESS;
  }
  else {
    ATH_MSG_DEBUG("Event with BCID = " << bcid << " has LED type " << evtLEDType);
  }

  // We are currently calculating the presample as an unsigned it, but there is another "presample"
  //   from ZdcAnalysisTool which for good reasons is float. So we have to match the type.
  // 
  SG::WriteDecorHandle<xAOD::ZdcModuleContainer,float> moduleLEDPresampleADCHandle(m_ZdcLEDPresampleADC);
  SG::WriteDecorHandle<xAOD::ZdcModuleContainer,int> moduleLEDADCSumHandle(m_ZdcLEDADCSum);
  SG::WriteDecorHandle<xAOD::ZdcModuleContainer,int> moduleLEDMaxADCHandle(m_ZdcLEDMaxADC);
  SG::WriteDecorHandle<xAOD::ZdcModuleContainer,unsigned int> moduleLEDMaxSampleHandle(m_ZdcLEDMaxSample);
  SG::WriteDecorHandle<xAOD::ZdcModuleContainer,float> moduleLEDAvgTimeHandle(m_ZdcLEDAvgTime);

  SG::WriteDecorHandle<xAOD::ZdcModuleContainer,unsigned int> LEDTypeHandle(m_ZdcLEDType);

  ATH_MSG_DEBUG("Starting event processing for LED " << m_LEDNames[evtLEDType]);

  for (const auto zdcModule : moduleContainer)
  {
    ZDCLEDModuleResults results;
    if (zdcModule->zdcType() == 0) {
      results = processZDCModule(*zdcModule);
    }
    else if (zdcModule->zdcType() == 1 && zdcModule->zdcModule() == 4) {
      results = processRPDModule(*zdcModule);
    }

    ATH_MSG_DEBUG("Writing aux decors to module with side, module, channel = " << zdcModule->zdcSide() << ", " << zdcModule->zdcModule()
		<< ", " << zdcModule->zdcChannel());

    moduleLEDPresampleADCHandle(*zdcModule) = std::floor(results.getPresampleADC() + 1.0e-6);
    moduleLEDADCSumHandle(*zdcModule) = results.getADCSum();
    moduleLEDMaxADCHandle(*zdcModule) = results.getMaxADC();
    moduleLEDMaxSampleHandle(*zdcModule) = results.getMaxSample();
    moduleLEDAvgTimeHandle(*zdcModule) = results.getAvgTime();
  } 

  // Write the LED type to the moduleSum container keep event-level data
  //
  LEDTypeHandle(*moduleSumEventInfo_ptr) = evtLEDType;

  ATH_MSG_DEBUG("Finishing event processing");


  return StatusCode::SUCCESS;
  }

StatusCode ZdcLEDAnalysisTool::reprocessZdc()
{
  if (!m_init) {
    ATH_MSG_WARNING("Tool not initialized!");
    return StatusCode::FAILURE;
  }
  
  ATH_MSG_DEBUG ("Trying to retrieve " << m_zdcModuleContainerName);
  
  m_zdcModules = 0;
  ATH_CHECK(evtStore()->retrieve(m_zdcModules, m_zdcModuleContainerName));
  
  
  ATH_CHECK(recoZdcModules(*m_zdcModules, *m_zdcSums));
  
  return StatusCode::SUCCESS;
}

ZDCLEDModuleResults ZdcLEDAnalysisTool::processZDCModule(const xAOD::ZdcModule& module)
{
  ATH_MSG_DEBUG("Processing ZDC side, channel = " << module.zdcSide() << ", " << module.zdcModule());
  bool doLG = false;

  static const SG::ConstAccessor<std::vector<uint16_t> > g1dataAcc ("g1data");
  static const SG::ConstAccessor<std::vector<uint16_t> > g0dataAcc ("g0data");
  std::vector<uint16_t> HGSamples = g1dataAcc (module);
  std::vector<uint16_t> LGSamples = g0dataAcc (module);

  std::vector<uint16_t>::const_iterator maxIter = std::max_element(HGSamples.begin(), HGSamples.end());
  if (maxIter != HGSamples.end()) {
    if (*maxIter > m_HGADCOverflow) doLG = true;
  }

  if (doLG) {
    return processModuleData(module.zdcSide(),module.zdcModule(),LGSamples, m_sampleAnaStartZDC, m_sampleAnaEndZDC, m_ZdcLowGainScale);
  }
  else {
    return processModuleData(module.zdcSide(),module.zdcModule(),HGSamples, m_sampleAnaStartZDC, m_sampleAnaEndZDC, 1);
  }
}

ZDCLEDModuleResults ZdcLEDAnalysisTool::processRPDModule(const xAOD::ZdcModule& module)
{
  ATH_MSG_DEBUG("Processing RPD side, channel = " << module.zdcSide() << ", " << module.zdcChannel());
  static const SG::ConstAccessor<std::vector<uint16_t> > g0dataAcc ("g0data");
  return processModuleData(-1,-1,g0dataAcc (module), m_sampleAnaStartRPD, m_sampleAnaEndRPD, 1);
}

ZDCLEDModuleResults ZdcLEDAnalysisTool::processModuleData(int iside, int imod, const std::vector<unsigned short>& data,
							  unsigned int startSample, unsigned int endSample, float gainScale)
{

  int ADCSum = 0;
  int maxADCsub = -999;
  unsigned int maxSample = 0;
  float avgTime = 0.f;
  bool highgain = (gainScale<1.1); // HG is 1 and LG is 10
  
  if (startSample > m_numSamples || endSample > m_numSamples) {
    ATH_MSG_ERROR("Start or end sample number greater than number of samples");
    return ZDCLEDModuleResults();
  }

  int preFADC = data[m_preSample];
  
  for (unsigned int sample = startSample; sample <= endSample; sample++) {

    int FADCsub = data[sample] - preFADC;
    if (m_doFADCCorr && (iside!=-1) ) // iside==-1 dummy to indicate RPD and ensure no FADC correction
      {
	float corr = getAmplitudeCorrection(iside,imod,highgain,FADCsub);
	FADCsub *= corr;
      }
    
    float time = (sample + 0.5f)*m_deltaTSample;
    ADCSum += FADCsub;
    if (FADCsub > maxADCsub) {
      maxADCsub = FADCsub;
      maxSample = sample;
    }

    avgTime += time*FADCsub;
  }
  if (ADCSum!=0){
    avgTime /= ADCSum;
  } else {
    avgTime = 0.f; //used as default in the ZDCLEDModuleResults c'tor
  }

  return ZDCLEDModuleResults(preFADC, ADCSum*gainScale, maxADCsub*gainScale, maxSample, avgTime);
}

// FADC corrections

double ZdcLEDAnalysisTool::getAmplitudeCorrection(int iside, int imod, bool highGain, float fitAmp)
{
  double amplCorrFactor  = 1;

  double fadcCorr = highGain ? m_FADCCorrHG[iside][imod]->Interpolate(fitAmp) : m_FADCCorrLG[iside][imod]->Interpolate(fitAmp);
  amplCorrFactor *= fadcCorr;
    
  return amplCorrFactor;
}

void ZdcLEDAnalysisTool::setFADCCorrections(unsigned int runNumber)
{
  std::string filename;
  std::string runString;
  
  if (runNumber == 0) runString = "ZdcFADCCorr_" + m_configuration + "_default.root";
  else runString = ("ZdcFADCCorr_Run"+TString::Itoa(runNumber,10)+".root").Data();
  
  filename = PathResolverFindCalibFile("ZdcAnalysis/" + runString );

  if (filename.empty())
    {
      ATH_MSG_INFO("No FADC corrections file - disabling correction");
      m_doFADCCorr = false; // disable correction
      return;
    }
  
  ATH_MSG_INFO("Opening FADC corrections file " << filename);
  std::unique_ptr<TFile> fFADCCorr(TFile::Open(filename.c_str(), "READ"));
  
  if (!fFADCCorr->IsOpen()) {
    ATH_MSG_INFO ("setFADCCorrections: failed to open file: " << filename << ". Disabling correction.");
    m_doFADCCorr = false; // disable correctio
    return;
    //throw std::runtime_error ("ZdcAnalysisTool failed to open FADCCorrections file " + filename);
  }
  
  // Attempt to read histograms with corrections from file
  //
  bool readSuccess = true;
  std::array<std::array<std::unique_ptr<const TH1>, 4>, 2> histogramsHG;
  std::array<std::array<std::unique_ptr<const TH1>, 4>, 2> histogramsLG;
  
  for (size_t side : {0, 1}) {
    for (int module : {0, 1, 2, 3}) {
      std::string histNameHG = "ZDC_FADCCorr_s" + std::to_string(side) + "_m" + std::to_string(module)+"_HG";
      std::string histNameLG = "ZDC_FADCCorr_s" + std::to_string(side) + "_m" + std::to_string(module)+"_LG";

      ATH_MSG_DEBUG("setFADCCorrections: Searching for histograms HG and LG: " << histNameHG << ", " << histNameLG);
      
      TH1* histHG_ptr = static_cast<TH1*>(fFADCCorr->GetObjectChecked(histNameHG.c_str(), "TH1"));
      TH1* histLG_ptr = static_cast<TH1*>(fFADCCorr->GetObjectChecked(histNameLG.c_str(), "TH1"));

      if (!histHG_ptr || !histLG_ptr) {
	std::string errMsg = "setFADCCorrections: unable to read FADC correction histogram(s) ";
	if (!histHG_ptr) errMsg += histNameHG + " ";
	if (!histLG_ptr) errMsg += histNameLG;

	ATH_MSG_ERROR(errMsg);
	readSuccess = false;
	break;
      }
      else {
	//
	//  Check for valid range (Lion uses -0.5 to 4095.5)
	//
	
	if ( std::abs(histHG_ptr->GetXaxis()->GetXmin()+0.5) > 1e-3 || std::abs(histHG_ptr->GetXaxis()->GetXmax() - 4095.5) > 1e-3) {
	  ATH_MSG_ERROR("setFADCCorrections: invalid axis range for HG FADC corrections in histogram with name " << histNameHG);
	  readSuccess = false;
	  break;
	}
	if (std::abs(histLG_ptr->GetXaxis()->GetXmin()+0.5) > 1e-3 || std::abs(histLG_ptr->GetXaxis()->GetXmax() - 4095.5) > 1e-3) {
	  ATH_MSG_ERROR("setFADCCorrections: invalid axis range for HG FADC corrections in histogram with name " << histNameLG);
	  readSuccess = false;
	  break;
	}
	ATH_MSG_INFO("Configuring FADC histos for side " << side << " mod " << module);
	m_FADCCorrHG[side][module].reset(histHG_ptr);
	m_FADCCorrLG[side][module].reset(histLG_ptr);
	
      }
    }
  }
    
  fFADCCorr->Close();

  if (readSuccess) {
    ATH_MSG_INFO("Successfully configured FADC correction");
    m_doFADCCorr = true;
  }
  else {
    ATH_MSG_ERROR("setFADCCorrections: due to at least one error, FADC corrections are not implemented");
    m_doFADCCorr = false;
  }
  
  return;
}

  
} // namespace ZDC

