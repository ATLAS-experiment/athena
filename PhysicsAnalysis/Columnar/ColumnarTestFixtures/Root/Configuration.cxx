/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack

#include <ColumnarTestFixtures/Configuration.h>

#include <TString.h>
#include <TSystem.h>

#include <nlohmann/json.hpp>

#include <cstdlib>
#include <fstream>
#include <stdexcept>

namespace columnar
{
  namespace TestUtils
  {
    void UserConfiguration ::
    loadFromFile (const std::string& filename)
    {
      std::ifstream file (filename);
      if (!file.is_open())
        throw std::runtime_error ("failed to open configuration file: " + filename);

      auto json = nlohmann::json::parse (file);

      if (json.contains ("targetTime"))
        targetTime = std::chrono::seconds (json["targetTime"].get<int>());
      if (json.contains ("batchSize"))
        batchSize = json["batchSize"].get<unsigned int>();
      if (json.contains ("runToolTwice"))
        runToolTwice = json["runToolTwice"].get<bool>();
      if (json.contains ("skipShallowCopies"))
        skipShallowCopies = json["skipShallowCopies"].get<bool>();
      if (json.contains ("measureNonAccessForEmpty"))
        measureNonAccessForEmpty = json["measureNonAccessForEmpty"].get<bool>();
      if (json.contains("isrntuple"))
        isrntuple = json["isrntuple"].get<bool>();
      if (json.contains ("inputFile"))
        inputFile = json["inputFile"].get<std::string>();
    }

    std::string UserConfiguration ::
    getInputFileName () const
    {
      if (!inputFile.empty())
      {
        TString fileName = inputFile;
        if (gSystem->ExpandPathName (fileName))
          throw std::runtime_error ("failed to expand input file name: " + inputFile);
        return fileName.Data();
      }
      const std::string defaultEnvVar = isrntuple ? "ASG_TEST_FILE_RNTUPLE_LITE_MC" : "ASG_TEST_FILE_LITE_MC";
      const char *fileName = std::getenv (defaultEnvVar.c_str());
      if (fileName == nullptr)
        throw std::runtime_error ("missing " + defaultEnvVar + " (alternatively set \"inputFile\" in the COLUMNAR_TEST_CONFIG file)");
      return fileName;
    }

    UserConfiguration UserConfiguration :: fromEnvironment ()
    {
      UserConfiguration config;
      const char *envPath = std::getenv ("COLUMNAR_TEST_CONFIG");
      if (envPath != nullptr && envPath[0] != '\0')
        config.loadFromFile (envPath);
      return config;
    }
  }
}