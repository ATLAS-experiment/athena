/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef EVENT_LOOP__WORKER_CONFIG_MODULE_H
#define EVENT_LOOP__WORKER_CONFIG_MODULE_H

#include <EventLoop/Global.h>

#include <EventLoop/Module.h>

namespace EL
{
  namespace Detail
  {
    /// \brief a \ref Module implementation for running user
    /// configuration on the worker node

    class WorkerConfigModule final : public Module
    {
    public:

      using Module::Module;

      virtual StatusCode onInitialize (ModuleData& data) override;
    };
  }
}

#endif
