/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
// Dear emacs, this is -*-c++-*-
#ifndef LARCONDITIONSTEST_LARSCIDVSIDTEST_H
#define LARCONDITIONSTEST_LARSCIDVSIDTEST_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "Identifier/HWIdentifier.h"
#include "LArCabling/LArOnOffIdMapping.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "CaloDetDescr/ICaloSuperCellIDTool.h"
#include "LArIdentifier/LArOnlineID.h"
#include "LArIdentifier/LArOnline_SuperCellID.h"
#include "CaloIdentifier/CaloCell_ID.h"
#include "CaloIdentifier/CaloCell_SuperCell_ID.h"
#include "CaloDetDescr/CaloDetDescrManager.h"
#include "CaloGeoHelpers/CaloSampling.h"

class LArSCIdvsIdTest : public AthReentrantAlgorithm {
 public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  // standard algorithm methods
  StatusCode initialize();
  StatusCode execute(const EventContext& ctx) const;
  StatusCode finalize() { return StatusCode::SUCCESS; }

 private:
  SG::ReadCondHandleKey<LArOnOffIdMapping> m_cablingKey{this, "CablingKey", "LArOnOffIdMap", "SG Key of LArOnOffIdMapping object"};
  SG::ReadCondHandleKey<LArOnOffIdMapping> m_scCablingKey{this, "SCCablingKey", "LArOnOffIdMapSC", "SG Key of LArOnOffIdMapping object"};
  SG::ReadCondHandleKey<CaloSuperCellDetDescrManager> m_caloSuperCellMgrKey{this, "CaloSuperCellDetDescrManager", "CaloSuperCellDetDescrManager", "SG key of the resulting CaloSuperCellDetDescrManager" };
  SG::ReadCondHandleKey<CaloDetDescrManager> m_caloMgrKey{this, "CaloDetDescrManager", "CaloDetDescrManager", "SG Key for CaloDetDescrManager in the Condition Store"};

  ToolHandle<ICaloSuperCellIDTool> m_scidtool{this, "CaloSuperCellIDTool", "CaloSuperCellIDTool", "Offline / SuperCell ID mapping tool"};
  bool m_print;
  const LArOnlineID* m_onlineId=nullptr;
  const CaloCell_ID* m_caloCellId=nullptr;

  const LArOnline_SuperCellID* m_scOnlineId=nullptr;
  const CaloCell_SuperCell_ID* m_scCaloCellId=nullptr;

};

#endif