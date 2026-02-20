//Dear emacs, this is -*- C++ -*- 

/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARLATOMEMAPPINGALG_H
#define LARLATOMEMAPPINGALG_H

#include "AthenaBaseComps/AthCondAlgorithm.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/WriteCondHandleKey.h"

#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "LArCabling/LArLATOMEMapping.h"

/**
 * @brief class to fill SC mapping object from conditions DB
 */

class LArLATOMEMappingAlg: public AthCondAlgorithm {

public:
  //Delegate constructor:
  using AthCondAlgorithm::AthCondAlgorithm;
  
  virtual ~LArLATOMEMappingAlg() = default;

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;


 private:
  SG::ReadCondHandleKey<CondAttrListCollection> m_readKey {this,"ReadKey","/LAR/IdentifierSC/LatomeMapping"};
  SG::WriteCondHandleKey<LArLATOMEMapping>  m_writeKey{this,"WriteKey","LArLATOMEMap"};
  Gaudi::Property<bool> m_isSuperCell{this,"isSuperCell",false};

};

#endif
