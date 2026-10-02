/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TESTDRIVER_H
#define TESTDRIVER_H

#include <vector>
#include <string>
#include "GaudiKernel/SmartIF.h"
#include "SimpleTestClass.h"
#include "PoolSvc/IDatabase.h"

class Token;

namespace Gaudi {
  class IFileCatalog;
  class IFileCatalogMgr;
}

namespace pool {

  class TestDriver {
  public:
    TestDriver(const std::string& filename, const std::string& catname);
    ~TestDriver();
    TestDriver(const TestDriver & ) = delete;
    TestDriver& operator=(const TestDriver & ) = delete;
    void loadLibraries( const std::vector<std::string>& libraries );
    void write();
     // read back using possibly a different path and name type
     void read(const std::string& fileName = "",  // by default use the name used for writing
              DatabaseSpecification::NameType nameType = DatabaseSpecification::PFN);

    std::string           m_fileName;

private:
    SmartIF<Gaudi::IFileCatalogMgr>   m_fileCatalogMgr;
    SmartIF<Gaudi::IFileCatalog>      m_fileCatalog;
    std::vector< SimpleTestClass > m_simpleTestClass;
  };

}

#endif
