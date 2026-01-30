//Dear emacs, this is -*- C++ -*- 

/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARRECCONDITIONS_LARFEBRODMAPPINGALG_H
#define LARRECCONDITIONS_LARFEBRODMAPPINGALG_H

#include "AthenaBaseComps/AthCondAlgorithm.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/WriteCondHandleKey.h"

#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "LArRecConditions/LArFebRodMapping.h"
#include "PersistentDataModel/AthenaAttributeList.h"

class LArFebRodMappingAlg: public AthCondAlgorithm {

public:

  using AthCondAlgorithm::AthCondAlgorithm;

  virtual ~LArFebRodMappingAlg() = default;

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;


 private:
  SG::ReadCondHandleKey<AthenaAttributeList> m_readKey  {this,"ReadKey","/LAR/Identifier/FebRodMap"};
  SG::WriteCondHandleKey<LArFebRodMapping>   m_writeKey {this,"WriteKey","LArFebRodMap"};
};



#endif
