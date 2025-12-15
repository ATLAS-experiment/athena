/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ATHEXHIVE_CONDEX_CONDALGY_H
#define ATHEXHIVE_CONDEX_CONDALGY_H

#include "AthenaBaseComps/AthCondAlgorithm.h"
#include "GaudiKernel/ServiceHandle.h"
#include "StoreGate/WriteCondHandleKey.h"

#include "AthExHive/CondDataObjY.h"
#include "AthExHive/IASCIICondDbSvc.h"


class CondAlgY  :  public AthCondAlgorithm {

public:
  using AthCondAlgorithm::AthCondAlgorithm;

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;

private:
  
  SG::WriteCondHandleKey<CondDataObjY> m_wch1 {this, "Key_CH1", "Y1", "cond handle key 1"};
  SG::WriteCondHandleKey<CondDataObjY> m_wch2 {this, "Key_CH2", "Y2", "cond handle key 2"};

  Gaudi::Property<std::string> m_dbk1 {this, "Key_DB1", "Y1", "explicit dbKey for cond handle 1"};
  Gaudi::Property<std::string> m_dbk2 {this, "Key_DB2", "Y2", "explicit dbKey for cond handle 2"};

  ServiceHandle<IASCIICondDbSvc> m_cds{this, "ASCIICondDbSvc", "ASCIICondDbSvc", "Handle to the ASCII CondDb service"};

};

#endif
