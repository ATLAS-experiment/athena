/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_TEST_FIXTURES_PHYSLITE_TEST_H
#define COLUMNAR_TEST_FIXTURES_PHYSLITE_TEST_H

#include <span>

class TFile;

namespace columnar
{
  namespace TestUtils
  {
    struct TestDefinition;
    struct UserConfiguration;

    void runXaodTest (const UserConfiguration& userConfiguration, std::span<const TestDefinition> testDefinitions, TFile *file);
    void runXaodArrayTest (const UserConfiguration& userConfiguration, const TestDefinition& testDefinition, TFile *file);
  }
}

#endif