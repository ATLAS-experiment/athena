/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack

#ifndef COLUMNAR_TEST_FIXTURES_PERFORMANCE_DATA_H
#define COLUMNAR_TEST_FIXTURES_PERFORMANCE_DATA_H

#include <optional>
#include <string>

namespace columnar
{
  namespace TestUtils
  {
    /// the performance data for reading a single branch/column
    struct BranchPerfData final
    {
      std::string name;
      std::optional<float> timeRead;
      std::optional<float> timeReadAgain;
      std::optional<float> timeUnpack;
      std::optional<float> timeShallowCopy;
      std::optional<float> timeShallowRegister;
      std::optional<float> entrySize;
      std::optional<float> uncompressedSize;
      std::optional<unsigned> numBaskets;
      std::optional<unsigned> entries;
      std::optional<unsigned> nullEntries;
    };

    /// the performance data for running a single tool
    struct ToolPerfData final
    {
      std::string name;
      std::optional<float> timeCheck;
      std::optional<float> timeCall;
      std::optional<float> timeCall2;
    };
  }
}

#endif
