/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


//
// includes
//

#include <EventLoop/KubernetesDriver.h>

#include <fstream>
#include <sstream>
#include <TSystem.h>
#include <AsgMessaging/StatusCode.h>
#include <EventLoop/Job.h>
#include <EventLoop/ManagerData.h>
#include <EventLoop/MessageCheck.h>
#include <PathResolver/PathResolver.h>
#include <RootCoreUtils/Assert.h>
#include <RootCoreUtils/ShellExec.h>
#include <RootCoreUtils/StringUtil.h>

//
// method implementations
//

ClassImp(EL::KubernetesDriver)

namespace EL
{
  void KubernetesDriver ::
  testInvariant () const
  {}



  KubernetesDriver ::
  KubernetesDriver ()
  {
    RCU_NEW_INVARIANT (this);
  }



  ::StatusCode KubernetesDriver ::
  doManagerStep (Detail::ManagerData& data) const
  {
    RCU_READ_INVARIANT (this);
    using namespace msgEventLoop;
    ANA_CHECK (BatchDriver::doManagerStep (data));
    switch (data.step)
    {
    case Detail::ManagerStep::batchScriptVar:
      {
        data.batchSkipReleaseSetup = true;
      }
      break;

    case Detail::ManagerStep::submitJob:
    case Detail::ManagerStep::doResubmit:
      {
        const std::string dockerImage {
          data.options.castString(Job::optDockerImage)};

        const std::string dockerOptions {
          data.options.castString(Job::optDockerOptions)};
        if (!dockerOptions.empty())
        {
          ANA_MSG_WARNING ("you specified docker options for kubernetes driver");
          ANA_MSG_WARNING ("this is not supported in this way");
          ANA_MSG_WARNING ("instead you need to provide your own kubernetes config file");
        }

        /// \brief the setup file we use as a template
        const std::string batchSetupFile {
          data.options.castString(Job::optBatchSetupFile, "EventLoop/kubernetes_setup.yml")};


        /// \brief the config file we use as a template
        const std::string batchConfigFile {
          data.options.castString(Job::optBatchConfigFile, "EventLoop/kubernetes_job.yml")};
        std::string baseConfig;
        {
          const std::string resolved {PathResolverFindDataFile (batchConfigFile)};
          if (resolved.empty())
          {
            ANA_MSG_ERROR ("failed to find batch config file " << batchConfigFile);
            return StatusCode::FAILURE;
          }
          std::ifstream file (resolved.c_str());
          if (!file)
          {
            ANA_MSG_ERROR ("failed to open batch config file " << resolved);
            return StatusCode::FAILURE;
          }
          baseConfig = std::string (std::istreambuf_iterator<char>(file),
                                    std::istreambuf_iterator<char>() );
        }
        baseConfig = RCU::substitute (baseConfig, "%%DOCKERIMAGE%%", dockerImage);
        baseConfig = RCU::substitute (baseConfig, "%%SUBMITDIR%%", data.submitDir);

        std::ostringstream basedirName;
        basedirName << data.submitDir << "/tmp";
        if (!data.resubmit)
        {
          if (gSystem->MakeDirectory (basedirName.str().c_str()) != 0)
          {
            ANA_MSG_ERROR ("failed to create directory " << basedirName.str());
            return StatusCode::FAILURE;
          }
        }

        const std::string jobFilePath {data.submitDir + "/job.yml"};
        {
          bool first {true};
          std::ofstream jobFile (jobFilePath.c_str());
          if (!batchSetupFile.empty())
          {
            const std::string resolved {PathResolverFindDataFile (batchSetupFile)};
            if (resolved.empty())
            {
              ANA_MSG_ERROR ("failed to find batch setup file " << batchSetupFile);
              return StatusCode::FAILURE;
            }
            std::ifstream file (resolved.c_str());
            if (!file)
            {
              ANA_MSG_ERROR ("failed to open batch setup file " << resolved);
              return StatusCode::FAILURE;
            }
            std::string setupConfig {std::istreambuf_iterator<char>(file),
                std::istreambuf_iterator<char>()};
            setupConfig = RCU::substitute (setupConfig, "%%DOCKERIMAGE%%", dockerImage);
            setupConfig = RCU::substitute (setupConfig, "%%SUBMITDIR%%", data.submitDir);
            jobFile << setupConfig;
            first = false;
          }

          for (std::size_t jobIndex : data.batchJobIndices)
          {
            std::ostringstream dirName;
            dirName << basedirName.str() << "/" << jobIndex;
            // on resubmit the per-index directory already exists from the
            // first submission, so tolerate that
            if (gSystem->MakeDirectory (dirName.str().c_str()) != 0 && !data.resubmit)
            {
              ANA_MSG_ERROR ("failed to create directory " << dirName.str());
              return StatusCode::FAILURE;
            }

            if (first)
              first = false;
            else
              jobFile << "---\n";

            std::string myConfig = baseConfig;
            myConfig = RCU::substitute (myConfig, "%%JOBINDEX%%", std::to_string (jobIndex));
            std::ostringstream command;
            command << RCU::Shell::quote (data.submitDir) << "/submit/run " << jobIndex;
            myConfig = RCU::substitute (myConfig, "%%COMMAND%%", command.str());

            jobFile << myConfig << "\n";
          }
        }

        std::ostringstream cmd;
        cmd << "kubectl create -f " << jobFilePath;
        RCU::Shell::exec (cmd.str());
        data.submitted = true;
      }
      break;

    default:
      break;
    }
    return ::StatusCode::SUCCESS;
  }
}
