/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARCONDFLATBASE_H
#define LARCONDFLATBASE_H

#include <string>
#include "AthenaBaseComps/AthMessaging.h"

class LArOnlineID;
class StatusCode;

class LArCondFlatBase
  : public AthMessaging
{

 public:
  LArCondFlatBase(const std::string& name);
  ~LArCondFlatBase() = default;
  StatusCode initializeBase();
  
 protected:
  bool 	m_isInitialized{};
  const LArOnlineID*          m_onlineHelper{};
};

#endif
