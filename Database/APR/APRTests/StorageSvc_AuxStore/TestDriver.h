/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TESTDRIVER_H
#define TESTDRIVER_H

#include "StorageSvc/DbType.h"
#include <string>
#include <vector>
#include "PoolTestBase.h"

class TestDriver : public PoolTestBase {

public:
   TestDriver(const std::string& filename, pool::DbType storage_type);
   ~TestDriver();

   std::string testWriting();
   void testReading(const std::string& testTypeID);

protected:
   std::string          m_fileName;
   pool::DbType         m_storageType;

};

#endif
