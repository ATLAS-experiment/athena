/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ATHENAKERNEL_IATHENASHAREDWRITERSVC_H
#define ATHENAKERNEL_IATHENASHAREDWRITERSVC_H

#include "GaudiKernel/IService.h"

class IAthenaSharedWriterSvc : virtual public ::IService {
public:
   DeclareInterfaceID( IAthenaSharedWriterSvc, 1, 0 );

   virtual StatusCode share(int numClients = 0, bool motherClient = false) = 0;
};

#endif
