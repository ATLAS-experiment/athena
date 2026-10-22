/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TESTDRIVER_H
#define TESTDRIVER_H

#include <vector>
#include <string>
#include "PoolTestBase.h"

class TestDriver : public PoolTestBase {
public:
  TestDriver();
  ~TestDriver();
 
  void testWriting();
  void testParallelReadWrite();
  void testReading();
};

#endif
