/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TESTDRIVER_H
#define TESTDRIVER_H

#include "TestBase.h"

#include <vector>
#include <string>

class TestDriver : public TestBase {
public:
  TestDriver();
  ~TestDriver();

  void testWriting();
  void testReading();
};

#endif
