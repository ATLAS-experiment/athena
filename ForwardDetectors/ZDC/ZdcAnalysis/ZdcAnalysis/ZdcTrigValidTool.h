/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ZDCTRIGVALID_ZDCTRIGVALIDTOOL_H
#define ZDCTRIGVALID_ZDCTRIGVALIDTOOL_H

// Matthew Hoppesch.
// July 2023
//
// This is a simple tool to compare L1 ZDC decions made in CTP to those made using ZDC firmware
// Used for validation of ZDC firmware in Run3, 
// WriteAux Property Determines if Trigger Validation Status is written to xAOD::ZdcModuleContainer

#include <xAODForward/ZdcModuleAuxContainer.h>
#include "ZdcAnalysis/IZdcAnalysisTool.h"
#include "AsgTools/AsgTool.h"
#include "TrigDecisionTool/TrigDecisionTool.h"
#include "ZdcUtils/ZDCTriggerSim.h"
#include "xAODEventInfo/EventInfo.h"

#include "nlohmann/json.hpp"

#include "AsgDataHandles/ReadDecorHandleKey.h"
#include "AsgDataHandles/WriteDecorHandleKey.h"


namespace ZDC {
class ATLAS_NOT_THREAD_SAFE ZdcTrigValidTool : public virtual IZdcAnalysisTool, public asg::AsgTool 
{
  ASG_TOOL_CLASS(ZdcTrigValidTool, ZDC::IZdcAnalysisTool)

 public:
  ZdcTrigValidTool(const std::string& name);
  virtual ~ZdcTrigValidTool() override;
  virtual StatusCode initialize() override;

  virtual StatusCode recoZdcModules(const xAOD::ZdcModuleContainer& moduleContainer, const xAOD::ZdcModuleContainer& moduleSumContainer) override;
  virtual StatusCode reprocessZdc() override {return StatusCode::SUCCESS;}

 protected:

#ifdef XAOD_STANDALONE
  ToolHandle<Trig::TrigDecisionTool> m_trigDecTool {this, "TrigDecisionTool",""}; ///< Tool to tell whether a specific trigger is passed
#else
  PublicToolHandle<Trig::TrigDecisionTool> m_trigDecTool {this, "TrigDecisionTool",""}; ///< Tool to tell whether a specific trigger is passed
#endif

 private:  
  /* properties */
  Gaudi::Property<std::vector<std::string>> m_triggerList{
      this, "triggerList", {}, "Add triggers to this to be monitored"};
  Gaudi::Property<std::string> m_lutFile{this, "filepath_LUT", "TrigT1ZDC/zdcRun3T1LUT_v1_30_05_2023.json", "path to LUT file"};

  /** A data member to hold the ZDCTrigger Object that stores input floats: shared ptr to ensure cleanup */
  std::shared_ptr<ZDCTriggerSim::ModuleAmplInputsFloat> m_modInputs_p;

  /** A data member to hold the ZDCTrigger Object that computes the LUT logic: shared ptr to ensure cleanup */
  std::shared_ptr<ZDCTriggerSimModuleAmpls> m_simTrig;
  
  std::string m_msg;
  std::map<std::string, unsigned int > m_triggerMap;
  std::string m_auxSuffix;
  std::string m_name;
  bool m_writeAux;

  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_trigValStatus{this, "TrigValStatus", "","Trigger validation status"};

  SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey {
    this, "EventInfoKey", "EventInfo",
    "Location of the event info."};

};
}
#endif
