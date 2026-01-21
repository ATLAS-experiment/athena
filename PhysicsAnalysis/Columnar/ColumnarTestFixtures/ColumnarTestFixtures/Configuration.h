/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_TEST_FIXTURES_CONFIGURATION_H
#define COLUMNAR_TEST_FIXTURES_CONFIGURATION_H

#include <chrono>
#include <string>
#include <vector>
#include <utility>

class TFile;

namespace asg
{
  class AsgTool;
}

namespace columnar
{
  namespace TestUtils
  {
    /// @brief a struct holding user configuration for the PHYSLITE tests
    ///
    /// Eventually I can hopefully make this configurable at runtime,
    /// but for now this should at least allow me to share the settings
    /// between different files, so that they don't need to be all in
    /// the same file.

    struct UserConfiguration final
    {
      // the target time to run a given tool
      std::chrono::seconds targetTime = std::chrono::seconds(5);

      // the number of events per batch in columnar mode
      unsigned int batchSize = 1000;

      // whether to run the tool a second time to get a "warm" cache measurement
      bool runToolTwice = true;

      // whether to skip all shallow copies in xAOD array mode
      bool skipShallowCopies = false;

      /// whether to measure non-retrieval for empty containers
      bool measureNonAccessForEmpty = false;
    };



    /// @brief the general configuration for a single test
    struct TestDefinition final
    {
      std::string name;

      TFile *file = nullptr;

      asg::AsgTool *tool = nullptr;

      std::vector<std::pair<std::string,std::string>> containerRenames;
    };
  }
}

#endif
