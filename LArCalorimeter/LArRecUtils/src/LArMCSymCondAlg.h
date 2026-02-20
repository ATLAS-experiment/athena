//Dear emacs, this is -*- C++ -*- 

/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARRECCONDITIONS_LARMCSYMCONDGALG_H
#define LARRECCONDITIONS_LARMCSYMCONDGALG_H

#include "AthenaBaseComps/AthCondAlgorithm.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/WriteCondHandleKey.h"

#include "LArCabling/LArOnOffIdMapping.h"
#include "LArRawConditions/LArMCSym.h"

class LArMCSymCondAlg: public AthCondAlgorithm {

public:
  using AthCondAlgorithm::AthCondAlgorithm;

  ~LArMCSymCondAlg()=default;

  StatusCode initialize();
  StatusCode execute(const EventContext& ctx) const;
  StatusCode finalize() {return StatusCode::SUCCESS;}

 private:
  SG::ReadCondHandleKey<LArOnOffIdMapping> m_readKey  {this,"ReadKey","LArOnOffIdMap"};
  SG::WriteCondHandleKey<LArMCSym>         m_writeKey {this,"WriteKey","LArMCSym"};
  BooleanProperty m_isSC {this, "SuperCell", false, "Creating for SC ?"};
};



#endif
