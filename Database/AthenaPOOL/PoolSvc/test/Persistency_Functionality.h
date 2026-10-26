/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TEST_SESSION_FUNCTIONALITY_H
#define TEST_SESSION_FUNCTIONALITY_H

#include "PersTestBase.h"

#include "SimpleTestClass.h"
#include "TestClassPrimitives.h"
#include "TestClassVectors.h"
#include "TestClassSTLContainers.h"
#include "TestClassSTLContainersExt.h"

class Token;

namespace pool {
  class DbType; 
}

class TestDriver : public PersTestBase {
  public:
    explicit TestDriver( const std::string& catname = "PersF.catatlog.xml" );

    int execute();
    void runTestForStorageType(const pool::DbType& storageType);

    void write( pool::DbType storageType );
    void read();
    void readCollections();
    void clearCache();
    void readFileSizes();

  private:
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

#endif

