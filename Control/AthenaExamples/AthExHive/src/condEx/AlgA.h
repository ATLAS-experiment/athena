/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ATHEXHIVE_CONDEX_ALGA_H
#define ATHEXHIVE_CONDEX_ALGA_H

#include "AthenaBaseComps/AthAlgorithm.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/ReadHandleKey.h"
#include "AthExHive/HiveDataObj.h"

#include "xAODEventInfo/EventInfo.h"

class AlgA  :  public AthAlgorithm {
  
public:
  
  AlgA (const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~AlgA () = default;

  virtual bool isClonable() const override { return true; }
  
  virtual StatusCode initialize() override;
  virtual StatusCode execute() override;
  
private:

  SG::WriteHandleKey<HiveDataObj> m_wrh1 {this, "Key_W1", "a1", "write key 1"};
  SG::WriteHandleKey<HiveDataObj> m_wrh2 {this, "Key_W2", "a2", "write key 2"};
  SG::ReadHandleKey<xAOD::EventInfo>    m_evt  {this, "EvtInfo", "EventInfo", "EventInfo name"};
};
#endif
