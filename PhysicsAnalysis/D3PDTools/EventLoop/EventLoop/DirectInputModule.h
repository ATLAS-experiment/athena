/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef EVENT_LOOP__DIRECT_INPUT_MODULE_H
#define EVENT_LOOP__DIRECT_INPUT_MODULE_H

#include <AsgTools/PropertyWrapper.h>
#include <EventLoop/Module.h>
#include <optional>
#include <vector>
#include <cstdint>

namespace EL
{
  namespace Detail
  {
    /// @brief the @ref IInputModule implementation for the direct driver

    class DirectInputModule final : public Module
    {
      /// Public Members
      /// ==============

    public:

      using Module::Module;

      Gaudi::Property<std::vector<std::string>> fileList {this, "fileList", {},
        "the list of files to process"};
      Gaudi::Property<uint64_t> skipEvents {this, "skipEvents", 0,
        "the number of events to skip"};
      Gaudi::Property<int64_t> maxEvents {this, "maxEvents", -1,
        "the maximum number of events to process (-1 means all)"};

      /// Inherited Members
      /// =================

    public:

      StatusCode processInputs (ModuleData& data, IInputModuleActions& actions) override;
    };
  }
}

#endif
