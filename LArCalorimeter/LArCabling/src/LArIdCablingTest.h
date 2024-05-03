//Dear emacs, this is -*-c++-*-
/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/


#ifndef LARIDCABLINGTEST_H
#define LARIDCABLINGTEST_H

#include "AthenaBaseComps/AthAlgorithm.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "LArCabling/LArOnOffIdMapping.h"

class LArIdCablingTest : public AthAlgorithm 
{
public:

  using AthAlgorithm::AthAlgorithm;
  
  ~LArIdCablingTest();

  virtual StatusCode initialize() final;
  virtual StatusCode execute() final;

private:
  SG::ReadCondHandleKey<LArOnOffIdMapping> m_cablingKey{this,"CablingKey","LArOnOffIdMap"};

  Gaudi::Property<bool>m_isSC{this,"isSC",false};

};

#endif
