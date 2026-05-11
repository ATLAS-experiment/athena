/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ATHEXHIVE_CONDEX_ALGE_H
#define ATHEXHIVE_CONDEX_ALGE_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadCondHandleKey.h"

#include "AthExHive/HiveDataObj.h"
#include "AthExHive/CondDataObj.h"
#include "AthExHive/CondDataObjY.h"
#include "AthExHive/CondDataObjZ.h"

class AlgE  :  public AthReentrantAlgorithm {
  
public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;
  
private:
  
  SG::ReadHandleKey<HiveDataObj> m_rdh1 {this, "Key_R1", "a1", "read key 1"};
  
  SG::ReadCondHandleKey<CondDataObj> m_rch1 {this, "Key_CH1", "X2", "cond read key1"};
  SG::ReadCondHandleKey<CondDataObjY> m_rch2 {this, "Key_CH2", "Y1", "cond read key2"};
  SG::ReadCondHandleKey<CondDataObjY> m_rch3 {this, "Key_CH3", "Y2", "cond read key3"};

  SG::ReadCondHandleKey<CondDataObjZ> m_rch4 {this, "Key_CH4", "Z1", "cond read key4"};

  
};
#endif
