/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TESTDRIVER_H
#define TESTDRIVER_H

#include "StorageSvc/DbType.h"
#include "TestBase.h"

#include <vector>
#include <string>

class TestDriver : public TestBase {
public:
  TestDriver();
  ~TestDriver();
  void testWriting();
  void testReading();

  // default values for this test
  std::string           m_filename      = "BF.pool_test.root";
  std::string           m_objContainerName = "MyObjContainer";
  std::string           m_strContainerName = "MyString";
  int                   m_nObjects      = 10;

  pool::DbType          m_storageType   = pool::ROOTTREE_StorageType;
  bool                  m_commitEveryRow = false;

};

#endif
