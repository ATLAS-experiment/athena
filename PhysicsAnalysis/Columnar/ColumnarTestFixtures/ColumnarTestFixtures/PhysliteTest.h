/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_TEST_FIXTURES_PHYSLITE_TEST_H
#define COLUMNAR_TEST_FIXTURES_PHYSLITE_TEST_H

namespace columnar
{
  namespace TestUtils
  {
    struct TestDefinition;
    struct UserConfiguration;

    void runXaodArrayTest (const UserConfiguration& userConfiguration, const TestDefinition& testDefinition);
  }
}

#endif