/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ATHENAKERNEL_IATHENAIPCTOOL_H
#define ATHENAKERNEL_IATHENAIPCTOOL_H

#include <string>

#include "GaudiKernel/IAlgTool.h"
#include "CxxUtils/checker_macros.h"


class IAthenaIPCTool : virtual public ::IAlgTool {
public:
   DeclareInterfaceID(IAthenaIPCTool, 1, 0);
 
   virtual StatusCode makeServer(int num, const std::string& streamPortSuffix) = 0;
   virtual bool isServer() const = 0;
   virtual StatusCode makeClient(int num, std::string& streamPortSuffix) = 0;
   virtual bool isClient() const = 0;

   virtual StatusCode putEvent ATLAS_NOT_THREAD_SAFE (long eventNumber, const void* source, size_t nbytes, unsigned int status) const = 0;
   virtual StatusCode getLockedEvent(void** target, unsigned int& status) const = 0;
   virtual StatusCode lockEvent(long eventNumber) const = 0;

   virtual StatusCode putObject(const void* source, size_t nbytes, int num = 0) = 0;
   virtual StatusCode getObject(void** target, size_t& nbytes, int num = 0) = 0;
   virtual StatusCode clearObject(const char** tokenString, int& num) = 0;
   virtual StatusCode lockObject(const char* tokenString, int num = 0) = 0;
};

#endif
