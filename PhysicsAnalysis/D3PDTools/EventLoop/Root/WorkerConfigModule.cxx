/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
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
        configFile = PathResolverFindDataFile (configFile);
        TPython::LoadMacro (configFile.c_str());
        WorkerConfig config (&data);
        TPython::Bind (&config, "workerConfig");
        TPython::Eval ("fillWorkerConfig (workerConfig)");
        TPython::Bind (nullptr, "workerConfig");
      }

      return StatusCode::SUCCESS;
    }
  }
}
