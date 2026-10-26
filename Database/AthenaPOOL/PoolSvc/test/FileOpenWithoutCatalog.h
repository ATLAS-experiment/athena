/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef FILEOPENWITHOUTCATALOG_H
#define FILEOPENWITHOUTCATALOG_H

#include "SimpleTestClass.h"
#include "PoolSvc/IDatabase.h"
#include "PersTestBase.h"

#include <vector>
#include <string>

class TestDriver : public PersTestBase {
  public:
    TestDriver(const std::string& filename, const std::string& catname);

    int execute();

    void write();
     // read back using possibly a different path and name type
     void read(const std::string& fileName = "",  // by default use the name used for writing
              pool::DatabaseSpecification::NameType nameType = pool::DatabaseSpecification::PFN);

    std::string           m_fileName;

private:
    std::vector< SimpleTestClass > m_simpleTestClass;
};

#endif
