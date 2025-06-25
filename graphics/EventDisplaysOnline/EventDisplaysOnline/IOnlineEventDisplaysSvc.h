/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef IONLINEEVENTDISPLAYSSVC_H
#define IONLINEEVENTDISPLAYSSVC_H

#include "GaudiKernel/IInterface.h"

class IOnlineEventDisplaysSvc : virtual public IInterface {

public:
  DeclareInterfaceID(IOnlineEventDisplaysSvc, 1, 0);

  virtual ~IOnlineEventDisplaysSvc(){};

  virtual std::string getFileNamePrefix() = 0;
  virtual std::string getStreamName() = 0;
  virtual std::string getEntireOutputStr() = 0;
};

#endif
