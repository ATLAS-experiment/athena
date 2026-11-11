/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TESTDRIVER_H
#define TESTDRIVER_H

#include <vector>
#include <string>
#include "TestClassSTLContainersExt.h"

#include "PersTestBase.h"

class Token;
class DbType; 

class TestDriver : public PersTestBase {
  public:
    explicit TestDriver( const std::string& catname = "PersExtF.catatlog.xml" );
  
    int execute();
    void runTestForStorageType(const DbType& storageType);

    void write( DbType storageType );
    void read();
    void clearCache();

  private:
    std::string           m_fileName;
    int                   m_eventsToCommitAndHold;
    int                   m_events;
    std::vector< Token* > m_tokens;
    std::vector< TestClassSTLContainersExt >  m_testClassSTLContainersExt;
};

#endif
