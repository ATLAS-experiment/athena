/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef TRIGT1CALOMONITORING_L1CALOCTPMONITORALGORITHM_H
#define TRIGT1CALOMONITORING_L1CALOCTPMONITORALGORITHM_H

#include "AthenaMonitoring/AthMonitorAlgorithm.h"
#include "AthenaMonitoringKernel/Monitored.h"
#include "StoreGate/ReadHandleKey.h"

#include "xAODTrigL1Calo/CMXCPHitsContainer.h" 
#include "xAODTrigL1Calo/CMXJetHitsContainer.h"
#include "xAODTrigL1Calo/CMXEtSumsContainer.h" 

#include "TrigT1Result/CTP_RDO.h"
#include "TrigT1Result/CTP_Decoder.h"
#include "TrigT1Interfaces/FrontPanelCTP.h"
#include "TrigT1Interfaces/TrigT1StoreGateKeys.h"

#include "TrigConfInterfaces/ILVL1ConfigSvc.h"
#include "TrigT1CaloMonitoringTools/ITrigT1CaloMonErrorTool.h"
#include "TrigT1CaloMonitoringTools/TrigT1CaloLWHistogramTool.h"
#include "TrigT1Interfaces/TrigT1CaloDefs.h"
#include "TrigConfL1Data/CTPConfig.h"
#include "TrigConfL1Data/L1DataDef.h"
#include "TrigConfL1Data/Menu.h"
#include "TrigConfL1Data/TIP.h"
#include "TrigConfL1Data/TriggerThreshold.h"
#include "TrigConfData/L1Menu.h"
#include "TrigConfL1Data/PIT.h"
#include "TrigConfInterfaces/ITrigConfigSvc.h"


/** Monitoring of L1Calo --> CTP transmission
 * Compares L1Calo data with CTP TIP data.
 @authors Rajat Gupta
*/

class L1CaloCTPMonitorAlgorithm : public AthMonitorAlgorithm {
public:L1CaloCTPMonitorAlgorithm( const std::string& name, ISvcLocator* pSvcLocator );
  virtual ~L1CaloCTPMonitorAlgorithm()=default;
  virtual StatusCode initialize() override;
  virtual StatusCode fillHistograms( const EventContext& ctx ) const override;

private:

  // to deal with L1 menu
  ServiceHandle<TrigConf::ITrigConfigSvc> m_configSvc{this, "TrigConfigSvc", "TrigConf::xAODConfigSvc/xAODConfigSvc"};
  //ServiceHandle<TrigConf::ITrigConfigSvc> m_configSvc{this, "TrigConfigSvc", "TrigConf::TrigConfigSvc/TrigConfigSvc"};
  const TrigConf::L1Menu* getL1Menu(const EventContext& ctx) const;
  
  /// Hit types for binning
  enum L1CaloCTPHitTypes { EM1Type, EM2Type,                                              // EM1, EM2 cables 
                           Tau1Type, Tau2Type,                                            // TAU1, TAU2 cables
                           Jet3BitType, Jet2BitType,                                      // JET1, JET2 cables
                           TEFullEtaType, XEFullEtaType, XSType,                          // EN1 cable
                           TERestrictedEtaType, XERestrictedEtaType, NumberOfHitTypes };  // EN2 cable
  
  /// Compare L1Calo hits with corresponding TIP hits
  void compare(const CTP_BC& bunch, int hits, int totalBits, int offset, L1CaloCTPHitTypes type, const EventContext& ctx) const;

  /// Bin labels for summary plots
  //void setLabels(LWHist* hist, bool xAxis = true);

  StringProperty m_packageName{this,"PackageName","L1CaloCTPMonitor","group name for histograming"};

  // container keys including steering parameter and description
  SG::ReadHandleKey<CTP_RDO> m_ctpRdoKey {this, "CTPRDOLocation", LVL1CTP::DEFAULT_RDOOutputLocation,"Key of the CTP RDO object"};
  SG::ReadHandleKey<xAOD::CMXJetHitsContainer> m_cmxJetHitsLocation {this, "CMXJetHitsLocation", LVL1::TrigT1CaloDefs::CMXJetHitsLocation, "CMXJetHits container"};
  SG::ReadHandleKey<xAOD::CMXEtSumsContainer> m_cmxEtSumsLocation {this, "CMXEtSumsLocation", LVL1::TrigT1CaloDefs::CMXEtSumsLocation, "CMXEtSums container"};
  SG::ReadHandleKey<xAOD::CMXCPHitsContainer> m_cmxCpHitsLocation {this, "CMXCPHitsLocation", LVL1::TrigT1CaloDefs::CMXCPHitsLocation, "CMXCPHits container"};

  SG::ReadHandleKey<TrigConf::L1Menu> m_L1MenuKey  { this, "L1TriggerMenu", "DetectorStore+L1TriggerMenu", "L1 Menu" };

  // Event veto error tool 
  ToolHandle<LVL1::ITrigT1CaloMonErrorTool>    m_errorTool;  

};
#endif
