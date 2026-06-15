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

    // The keys are calculated as crc64(name) extended with the
    // container CLID and masked to 30 bits (see
    // columnar::computeSgKey, which reproduces
    // SG::StringPool::stringToKey). For sole-target link columns the
    // keys are now computed from ColumnInfo::soleLinkTargetClid; this
    // table remains as a fallback for variant-link targets, which do
    // not carry a CLID in ColumnInfo.
    extern const std::unordered_map<std::string,SG::sgkey_t> knownKeys;

    void runXaodTest (const UserConfiguration& userConfiguration, std::span<const TestDefinition> testDefinitions, TFile *file);
    void runXaodArrayTest (const UserConfiguration& userConfiguration, const TestDefinition& testDefinition, TFile *file);
  }
}

#endif