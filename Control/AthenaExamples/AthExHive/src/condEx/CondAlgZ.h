/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ATHEXHIVE_CONDEX_CONDALGZ_H
#define ATHEXHIVE_CONDEX_CONDALGZ_H

#include "AthenaBaseComps/AthAlgorithm.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/WriteCondHandleKey.h"

#include "AthExHive/CondDataObj.h"
#include "AthExHive/CondDataObjY.h"
#include "AthExHive/CondDataObjZ.h"

#include "xAODEventInfo/EventInfo.h"

class CondAlgZ  :  public AthAlgorithm {
  
public:
    
  CondAlgZ (const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~CondAlgZ() = default;
  
  virtual StatusCode initialize() override;
  virtual StatusCode execute() override;

private:
  
  SG::ReadCondHandleKey<CondDataObj> m_rchk1 {this, "Key_RCH1", "X2", "read cond handle key 1"};
  SG::ReadCondHandleKey<CondDataObjY> m_rchk2 {this, "Key_RCH2", "Y1", "read cond handle key 2"};
  SG::ReadCondHandleKey<CondDataObjY> m_rchk3 {this, "Key_RCH3", "Y2", "read cond handle key 3"};

  SG::WriteCondHandleKey<CondDataObjZ> m_wch {this, "Key_WCH", "Z1", "write cond handle key"};
};

#endif
