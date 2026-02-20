/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_TEST_FIXTURES_PHYSLITE_TEST_H
#define COLUMNAR_TEST_FIXTURES_PHYSLITE_TEST_H

#include <SGCore/sgkey_t.h>

#include <span>
#include <string>
#include <unordered_map>

class TFile;

namespace columnar
{
  namespace TestUtils
  {
    struct TestDefinition;
    struct UserConfiguration;

    // I never figured out how the keys get calculated, so I looked
    // at what's in the input file, and hard-coded it here.
    extern const std::unordered_map<std::string,SG::sgkey_t> knownKeys;

    void runXaodTest (const UserConfiguration& userConfiguration, std::span<const TestDefinition> testDefinitions, TFile *file);
    void runXaodArrayTest (const UserConfiguration& userConfiguration, const TestDefinition& testDefinition, TFile *file);
  }
}

#endif