/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ATHEXHIVE_CONDEX_ALGC_H
#define ATHEXHIVE_CONDEX_ALGC_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadCondHandleKey.h"

#include "AthExHive/HiveDataObj.h"
#include "AthExHive/CondDataObj.h"

class AlgC  :  public AthReentrantAlgorithm {
  
public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;
  
private:
  
  SG::ReadHandleKey<HiveDataObj> m_rdh1 {this, "Key_R1", "a2", "read key 1"};
  
  SG::ReadCondHandleKey<CondDataObj> m_rch {this, "Key_CH", "X1", "cond read key"};

};
#endif
