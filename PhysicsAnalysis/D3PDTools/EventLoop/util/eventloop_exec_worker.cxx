/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include <AsgMessaging/MessageCheck.h>
#include <EventLoop/Driver.h>
#include <RootCoreUtils/ShellExec.h>
#include <TSystem.h>
#include <xAODRootAccess/Init.h>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

int main (int argc, char **argv)
{
  using namespace asg::msgUserCode;
  ANA_CHECK_SET_TYPE (int);

  ANA_CHECK (xAOD::Init ());

  // arguments: <submitDir> <resubmit:0|1> <index>...
  if (argc < 3)
  {
    ANA_MSG_ERROR ("invalid number of arguments");
    return -1;
  }

  const std::string submitDir = argv[1];
  const bool resubmit = (std::string (argv[2]) == "1");

  std::vector<std::size_t> indices;
  for (int arg = 3; arg != argc; ++ arg)
  {
    try
    {
      indices.push_back (std::stoul (argv[arg]));
    } catch (std::exception& e)
    {
      ANA_MSG_ERROR ("failed to parse job index \"" << argv[arg] << "\": " << e.what());
      return -1;
    }
  }

  const std::string basedirName = submitDir + "/tmp";

  // on resubmit the tmp directory already exists from the first submission
  if (!resubmit && gSystem->MakeDirectory (basedirName.c_str()) != 0)
  {
    ANA_MSG_ERROR ("failed to create directory " + basedirName);
    return -1;
  }

  auto submitSingle = [&] (std::size_t index) noexcept -> StatusCode
  {
    try
    {
      const std::string dirName = basedirName + "/" + std::to_string (index);
      if (gSystem->MakeDirectory (dirName.c_str()) != 0 && !resubmit)
      {
        ANA_MSG_ERROR ("failed to create directory " + dirName);
        return StatusCode::FAILURE;
      }

      std::ostringstream cmd;
      cmd << "cd " << RCU::Shell::quote (dirName) << " && ";
      cmd << RCU::Shell::quote (submitDir) << "/submit/run " << index;
      RCU::Shell::exec (cmd.str());
    } catch (std::exception& e)
    {
      ANA_MSG_ERROR ("exception in job " << index << ": " << e.what());
      return StatusCode::FAILURE;
    }
    return StatusCode::SUCCESS;
  };
  for (std::size_t index : indices)
  {
    if (submitSingle (index).isFailure())
      return EXIT_FAILURE;
  }
  // this particular file can be checked to see if a job has
  // been submitted successfully.
  std::ofstream ((submitDir + "/submitted").c_str());
  if (!EL::Driver::retrieve (submitDir))
  {
    ANA_MSG_ERROR ("failed to retrieve job output in " + submitDir);
    return EXIT_FAILURE;
  }
  return 0;
}
