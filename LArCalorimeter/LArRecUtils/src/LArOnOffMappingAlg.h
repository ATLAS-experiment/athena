//Dear emacs, this is -*- C++ -*- 

/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARRECCONDITIONS_LARONOFFMAPPINGALG_H
#define LARRECCONDITIONS_LARONOFFMAPPINGALG_H

#include "AthenaBaseComps/AthCondAlgorithm.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/WriteCondHandleKey.h"

#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "LArCabling/LArOnOffIdMapping.h"
#include "PersistentDataModel/AthenaAttributeList.h"

class LArOnOffMappingAlg: public AthCondAlgorithm {

public:
  //Delegate constructor:
  using AthCondAlgorithm::AthCondAlgorithm;
  
  virtual ~LArOnOffMappingAlg() = default;

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;


 private:
  SG::ReadCondHandleKey<AthenaAttributeList> m_readKey {this,"ReadKey","/LAr/Identifier/OnOnffMap"};
  SG::WriteCondHandleKey<LArOnOffIdMapping>  m_writeKey{this,"WriteKey","LArOnOffIdMap"};
  Gaudi::Property<bool> m_isSuperCell{this,"isSuperCell",false};
};



#endif
