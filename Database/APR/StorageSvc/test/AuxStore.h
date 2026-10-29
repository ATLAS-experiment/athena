/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TESTDRIVER_H
#define TESTDRIVER_H

#include "StorageSvc/DbType.h"
#include <string>
#include <vector>
#include "TestBase.h"

class TestDriver : public TestBase {

public:
   using TestBase::TestBase;

   int execute();
   void runTestForStorageType(const std::string& filename, const pool::DbType& storageType);

   std::string testWriting();
   void testReading(const std::string& testTypeID);

protected:
   std::string          m_fileName;
   pool::DbType         m_storageType;

};

#endif
