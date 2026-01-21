/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ATHEXHIVE_CONDEX_CONDALGX_H
#define ATHEXHIVE_CONDEX_CONDALGX_H

#include "AthenaBaseComps/AthCondAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteCondHandleKey.h"

#include "AthExHive/CondDataObj.h"
#include "AthExHive/IASCIICondDbSvc.h"

#include "xAODEventInfo/EventInfo.h"
#include "GaudiKernel/ICondSvc.h"

class CondAlgX  :  public AthCondAlgorithm {
  
public:
  using AthCondAlgorithm::AthCondAlgorithm;

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;

private:
  
  SG::ReadHandleKey<xAOD::EventInfo> m_evt {this,"EvtInfo", "EventInfo", "EventInfo name"};

  SG::WriteCondHandleKey<CondDataObj> m_wchk {this, "Key_CH", "X2", "cond handle key"};

  Gaudi::Property<std::string> m_dbKey {this, "Key_DB", "X2", "explicit dbKey for cond handle"};

  ServiceHandle<IASCIICondDbSvc> m_cds{this, "ASCIICondDbSvc", "ASCIICondDbSvc", "Handle to the ASCII CondDb service"};
};

#endif
