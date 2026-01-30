/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TESTDRIVER_H
#define TESTDRIVER_H

#include <vector>
#include <string>
#include "SimpleTestClass.h"
#include "TestClassPrimitives.h"
#include "TestClassVectors.h"
#include "TestClassSTLContainers.h"
#include "TestClassSTLContainersExt.h"

class Token;

namespace pool {
  class IFileCatalog;
  class DbType; 

  class TestDriver {
  public:
    explicit TestDriver( const std::string& catname = "PersF.catatlog.xml" );
    ~TestDriver();
    TestDriver(const TestDriver & ) = delete;
    TestDriver& operator=(const TestDriver & ) = delete;
    void loadLibraries( const std::vector<std::string>& libraries );
    void write( pool::DbType storageType );
    void read();
    void readCollections();
    void clearCache();
    void readFileSizes();

  private:
    pool::IFileCatalog*   m_fileCatalog;
    std::string           m_fileName1;
    std::string           m_fileName2;
    int                   m_events;
    int                   m_eventsToCommitAndHold;
    std::vector< Token* >                     m_tokens;
    std::vector< SimpleTestClass >            m_simpleTestClass;
    std::vector< TestClassPrimitives >        m_testClassPrimitives;
    std::vector< TestClassVectors >           m_testClassVectors;
    std::vector< TestClassSTLContainers >     m_testClassSTLContainers;
  };

}

#endif
