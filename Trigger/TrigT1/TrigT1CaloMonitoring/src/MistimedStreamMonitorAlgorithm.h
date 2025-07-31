/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/


#ifndef TRIGT1CALOMONITORING_MISTIMEDSTREAMMONITORALGORITHM
#define TRIGT1CALOMONITORING_MISTIMEDSTREAMMONITORALGORITHM

#include "AthenaMonitoring/AthMonitorAlgorithm.h"
#include "AthenaMonitoringKernel/Monitored.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadHandle.h"

#include "TrigT1CaloToolInterfaces/IL1TriggerTowerToolRun3.h"
#include "TrigT1Interfaces/TrigT1CaloDefs.h"
#include "TrigT1CaloCalibConditions/L1CaloRunParametersContainer.h"  
#include "TrigT1CaloCalibConditions/L1CaloReadoutConfigContainerJSON.h"
#include "xAODTrigL1Calo/TriggerTowerContainer.h"
#include "xAODTrigL1Calo/CPMTowerContainer.h" 
#include "xAODTrigL1Calo/JetElementContainer.h"
#include "CaloEvent/CaloCellContainer.h"

#include "xAODTrigL1Calo/eFexTowerContainer.h"
#include "xAODTrigL1Calo/eFexTowerAuxContainer.h"
#include "xAODTrigL1Calo/eFexTower.h"
#include "xAODTrigger/eFexEMRoIContainer.h"
#include "xAODTrigger/eFexEMRoI.h"
#include "xAODTrigger/eFexTauRoIContainer.h"
#include "xAODTrigger/eFexTauRoI.h"

#include "xAODTrigL1Calo/jFexTowerContainer.h"
#include "xAODTrigL1Calo/jFexTowerAuxContainer.h"
#include "xAODTrigL1Calo/jFexTower.h"
#include "CaloEvent/CaloCellContainer.h"
#include "xAODTrigger/jFexLRJetRoIContainer.h"
#include "xAODTrigger/jFexLRJetRoIAuxContainer.h"
#include "xAODTrigger/jFexLRJetRoI.h"
#include "xAODTrigger/jFexSRJetRoIContainer.h"
#include "xAODTrigger/jFexSRJetRoIAuxContainer.h"
#include "xAODTrigger/jFexSRJetRoI.h"
#include "xAODTrigger/jFexTauRoIContainer.h"
#include "xAODTrigger/jFexTauRoIAuxContainer.h"
#include "xAODTrigger/jFexTauRoI.h"

#include "xAODTrigger/gFexJetRoI.h"
#include "xAODTrigger/gFexJetRoIContainer.h"

#include <TH2.h>

// Trigger include(s):
#include "TrigDecisionTool/TrigDecisionTool.h"
#include "TrigT1Interfaces/TrigT1StoreGateKeys.h"
#include "TrigT1Result/CTP_RDO.h"
#include "TrigT1Result/CTP_Decoder.h"


class  MistimedStreamMonitorAlgorithm : public AthMonitorAlgorithm {
public: MistimedStreamMonitorAlgorithm( const std::string& name, ISvcLocator* pSvcLocator );
  virtual ~MistimedStreamMonitorAlgorithm()=default;
  virtual StatusCode initialize() override;
  virtual StatusCode fillHistograms( const EventContext& ctx ) const override;
  
  /// Struct to contain PPM trigger tower info
  struct MonitorTT { 
    const xAOD::TriggerTower* tower = nullptr;
    double phiScaled = 0; /// phi for 2d maps with integer bins (taking into account granularity in eta)
    double phi1d = 0;     /// phi for 1d phi distributions (taking into account granularity in eta)
  };

  /// Struct to contain CPM tower info
  struct MonitorCPM { 
    const xAOD::CPMTower* tower = nullptr;
    std::vector<double>  phiScaled;
    double etaScaled = 0; /// phi for 2d maps with integer bins (taking into account granularity in eta)
    double phi1d = 0;     /// phi for 1d phi distributions (taking into account granularity in eta) 
  };

  /// Struct to contain JE info
  struct MonitorJE { 
    const xAOD::JetElement* element = nullptr;
    std::vector<double>  phiScaled;
    std::vector<double> etaScaled;
    double phi1d = 0;     /// phi for 1d phi distributions (taking into account granularity in eta) 
  };


private:
  
  StringProperty m_packageName{this,"PackageName","MistimedStreamMonitor","group name for histogramming"};

  bool pulseQuality(const std::vector<uint16_t>& ttPulse, int peakSlice) const;
 

  // TrigDecisionTool
  PublicToolHandle< Trig::TrigDecisionTool > m_trigDec{this, "TriggerDecisionTool", "Trig::TrigDecisionTool/TrigDecisionTool", ""};

  SG::ReadHandleKey<CTP_RDO> m_ctpRdoReadKey {this, "CTPRDOReadKey", LVL1CTP::DEFAULT_RDOOutputLocation,"Read handle key to CTP_RDO for conversion to ByteStream"};
  
  /// container keys including steering parameter and description
  SG::ReadHandleKey<xAOD::TriggerTowerContainer> m_xAODTriggerTowerContainerName{this, "BS_xAODTriggerTowerContainer",LVL1::TrigT1CaloDefs::xAODTriggerTowerLocation,"Trigger Tower Container"};

  SG::ReadHandleKey<xAOD::CPMTowerContainer> m_cpmTowerLocation{this, "CPMTowerLocation", LVL1::TrigT1CaloDefs::CPMTowerLocation, "CPM container"};
  
  SG::ReadHandleKey<xAOD::JetElementContainer> m_jetElementLocation{this, "JetElementLocation", LVL1::TrigT1CaloDefs::JetElementLocation, "Jet Element Container"};
  
  SG::ReadHandleKey<xAOD::eFexEMRoIContainer> m_eFexEMContainerKey{this,"eFexEMContainer","L1_eEMxRoI","SG key of the input eFex RoI container"};
  SG::ReadHandleKey<xAOD::eFexTauRoIContainer> m_eFexTauContainerKey{this,"eFexTauContainer","L1_eTauxRoI","SG key of the input eFex Tau RoI container"};
  SG::ReadHandleKey<xAOD::eFexEMRoIContainer> m_eFexEMOutContainerKey{this,"eFexEMOutContainer","L1_eEMxRoIOutOfTime","SG key of the input eFex RoI container"};  

  // container keys for jFex
  SG::ReadHandleKey< xAOD::jFexLRJetRoIContainer > m_jFexLRJetContainerKey {this,"jFexLRJetRoIContainer","L1_jFexLRJetRoI","SG key of the input jFex LR Jet Roi container"};
  SG::ReadHandleKey< xAOD::jFexSRJetRoIContainer > m_jFexSRJetContainerKey {this,"jFexSRJetRoIContainer","L1_jFexSRJetRoI","SG key of the input jFex SR Jet Roi container"};
  SG::ReadHandleKey< xAOD::jFexTauRoIContainer   > m_jFexTauContainerKey   {this,"jFexTauRoIContainer"  ,"L1_jFexTauRoI"  ,"SG key of the input jFex Tau Roi container"};
  
  SG::ReadHandleKey<xAOD::jFexTowerContainer> m_jFexDataTowerKey    {this, "jFexDataTower","L1_jFexDataTowers","SG key of the input jFex Tower container"};
  SG::ReadHandleKey<xAOD::jFexTowerContainer> m_EmulTowerKey {this, "InputEmulatedTowers", "L1_jFexEmulatedTowers", "SG key of the emulated jFex Tower container"};

  // Define read handles for gFex
  SG::ReadHandleKeyArray<xAOD::gFexJetRoIContainer> m_gFexJetTobKeyList{this,"gFexJetTobKeyList",{"L1_gFexLRJetRoI", "L1_gFexSRJetRoI"},"Array of gFEX jet ReadHandleKeys to fill histograms for"};

  ToolHandle<LVL1::IL1TriggerTowerToolRun3> m_ttTool{this,"L1TriggerTowerToolRun3", "LVL1::L1TriggerTowerToolRun3/L1TriggerTowerToolRun3", "L1TriggerTowerToolRun3"};

  // Properties
  Gaudi::Property<double> m_phiScaleTT{this, "phiScaleTT", 32./M_PI, "Scale factor to convert trigger tower phi to integer binning"};
  Gaudi::Property<bool> m_uselegacy{this, "UseLegacy", false, "Use legacy system" };
  Gaudi::Property<bool> m_usephaseI{this, "UsePhase1", false, "Use phaseI system" };
  
  // L1Calo Conditions 
  SG::ReadCondHandleKey<L1CaloRunParametersContainer>  m_runParametersContainer{ this, "InputKeyRunParameters", "L1CaloRunParametersContainer"};
  SG::ReadCondHandleKey<L1CaloReadoutConfigContainerJSON>  m_readoutConfigContainerJSON{ this, "InputKeyReadoutConfig", "L1CaloReadoutConfigContainerJSON"};

  /// Helper functions
  StatusCode makeTowerPPM( const xAOD::TriggerTower* tt, 
                               std::vector<MonitorTT> &vecMonTT) const;

 
  StatusCode fillPPMEtaPhi( MonitorTT &monTT, 
                              const std::string& groupName, 
                              const std::string& weightName,
                              double weight) const;

  StatusCode makeTowerCPM( const xAOD::CPMTower* cpm, 
			   std::vector<MonitorCPM> &vecMonCPM) const;

  StatusCode makeTowerJE( const xAOD::JetElement* je, 
			   std::vector<MonitorJE> &vecMonJE) const;

  //Control maximum number of histograms per job
  Gaudi::Property<int> m_maxEvents{this,"MaxEvents",15};  
  
  // Event number counter
  mutable std::atomic<int>  m_eventCounter{0};
  // count number of error events per lumiblock across threads for each type of error
  mutable std::mutex m_mutex{};
  mutable std::map<uint32_t, int> m_event_counter ATLAS_THREAD_SAFE; 

  //  Bin Names for CutFlow
  enum cutFlow {
    All,
    UnsuitableReadout,
    HLT_mistimemonj400,
    L1_Trigger,
    lateTT,
    InTime,
    EtaPhiOverlap
  };
   
};
#endif
