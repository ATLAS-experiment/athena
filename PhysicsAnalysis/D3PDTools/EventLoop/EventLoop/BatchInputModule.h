/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef EVENT_LOOP__BATCH_INPUT_MODULE_H
#define EVENT_LOOP__BATCH_INPUT_MODULE_H

#include <EventLoop/Module.h>
#include <cstdint>
#include <optional>

namespace EL
{
  struct BatchSample;
  struct BatchSegment;

  namespace Detail
  {
    /// @brief the @ref IInputModule implementation for the batch driver

    class BatchInputModule final : public Module
    {
      /// Public Members
      /// ==============

    public:

      using Module::Module;

      Gaudi::Property<int> jobId {this, "jobId", -1,
        "the id/index of the subjob we are processing"};
      Gaudi::Property<std::int64_t> maxEvents {this, "maxEvents", -1,
        "the maximum number of events to process (-1 means all)"};



      /// Inherited Members
      /// =================

    public:

      StatusCode processInputs (ModuleData& data, IInputModuleActions& actions) override;
    };
  }
}

#endif
