// -*- C++ -*-

/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ATHEXSTOREGATEEXAMPLE_WRITEDATA_H
#define ATHEXSTOREGATEEXAMPLE_WRITEDATA_H 1

#include <string>
#include "AthenaBaseComps/AthAlgorithm.h"

/////////////////////////////////////////////////////////////////////////////

class WriteData:public AthAlgorithm {
public:
  using AthAlgorithm::AthAlgorithm;
  virtual StatusCode initialize() override;
  virtual StatusCode execute() override;

private:
  StatusCode onError();
};


#endif // not ATHEXSTOREGATEEXAMPLE_WRITEDATA_H


