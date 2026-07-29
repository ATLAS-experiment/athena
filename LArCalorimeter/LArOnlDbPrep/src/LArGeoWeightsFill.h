/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARGEOWEIGHTSFILL_H
#define LARGEOWEIGHTSFILL_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "LArCabling/LArOnOffIdMapping.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "CaloTriggerTool/CaloTriggerTowerService.h"
#include "CaloDetDescr/CaloDetDescrManager.h"
class StoreGateSvc;
class LArOnlineID;

class LArGeoWeightsFill : public AthReentrantAlgorithm {
 public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;
  ~LArGeoWeightsFill();
  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext&) const override {return StatusCode::SUCCESS;}
  virtual StatusCode stop() override;

 private:
  SG::ReadCondHandleKey<LArOnOffIdMapping> m_cablingKey{this, "OnOffMap", "LArOnOffIdMap", "SG key for mapping object"};
  SG::ReadCondHandleKey<CaloDetDescrManager> m_caloMgrKey{this,"CaloDetDescrManager","CaloDetDescrManager","SG Key for CaloDetDescrManager in the Condition Store" };

  const LArOnlineID* m_onlineID = nullptr;

  StringProperty  m_key  { this, "Key", "GeoWeights" };
  BooleanProperty m_fill { this, "Fill", true };
  BooleanProperty m_dump { this, "Dump", false };
  StringProperty  m_outFileName { this, "OutFile", "out.txt" };

  ToolHandle < CaloTriggerTowerService > m_ttService { "CaloTriggerTowerService" };
};

#endif
