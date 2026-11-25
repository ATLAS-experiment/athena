/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef STORAGESVC_TESTBASE_H
#define STORAGESVC_TESTBASE_H

#include "AthenaBaseComps/AthMessaging.h"
#include "GaudiKernel/ToolHandle.h"
#include "StorageSvc/DbType.h"
#include "StorageSvc/pool.h"

#include <string>
#include <vector>

class IStorageTool;
class FileDescriptor;

/// Small helper base class for pool tests 
class TestBase : public AthMessaging {
   SmartIF<IMessageSvc> m_msgSvc;
public:
  TestBase( const std::string& testname);
  TestBase(const TestBase&) = delete;
  TestBase& operator=(const TestBase&) = delete;
  virtual ~TestBase();

  /// load local dictionary libraries for the test
  void loadLibraries( const std::vector<std::string>& libraries );
  /// create a standard StorageTool 
  ToolHandle<IStorageTool> createStorageTool();
  void startSession( ToolHandle<IStorageTool> storageTool, Io::IoFlag mode, DbType technology );
  void endSession( ToolHandle<IStorageTool> storageTool );
  void endSession( ToolHandle<IStorageTool> storageTool, FileDescriptor& fd );
};


#endif