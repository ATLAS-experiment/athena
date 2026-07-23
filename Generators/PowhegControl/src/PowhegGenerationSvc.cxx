// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

#include "PowhegGenerationSvc.h"

#include <cerrno>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <string>
#include <system_error>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

PowhegGenerationSvc::PowhegGenerationSvc(const std::string& name,
                                         ISvcLocator* svcLoc)
    : AthService(name, svcLoc) {}

StatusCode PowhegGenerationSvc::initialize() {
  ATH_CHECK(AthService::initialize());

  if (m_configuration.value().empty()) {
    ATH_MSG_ERROR("No serialized Powheg configuration was provided");
    return StatusCode::FAILURE;
  }

  const std::filesystem::path workdir =
      std::filesystem::absolute(m_workingDirectory.value());
  if (!std::filesystem::is_directory(workdir)) {
    ATH_MSG_ERROR("Powheg working directory does not exist: " << workdir);
    return StatusCode::FAILURE;
  }

  const std::filesystem::path configPath =
      workdir / (".powheg-ca-" + std::to_string(::getpid()) + ".json");
  {
    std::ofstream config(configPath);
    config << m_configuration.value();
    if (!config) {
      ATH_MSG_ERROR("Failed to write Powheg run configuration to "
                    << configPath);
      return StatusCode::FAILURE;
    }
  }

  ATH_MSG_INFO("Starting isolated Powheg generation in " << workdir);
  const pid_t child = ::fork();
  if (child == -1) {
    ATH_MSG_ERROR("Failed to fork Powheg runner: " << std::strerror(errno));
    std::filesystem::remove(configPath);
    return StatusCode::FAILURE;
  }

  if (child == 0) {
    if (::chdir(workdir.c_str()) != 0) {
      _exit(126);
    }
    ::execlp(m_pythonExecutable.value().c_str(),
             m_pythonExecutable.value().c_str(), "-m",
             m_runnerModule.value().c_str(), "--config",
             configPath.c_str(), static_cast<char*>(nullptr));
    _exit(127);
  }

  int status = 0;
  pid_t waited = -1;
  do {
    waited = ::waitpid(child, &status, 0);
  } while (waited == -1 && errno == EINTR);

  std::error_code removeError;
  std::filesystem::remove(configPath, removeError);

  if (waited == -1) {
    ATH_MSG_ERROR("Failed waiting for Powheg runner: "
                  << std::strerror(errno));
    return StatusCode::FAILURE;
  }
  if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
    if (WIFSIGNALED(status)) {
      ATH_MSG_ERROR("Powheg runner terminated by signal "
                    << WTERMSIG(status));
    } else {
      ATH_MSG_ERROR("Powheg runner exited with status "
                    << WEXITSTATUS(status));
    }
    return StatusCode::FAILURE;
  }

  if (m_validateOutput.value()) {
    const std::filesystem::path output =
        std::filesystem::path(m_outputLHE.value()).is_absolute()
            ? std::filesystem::path(m_outputLHE.value())
            : workdir / m_outputLHE.value();
    if (!std::filesystem::is_regular_file(output) ||
        std::filesystem::file_size(output) == 0) {
      ATH_MSG_ERROR("Powheg runner did not produce non-empty LHE output "
                    << output);
      return StatusCode::FAILURE;
    }
    ATH_MSG_INFO("Powheg generation produced " << output);
  }

  return StatusCode::SUCCESS;
}
