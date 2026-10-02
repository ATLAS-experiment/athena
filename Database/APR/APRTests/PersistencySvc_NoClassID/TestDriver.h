/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TESTDRIVER_H
#define TESTDRIVER_H

#include <vector>
#include <string>
#include "GaudiKernel/SmartIF.h"

class Token;

namespace Gaudi {
  class IFileCatalog;
  class IFileCatalogMgr;
}

namespace pool {

  class TestDriver {
  public:
    explicit TestDriver(const std::string& filename = "NCI.pool.root", const std::string& catname = "NCI.catatlog.xml" );
    ~TestDriver();
    TestDriver(const TestDriver & ) = delete;
    TestDriver& operator=(const TestDriver & ) = delete;
    void loadLibraries( const std::vector<std::string>& libraries );
    void write();
    void read();
  private:
    SmartIF<Gaudi::IFileCatalogMgr>   m_fileCatalogMgr;
    SmartIF<Gaudi::IFileCatalog>      m_fileCatalog;
    std::string           m_fileName;
    int                   m_events;
    std::vector< Token* > m_tokens;
  };

}

#endif
