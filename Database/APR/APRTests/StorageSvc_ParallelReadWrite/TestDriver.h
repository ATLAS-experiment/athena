/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TESTDRIVER_H
#define TESTDRIVER_H

#include <vector>
#include <string>
#include "TestBase.h"

class TestDriver : public TestBase {
public:
  TestDriver();
  ~TestDriver();
 
  void testWriting();
  void testParallelReadWrite();
  void testReading();
};

#endif
