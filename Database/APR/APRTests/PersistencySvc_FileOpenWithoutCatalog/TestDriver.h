/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TESTDRIVER_H
#define TESTDRIVER_H

#include <vector>
#include <string>
#include "SimpleTestClass.h"
#include "PersistencySvc/DatabaseSpecification.h"

class Token;

namespace pool {
  class IFileCatalog;

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
    IFileCatalog*         m_fileCatalog;
    std::vector< SimpleTestClass > m_simpleTestClass;
  };

}

#endif
