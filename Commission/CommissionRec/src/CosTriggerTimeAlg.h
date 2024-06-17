/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef COMMISSIONINGREC_COSTRIGTIMEALG_H
#define COMMISSIONINGREC_COSTRIGTIMEALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "CommissionEvent/CosTrigTime.h"
#include "LArSimEvent/LArHitContainer.h"
#include "StoreGate/ReadHandleKeyArray.h"
#include "StoreGate/WriteHandleKey.h"

class CosTriggerTimeAlg: public AthReentrantAlgorithm {

public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;
  
  virtual ~CosTriggerTimeAlg()=default;
  
  virtual StatusCode initialize();

  virtual StatusCode execute(const EventContext& cxt) const;
  
 private: 
  SG::ReadHandleKeyArray<LArHitContainer> m_larHitKeys{this,"LArHitKeys",{"LArHitEMB","LArHitEMEC","LArHitHEC","LArHitFCAL"},"input keys"};
  SG::WriteHandleKey<CosTrigTime> m_timeKey{this,"CosTimeKey","CosTrigTime"};
 };


#endif 
