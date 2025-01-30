/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ATHEXHIVE_CONDEX_ALGD_H
#define ATHEXHIVE_CONDEX_ALGD_H

#include "AthenaBaseComps/AthAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadCondHandleKey.h"

#include "AthExHive/HiveDataObj.h"
#include "AthExHive/CondDataObj.h"

class AlgD  :  public AthAlgorithm {
  
public:
    
  AlgD (const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~AlgD() = default;
  
  virtual bool isClonable() const override { return true; }

  virtual StatusCode initialize() override;
  virtual StatusCode execute() override;
  
private:
  
  SG::ReadHandleKey<HiveDataObj> m_rdh1 {this, "Key_R1", "a1", "read key 1"};
  
  SG::ReadCondHandleKey<CondDataObj> m_rch1 {this, "Key_CH1", "X1", "cond read key1"};
  SG::ReadCondHandleKey<CondDataObj> m_rch2 {this, "Key_CH2", "X2", "cond read key2"};

};
#endif
