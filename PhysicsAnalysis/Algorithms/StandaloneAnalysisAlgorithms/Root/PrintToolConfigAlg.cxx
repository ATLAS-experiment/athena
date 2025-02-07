/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

//
// includes
//

#include <StandaloneAnalysisAlgorithms/PrintToolConfigAlg.h>
#include <AsgTools/ToolStore.h>

#include <filesystem>
#include <fstream>
#include <iostream>

namespace CP {

StatusCode PrintToolConfigAlg::finalize() {
  // Log the current working directory
  ANA_MSG_INFO("The current filesystem path is: " +
               std::filesystem::current_path().generic_string());

  // Log file where configuration will be dumped
  ANA_MSG_INFO("Dumping the tool configuration to: " + m_outputFileName);

  // Open the output file stream
  std::ofstream configFileStream(m_outputFileName);
  if (!configFileStream) {
    ANA_MSG_ERROR("Failed to open file: " + m_outputFileName);
    return StatusCode::FAILURE;
  }

  // Redirect std::cout to the file stream
  std::streambuf* originalCoutBuf = std::cout.rdbuf(configFileStream.rdbuf());

  try {
    asg::ToolStore::dumpToolConfig();  // Dump the configuration
  } catch (const std::exception& e) {
    ANA_MSG_ERROR("Exception while dumping tool configuration: " +
                  std::string(e.what()));
    std::cout.rdbuf(
        originalCoutBuf);  // Restore original buffer before returning
    return StatusCode::FAILURE;
  }

  // Restore original std::cout buffer
  std::cout.rdbuf(originalCoutBuf);

  // Close file stream (optional, as destructor will handle it)
  configFileStream.close();

  ANA_MSG_INFO("Tool configuration dump complete");

  return StatusCode::SUCCESS;
}

}  // namespace CP
