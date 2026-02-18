/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ATHEXHIVE_ALGL3_H
#define ATHEXHIVE_ALGL3_H 1

#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/UpdateHandleKey.h"
#include "AthExHive/HiveDataObj.h"
#include "../HiveAlgBase.h"

#include <string>

class HiveAlgL3  :  public HiveAlgBase {
  
public:
  
  // Standard Algorithm Constructor:
  
  HiveAlgL3 (const std::string& name, ISvcLocator* pSvcLocator);
  ~HiveAlgL3();
  
  // Define the initialize, execute and finalize methods:
  
  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;
  
private:
  SG::UpdateHandleKey<HiveDataObj> m_udh1{this, "Key_U1", "l1", "update handle"};
};
#endif
