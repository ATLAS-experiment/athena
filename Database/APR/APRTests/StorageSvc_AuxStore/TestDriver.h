/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TESTDRIVER_H
#define TESTDRIVER_H

#include "StorageSvc/DbType.h"
#include <string>
#include <vector>

class TestDriver {

public:
   TestDriver(const std::string& filename, pool::DbType storage_type);
   ~TestDriver();

   std::string testWriting();
   void testReading(const std::string& testTypeID);

   static void loadLibraries( const std::vector<std::string>& libraries );

protected:
   std::string          m_fileName;
   pool::DbType         m_storageType;

};

#endif
