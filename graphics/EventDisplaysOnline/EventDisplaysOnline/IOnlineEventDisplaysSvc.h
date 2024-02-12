
/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef IONLINEEVENTDISPLAYSSVC_H
#define IONLINEEVENTDISPLAYSSVC_H

#include "GaudiKernel/IService.h"


class IOnlineEventDisplaysSvc : virtual public IService {

public:

  virtual ~IOnlineEventDisplaysSvc();
   
  static const InterfaceID& interfaceID();
  virtual std::string getFileNamePrefix() = 0;
  virtual std::string getStreamName() = 0;
  virtual std::string getEntireOutputStr() = 0;
};

inline const InterfaceID& IOnlineEventDisplaysSvc::interfaceID()
{ 
  static const InterfaceID IID_IOnlineEventDisplaysSvc("IOnlineEventDisplaysSvc", 1, 0);
  return IID_IOnlineEventDisplaysSvc; 
}

#endif

