/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "CxxUtils/checker_macros.h"
ATLAS_NO_CHECK_FILE_THREAD_SAFETY;

#include <gtest/gtest.h>

int main(int , char **) {
  testing::InitGoogleTest();
  return RUN_ALL_TESTS();
}

