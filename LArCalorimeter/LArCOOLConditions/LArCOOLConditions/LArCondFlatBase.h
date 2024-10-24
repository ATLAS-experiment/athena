/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARCONDFLATBASE_H
#define LARCONDFLATBASE_H

#include <string>
#include "GaudiKernel/StatusCode.h"
#include "LArIdentifier/LArOnlineID.h"
#include "AthenaBaseComps/AthMessaging.h"

class LArCondFlatBase
  : public AthMessaging
{

 public:
  LArCondFlatBase(const std::string& name);
  ~LArCondFlatBase() = default;
  StatusCode initializeBase();
  
 protected:
  bool 	m_isInitialized;
  const LArOnlineID*          m_onlineHelper;
};

#endif
