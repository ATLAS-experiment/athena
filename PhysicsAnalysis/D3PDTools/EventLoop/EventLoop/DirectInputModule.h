/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef EVENT_LOOP__DIRECT_INPUT_MODULE_H
#define EVENT_LOOP__DIRECT_INPUT_MODULE_H

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

      std::vector<std::string> fileList;
      std::optional<uint64_t> skipEvents;
      std::optional<uint64_t> maxEvents;



      /// Inherited Members
      /// =================

    public:

      StatusCode processInputs (ModuleData& data, IInputModuleActions& actions) override;
    };
  }
}

#endif
