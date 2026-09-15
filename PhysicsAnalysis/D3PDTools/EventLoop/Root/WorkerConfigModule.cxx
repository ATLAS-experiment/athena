/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


//
// includes
//

#include <EventLoop/WorkerConfigModule.h>

#include <EventLoop/Job.h>
#include <EventLoop/ModuleData.h>
#include <EventLoop/WorkerConfig.h>
#include <EventLoop/Worker.h>
#include <PathResolver/PathResolver.h>
#include <SampleHandler/MetaObject.h>
#include <TPython.h>

//
// method implementations
//

namespace EL
{
  namespace Detail
  {
    StatusCode WorkerConfigModule ::
    onInitialize (ModuleData& data)
    {
      std::string configFile = data.m_worker->metaData()->castString (Job::optWorkerConfigFile, "");
      if (!configFile.empty())
      {
        const std::string resolved = PathResolverFindDataFile (configFile);
        if (resolved.empty())
        {
          ANA_MSG_ERROR ("failed to find worker config file " << configFile);
          return StatusCode::FAILURE;
        }
        TPython::LoadMacro (resolved.c_str());
        WorkerConfig config (&data);
        if (!TPython::Bind (&config, "workerConfig"))
        {
          ANA_MSG_ERROR ("failed to bind the worker config object");
          return StatusCode::FAILURE;
        }
        if (!TPython::Exec ("fillWorkerConfig (workerConfig)"))
        {
          ANA_MSG_ERROR ("failed to execute the worker config script " << resolved);
          TPython::Bind (nullptr, "workerConfig");
          return StatusCode::FAILURE;
        }
        TPython::Bind (nullptr, "workerConfig");
      }

      return StatusCode::SUCCESS;
    }
  }
}
