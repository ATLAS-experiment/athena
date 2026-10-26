/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef POOLSVC_PERSTESTBASE_H
#define POOLSVC_PERSTESTBASE_H

#include "GaudiKernel/SmartIF.h"

#include "AthenaBaseComps/AthMessaging.h"

#include "PoolSvc/ISessionTool.h"
#include "PoolSvc/FileCatalogUtils.h"

#include <string>
#include <vector>


/// Small helper base class for  tests in PoolSvc
class PersTestBase : public AthMessaging{
public:
  SmartIF<Gaudi::IFileCatalogMgr>   m_fileCatalogMgr;
  SmartIF<Gaudi::IFileCatalog>      m_fileCatalog;

  PersTestBase( const std::string& testname, const std::string& catname );
  virtual ~PersTestBase();
  
  PersTestBase(const PersTestBase&) = delete;
  PersTestBase& operator=(const PersTestBase&) = delete;

  /// load local dictionary libraries for the test
  void loadLibraries( const std::vector<std::string>& libraries );
  /// create a session tool for the test with @sessionName, attach the catalog to it
  ToolHandle<ISessionTool> createSession( Gaudi::IFileCatalog& fileCatalog, const std::string& sessionName );
};

#endif