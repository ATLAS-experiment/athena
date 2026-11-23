/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/


#ifndef LARRECUTILS_LARFCALDEGCONDALG
#define LARRECUTILS_LARFCALDEGCONDALG


#include "StoreGate/ReadCondHandleKey.h"
#include "AthenaBaseComps/AthCondAlgorithm.h"
#include "LArRecConditions/LArFcalDegFactors.h"
#include "StoreGate/WriteCondHandleKey.h"

class CondAttrListCollection;

class LArFcalDegCondAlg : public AthCondAlgorithm  {
public:
  using AthCondAlgorithm::AthCondAlgorithm;

  virtual StatusCode initialize() override final;
  virtual StatusCode execute(const EventContext& ctx) const override final;
  
 private: 
  SG::ReadCondHandleKey<CondAttrListCollection> m_factorsKey{this, "keyFactors", "LArFcalDegData", "Key for the degradation factors"};

  SG::WriteCondHandleKey<LArFcalDegFactors> m_outputKey{this,"OutputKey","LArFcalDegMap","SG key of output LArFcalDegFactors object"};

};
#endif
