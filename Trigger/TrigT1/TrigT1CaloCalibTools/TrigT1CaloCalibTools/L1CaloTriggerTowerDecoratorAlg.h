// Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGGER_TRIGT1_TRIGT1CALOXAODCALIBTOOLS_DECORATETRIGGERTOWERSALG_H
#define TRIGGER_TRIGT1_TRIGT1CALOXAODCALIBTOOLS_DECORATETRIGGERTOWERSALG_H

// Gaudi/Athena include(s):
#include "xAODTrigL1Calo/TriggerTowerContainer.h"
#include "AthenaBaseComps/AthAlgorithm.h"
#include "AsgTools/ToolHandle.h"

#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandle.h"

// Local include(s):
#include "TrigT1CaloCalibToolInterfaces/IL1CaloxAODOfflineTriggerTowerTools.h"
#include "TrigT1Interfaces/TrigT1CaloDefs.h"

#include <memory>

namespace LVL1 {
class L1CaloTriggerTowerDecoratorAlg : public AthAlgorithm {
 public:
  L1CaloTriggerTowerDecoratorAlg(const std::string& name, ISvcLocator* svcLoc);

  /// Function initialising the algorithm
  virtual StatusCode initialize() override;
  /// Function executing the algorithm
  virtual StatusCode execute() override;

 private:
  SG::ReadHandleKey<xAOD::TriggerTowerContainer> m_triggerTowerContainerKey
  { this, "sgKey_TriggerTowers", LVL1::TrigT1CaloDefs::xAODTriggerTowerLocation, "" };
  std::string m_sgKey_TriggerTowers;

  ToolHandle<LVL1::IL1CaloxAODOfflineTriggerTowerTools> m_ttTools;

  SG::WriteDecorHandleKey<xAOD::TriggerTowerContainer> m_caloCellEnergyKey{ this, "caloCellEnergyKey", "", "" };
  SG::WriteDecorHandleKey<xAOD::TriggerTowerContainer> m_caloCellETKey{ this, "caloCellETKey", "", "" };

  SG::WriteDecorHandleKey<xAOD::TriggerTowerContainer> m_caloCellsQualityKey{ this, "caloCellsQualityKey", m_triggerTowerContainerKey.key()+".CaloCellQuality", "" };
  SG::WriteDecorHandleKey<xAOD::TriggerTowerContainer> m_caloCellEnergyByLayerKey{this, "caloCellEnergyByLayerKey", m_triggerTowerContainerKey.key()+".CaloCellEnergyByLayer",""};
  SG::WriteDecorHandleKey<xAOD::TriggerTowerContainer> m_caloCellETByLayerKey{ this, "caloCellETByLayerKey", m_triggerTowerContainerKey.key()+".CaloCellETByLayer", "" };

  SG::WriteDecorHandleKey<xAOD::TriggerTowerContainer> m_caloCellEnergyByLayerByReceiverKey{ this, "caloCellEnergyByLayerByReceiverKey", "", ""};
  SG::WriteDecorHandleKey<xAOD::TriggerTowerContainer> m_caloCellETByLayerByReceiverKey{ this, "caloCellETByLayerByReceiverKey", "", "" };
};
}
#endif
