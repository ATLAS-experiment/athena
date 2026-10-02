/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TESTDRIVER_H
#define TESTDRIVER_H

#include <map>
#include <string>
#include "GaudiKernel/SmartIF.h"

namespace Gaudi {
  class IFileCatalog;
  class IFileCatalogMgr;
}

namespace pool {

  class TestDriver {
  public:
    TestDriver();
    ~TestDriver();
    TestDriver(const TestDriver & ) = delete;
    TestDriver& operator=(const TestDriver & ) = delete;
    void write();
    void read();

  private:
    SmartIF<Gaudi::IFileCatalogMgr>   m_fileCatalogMgr;
    SmartIF<Gaudi::IFileCatalog>      m_fileCatalog;
    std::string           m_fileName;
    std::map< std::string, std::string > m_parameters;
  };

}

#endif
