/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef TRIGT1CALOMONITORING_JETEFFICIENCYMONITORALGORITHM_H
#define TRIGT1CALOMONITORING_JETEFFICIENCYMONITORALGORITHM_H

#include "AthenaMonitoring/AthMonitorAlgorithm.h"
#include "AthenaMonitoringKernel/Monitored.h"

#include "AthenaKernel/Units.h"
#include "FourMomUtils/P4Helpers.h"

#include "StoreGate/ReadHandleKey.h"
#include "xAODJet/JetContainer.h"
#include "xAODTrigger/gFexJetRoI.h"
#include "xAODTrigger/gFexJetRoIContainer.h"
#include "xAODTrigger/gFexGlobalRoI.h"
#include "xAODTrigger/gFexGlobalRoIContainer.h"


// #include "TrigDecisionTool/TrigDecisionTool.h"
//#include "TrigT1Interfaces/TrigT1CaloDefs.h"

class JetEfficiencyMonitorAlgorithm : public AthMonitorAlgorithm {
public:JetEfficiencyMonitorAlgorithm( const std::string& name, ISvcLocator* pSvcLocator );
  virtual ~JetEfficiencyMonitorAlgorithm()=default;
  virtual StatusCode initialize() override;
  virtual StatusCode fillHistograms( const EventContext& ctx ) const override;
private:
  StringProperty m_packageName{this,"PackageName","JetEfficiencyMonitor","group name for histograming"};

  StringProperty m_bootstrap_reference_trigger{this,"BootstrapReferenceTrigger","L1_J15","the bootstrapping trigger"};
  Gaudi::Property<std::vector<std::string>> m_muon_reference_triggers{this,"MuonReferenceTriggers",{},"the muon refernce triggers"};
  Gaudi::Property<std::vector<std::string>> m_HLTrandom_reference_triggers{this,"HLTRandomReferenceTriggers",{},"the random refernce trigger"};
  Gaudi::Property<bool> m_passedb4Prescale{this,"PassedBeforePrescale",0,"boolean of if we want to measure the efficiency based on passed before prescale"};

  Gaudi::Property<std::vector<std::string>> m_SmallRadiusJetTriggers_phase1{this,"SmallRadiusJetTriggers_phase1",{},"Vector of all Small radius triggers"};
  Gaudi::Property<std::vector<std::string>> m_LargeRadiusJetTriggers_phase1{this,"LargeRadiusJetTriggers_phase1",{},"Vector of all Large radius triggers"};
 
  Gaudi::Property<std::vector<std::string>> m_SmallRadiusJetTriggers_gFEX{this,"SmallRadiusJetTriggers_gFEX",{},"Vector of all small radius gFEX triggers, whose trigger decision is emulated using gFEX TOB values"};
  Gaudi::Property<std::vector<std::string>> m_LargeRadiusJetTriggers_gFEX{this,"LargeRadiusJetTriggers_gFEX",{},"Vector of all large radius gFEX triggers, whose trigger decision is emulated using gFEX TOB values"};


  
  // container keys including steering parameter and description
  SG::ReadHandleKey<xAOD::JetContainer> m_jetKey{ this, "JetKey" , "AntiKt4EMPFlowJets", ""}; //offline jets
  SG::ReadHandleKey<xAOD::JetContainer> m_LRjetKey{ this, "LRJetKey" , "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets", ""}; //offline LR jets   
  SG::ReadHandleKey<xAOD::gFexJetRoIContainer> m_gFexSRJetContainerKey{ this, "mygFexSRJetRoIContainer" , "L1_gFexSRJetRoI" , ""}; //gfex SR jets
  SG::ReadHandleKey<xAOD::gFexJetRoIContainer> m_gFexLRJetContainerKey{ this, "mygFexLRJetRoIContainer" , "L1_gFexLRJetRoI" , ""}; //gfex LR jets
};
#endif
