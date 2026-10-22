/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TESTDRIVER_H
#define TESTDRIVER_H

#include "PoolTestBase.h"

#include <vector>
#include <string>

class TestDriver : public PoolTestBase {
public:
  TestDriver();
  ~TestDriver();

  void testWriting();
  void testReading();
};

#endif
