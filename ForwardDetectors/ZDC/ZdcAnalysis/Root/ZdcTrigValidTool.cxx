/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "ZdcAnalysis/ZdcTrigValidTool.h"
#include "PathResolver/PathResolver.h"
#include <fstream>
#include <bitset>
#include <stdexcept>
#include "AsgDataHandles/ReadDecorHandle.h"
#include "AsgDataHandles/WriteDecorHandle.h"

#include "xAODEventInfo/EventInfo.h"
#include "ZdcUtils/ZdcEventInfo.h"

using json = nlohmann::json;

//**********************************************************************
namespace ZDC{
ZdcTrigValidTool::ZdcTrigValidTool(const std::string& name)
 : asg::AsgTool(name), m_name(name) 
 {
  
#ifndef XAOD_STANDALONE
  declareInterface<IZdcAnalysisTool>(this);
#endif
  declareProperty("Message", m_msg = "");
  declareProperty("WriteAux", m_writeAux = true);
  declareProperty("AuxSuffix", m_auxSuffix = "");
}

//**********************************************************************

ZdcTrigValidTool::~ZdcTrigValidTool()
{
    ATH_MSG_DEBUG("Deleting ZdcTrigValidTool named " << m_name);
}

StatusCode ZdcTrigValidTool::initialize() {

  ATH_MSG_INFO("Initialising tool " << m_name);
  ATH_CHECK(m_trigDecTool.retrieve());
  ATH_MSG_INFO("TDT retrieved");
  
  // Find the full path to filename:
  std::string file = PathResolverFindCalibFile(m_lutFile);
  ATH_MSG_INFO("Reading file " << file);
  std::ifstream fin(file.c_str());
  if(!fin){
     ATH_MSG_ERROR("Can not read file: " << file);
     return StatusCode::FAILURE;
  }
  json data = json::parse(fin);
   
  // Obtain LUTs from Calibration Area
  // A data member to hold the side A LUT values
  std::array<unsigned int, 4096> sideALUT = data["LucrodHighGain"]["LUTs"]["sideA"];
  // A data member to hold the side C LUT values
  std::array<unsigned int, 4096> sideCLUT = data["LucrodHighGain"]["LUTs"]["sideC"];
  // A data member to hold the Combined LUT values
  std::array<unsigned int, 256> combLUT = data["LucrodHighGain"]["LUTs"]["comb"];
  
  //Construct Trigger Map
  m_triggerMap.insert({"L1_ZDC_BIT0",0});
  m_triggerMap.insert({"L1_ZDC_BIT1",1});
  m_triggerMap.insert({"L1_ZDC_BIT2",2});
   
  // Construct Simulation Objects
  m_modInputs_p = std::make_shared<ZDCTriggerSim::ModuleAmplInputsFloat>(ZDCTriggerSim::ModuleAmplInputsFloat());
  m_simTrig = std::make_shared<ZDCTriggerSimModuleAmpls>(ZDCTriggerSimModuleAmpls(sideALUT, sideCLUT, combLUT));
  ATH_MSG_INFO(m_name<<" Initialised");

  m_trigValStatus = "ZdcSums.TrigValStatus"+m_auxSuffix;
  ATH_CHECK(m_trigValStatus.initialize());

  ATH_CHECK( m_eventInfoKey.initialize());
  
  return StatusCode::SUCCESS;
  
}


StatusCode ZdcTrigValidTool::recoZdcModules(const xAOD::ZdcModuleContainer& moduleContainer, const xAOD::ZdcModuleContainer& moduleSumContainer)
{ 
  std::vector<float> moduleEnergy = {0., 0., 0., 0., 0., 0., 0., 0.};

  SG::ReadHandle<xAOD::EventInfo> eventInfo(m_eventInfoKey);
  if (!eventInfo.isValid()) return StatusCode::FAILURE;
  // Check for decoding errors and bail out if ZDC error found                                                               
  bool zdcErr = eventInfo->isEventFlagBitSet(xAOD::EventInfo::ForwardDet, ZdcEventInfo::ZDCDECODINGERROR );
  if (zdcErr)
    {
      ATH_MSG_WARNING("ZDC decoding error found - abandoning ZdcTrigValidTool!");
      return StatusCode::SUCCESS;
    }
  
  static SG::ConstAccessor<float> const zdcModuleAmpAcc("Amplitude" + m_auxSuffix);

  bool trigMatch = false;
  for (const auto zdcModule : moduleContainer) {

    if (zdcModule->zdcType() == 1) continue;
    
    // Side A
    if (zdcModule->zdcSide() > 0) {
      moduleEnergy.at(zdcModule->zdcModule()) = zdcModuleAmpAcc(*zdcModule);
    }
    
    // Side C
    if (zdcModule->zdcSide() < 0) {
      moduleEnergy.at(zdcModule->zdcModule() + 4) = zdcModuleAmpAcc(*zdcModule);
    }
  } 
  // Get Output as an integer (0-7)
  m_modInputs_p->setData(moduleEnergy);
   
  // call ZDCTriggerSim to actually get ZDC Bits
  unsigned int wordOut = m_simTrig->simLevel1Trig(ZDCTriggerSim::SimDataCPtr(m_modInputs_p));

  // convert int to bitset
  std::bitset<3> bin(wordOut);
  
  // get trigger decision tool
  const auto &trigDecTool = m_trigDecTool;
  unsigned int wordOutCTP = 0;
  
  
  // iterate through zdc bit output from CTP, validate that they match above bitset
  for (const auto &trig : m_triggerList)
    {

      if (m_triggerMap.find(trig) == m_triggerMap.end())
        continue;
      if (not trigDecTool->isPassed(trig, TrigDefs::Physics | TrigDefs::allowResurrectedDecision))
      {
        ATH_MSG_DEBUG("Chain " << trig << " is passed: NO");
        continue;
      }
      ATH_MSG_DEBUG("Chain " << trig << " is passed: YES");
      wordOutCTP += 1 << m_triggerMap[trig];
    }
    
  trigMatch = (wordOut == wordOutCTP) ? 1 : 0;
  SG::WriteDecorHandle<xAOD::ZdcModuleContainer,unsigned int> trigValStatus(m_trigValStatus);

  // write 1 if decision from ZDC firmware matches CTP, 0 otherwize  
  for(const auto zdc_sum : moduleSumContainer){
    if(m_writeAux) trigValStatus(*zdc_sum) = trigMatch;
  }
  ATH_MSG_DEBUG("ZDC Trigger Status: "  << trigMatch);

return StatusCode::SUCCESS;
}
} //namespace ZDC
