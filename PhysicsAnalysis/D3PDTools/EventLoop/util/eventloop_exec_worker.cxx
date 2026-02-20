/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include <AsgMessaging/MessageCheck.h>
#include <EventLoop/Driver.h>
#include <RootCoreUtils/ShellExec.h>
#include <RootCoreUtils/ThrowMsg.h>
#include <TSystem.h>
#include <xAODRootAccess/Init.h>
#include <fstream>

int main (int argc, char **argv)
{
  using namespace asg::msgUserCode;
  ANA_CHECK_SET_TYPE (int);

  ANA_CHECK (xAOD::Init ());

  if (argc != 3)
  {
    ANA_MSG_ERROR ("invalid number of arguments");
    return -1;
  }

  std::string submitDir = argv[1];
  std::size_t maxIndex = std::stoul (argv[2]);

  std::ostringstream basedirName;
  basedirName << submitDir << "/tmp";
  
  if (gSystem->MakeDirectory (basedirName.str().c_str()) != 0){
    ANA_MSG_ERROR ("failed to create directory " + basedirName.str());
    return -1;
  }

  auto submitSingle = [&] (std::size_t index) noexcept -> StatusCode
  {
    try
    {
      std::ostringstream dirName;
      dirName << basedirName.str() << "/" << index;
      if (gSystem->MakeDirectory (dirName.str().c_str()) != 0)
      {
        ANA_MSG_ERROR ("failed to create directory " + dirName.str());
        return StatusCode::FAILURE;
      }

      std::ostringstream cmd;
      cmd << "cd " << dirName.str() << " && ";
      cmd << RCU::Shell::quote (submitDir) << "/submit/run " << index;
      RCU::Shell::exec (cmd.str());
    } catch (std::exception& e)
    {
      ANA_MSG_ERROR ("exception in job " << index << ": " << e.what());
      return StatusCode::FAILURE;
    }
    return StatusCode::SUCCESS;
  };
  for (std::size_t index = 0u; index != maxIndex; ++ index)
  {
    if (submitSingle (index).isFailure())
      return EXIT_FAILURE;
  }
  // this particular file can be checked to see if a job has
  // been submitted successfully.
  std::ofstream ((submitDir + "/submitted").c_str());
  EL::Driver::retrieve (submitDir);
  return 0;
}
