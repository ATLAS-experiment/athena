/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
#ifndef TRIGGENERICALGS_L1CORRELATIONALG_H
#define TRIGGENERICALGS_L1CORRELATIONALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "AthenaMonitoringKernel/GenericMonitoringTool.h"
#include "ByteStreamCnvSvcBase/IROBDataProviderSvc.h"
#include "xAODTrigger/TrigCompositeContainer.h"
#include "TrigConfData/L1Menu.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/ReadHandleKey.h"

class L1CorrelationAlg : public AthReentrantAlgorithm {
 public:
  L1CorrelationAlg(const std::string& name, ISvcLocator* pSvcLocator);
  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;
  virtual StatusCode start() override;


 private:
  ServiceHandle<IROBDataProviderSvc> m_robDataProviderSvc{this, "ROBDataProviderSvc", "ROBDataProviderSvc", "Name of the ROB data provider"};
  SG::WriteHandleKey<xAOD::TrigCompositeContainer> m_trigCompositeKey{this, "TrigCompositeWriteHandleKey", "L1CorrelationTrigComposite", "Name of TrigComposite write handle"};
  SG::WriteDecorHandleKey<xAOD::TrigCompositeContainer> m_l1AKey{this, "l1AKey", "L1CorrelationTrigComposite.l1a_type", "L1 accept type"};
  SG::WriteDecorHandleKey<xAOD::TrigCompositeContainer> m_otherTypeKey{this, "otherTypeKey", "L1CorrelationTrigComposite.other_type", "Other type"};
  SG::WriteDecorHandleKey<xAOD::TrigCompositeContainer> m_beforeAfterKey{this, "beforeAfterKey", "L1CorrelationTrigComposite.beforeafterflag", "flag if BC is before or after nominal"};
  SG::WriteDecorHandleKey<xAOD::TrigCompositeContainer> m_otherTypeBeforeKey{this, "otherTypeBeforeKey", "L1CorrelationTrigComposite.other_type_before", "Trigger type (1-7) of the first fired BC before the central L1A BCID"};
  SG::WriteDecorHandleKey<xAOD::TrigCompositeContainer> m_otherTypeAfterKey{this, "otherTypeAfterKey", "L1CorrelationTrigComposite.other_type_after", "Trigger type (1-7) of the first fired BC after the central L1A BCID"};
  SG::WriteDecorHandleKey<xAOD::TrigCompositeContainer> m_beforeOffsetKey{this, "beforeOffsetKey", "L1CorrelationTrigComposite.offset_before", "Distance (in BCs) from the L1A to the first earlier neighbor trigger"};
  SG::WriteDecorHandleKey<xAOD::TrigCompositeContainer> m_afterOffsetKey{this, "afterOffsetKey", "L1CorrelationTrigComposite.offset_after", "Distance (in BCs) from the L1A to the first later neighbor trigger"};
  SG::WriteDecorHandleKey<xAOD::TrigCompositeContainer> m_passKey{this, "trigCompPassKey", "L1CorrelationTrigComposite.pass", "Key to indicate TrigComp is passed"};
  SG::ReadHandleKey<TrigConf::L1Menu> m_l1MenuKey{this, "L1MenuKey", "DetectorStore+L1TriggerMenu", "L1 Menu"};
  ToolHandle<GenericMonitoringTool> m_monTool{this, "MonTool", "", "Monitoring tool"};
  Gaudi::Property<bool> m_currentBCincl{this, "currentBCinclusive", true};
  Gaudi::Property<std::vector<std::string>> m_l1itemlist{this, "ItemList", {"L1_EM22VHI", "L1_J400", "L1_MU14FCH"}, "list of l1 items to be used"};


  std::vector<uint32_t> m_bitmasks;
  std::vector<uint32_t> m_bitmasks_ele;
  std::vector<uint32_t> m_bitmasks_mu;
  std::vector<uint32_t> m_bitmasks_jets;
};

#endif
