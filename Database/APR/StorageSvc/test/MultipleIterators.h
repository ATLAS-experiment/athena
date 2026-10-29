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
  using TestBase::TestBase;

  int execute();

  void testWriting();
  void testReadingParallelSameContainer();
};

#endif
