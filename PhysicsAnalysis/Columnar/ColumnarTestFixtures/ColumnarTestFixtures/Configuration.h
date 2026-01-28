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
      bool runToolTwice = false;

      // whether to skip all shallow copies in xAOD array mode
      bool skipShallowCopies = false;

      /// whether to measure non-retrieval for empty containers
      bool measureNonAccessForEmpty = false;


      /// @brief load configuration overrides from a JSON file
      ///
      /// Only fields present in the JSON are overridden; others keep
      /// their defaults.
      void loadFromFile (const std::string& filename);

      /// @brief create a UserConfiguration, loading from the file
      /// pointed to by the COLUMNAR_TEST_CONFIG environment variable
      ///
      /// If the environment variable is not set, returns defaults.
      /// If it is set but the file does not exist, throws an error.
      static UserConfiguration fromEnvironment ();
    };



    class IXAODToolCaller;

    /// @brief the general configuration for a single test
    struct TestDefinition final
    {
      /// @brief the tool being tested
      asg::AsgTool *tool = nullptr;

      /// @brief the name identifier for the test
      std::string name = {};

      /// @brief the callback for calling the tool in xAOD mode
      IXAODToolCaller *xAODToolCaller = nullptr;

      /// @brief the container name remappings to apply
      std::vector<std::pair<std::string,std::string>> containerRenames = {};

      /// @brief the systematic variation to apply (empty for nominal)
      std::string sysName = {};

      /// @brief the MET output term names (if empty, MET output columns are omitted)
      std::vector<std::string> metTermNames = {};
    };
  }
}

#endif
