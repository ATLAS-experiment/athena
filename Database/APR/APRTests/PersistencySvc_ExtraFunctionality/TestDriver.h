/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TESTDRIVER_H
#define TESTDRIVER_H

#include <vector>
#include <string>
#include "GaudiKernel/SmartIF.h"
#include "TestClassSTLContainersExt.h"

class Token;

namespace Gaudi {
  class IFileCatalog;
  class IFileCatalogMgr;
}

namespace pool {
  class DbType; 

  class TestDriver {
  public:
    explicit TestDriver( const std::string& catname = "PersExtF.catatlog.xml" );
    ~TestDriver();
    TestDriver(const TestDriver & ) = delete;
    TestDriver& operator=(const TestDriver & ) = delete;
    void loadLibraries( const std::vector<std::string>& libraries );
    void write( pool::DbType storageType );
    void read();
    void clearCache();

  private:
    SmartIF<Gaudi::IFileCatalogMgr>   m_fileCatalogMgr;
    SmartIF<Gaudi::IFileCatalog>      m_fileCatalog;
    std::string           m_fileName;
    int                   m_eventsToCommitAndHold;
    int                   m_events;
    std::vector< Token* > m_tokens;
    std::vector< TestClassSTLContainersExt >  m_testClassSTLContainersExt;
  };

}

#endif
