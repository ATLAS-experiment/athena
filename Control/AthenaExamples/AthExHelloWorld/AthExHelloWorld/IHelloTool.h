/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// Comment to modify file to start a test MR

#ifndef ATHEXHELLOWORLD_IHELLOTOOL_H
#define ATHEXHELLOWORLD_IHELLOTOOL_H 1

#include "GaudiKernel/IAlgTool.h"

class IHelloTool : virtual public IAlgTool {
 public:
  virtual StatusCode saySomething() = 0;

  DeclareInterfaceID(IHelloTool, 1, 0);
};

#endif
