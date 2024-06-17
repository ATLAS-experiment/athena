/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef COMMISSIONINGREC_COSTRGTIME_H
#define COMMISSIONINGREC_COSTRGTIME_H

#include "AthenaKernel/CLASS_DEF.h"

class CosTrigTime {

 public:
  CosTrigTime()=default;
  CosTrigTime(const float t);

  void setTime(const float t);

  float time() const {
    return m_time;
  }

 private:
  float m_time=0;

};

CLASS_DEF(CosTrigTime,66710315,0) 

#endif
