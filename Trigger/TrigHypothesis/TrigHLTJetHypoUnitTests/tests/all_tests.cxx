/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "CxxUtils/checker_macros.h"
ATLAS_NO_CHECK_FILE_THREAD_SAFETY;

#include "gmock/gmock.h"

int main(int argc, char **argv) {
  ::testing::InitGoogleMock( &argc, argv );
  return RUN_ALL_TESTS();
}
