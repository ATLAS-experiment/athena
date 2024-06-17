/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "CommissionEvent/CosTrigTime.h"

CosTrigTime::CosTrigTime(const float t) :
  m_time(t) {};


void CosTrigTime::setTime(const float t) {
  m_time=t;
}


  
