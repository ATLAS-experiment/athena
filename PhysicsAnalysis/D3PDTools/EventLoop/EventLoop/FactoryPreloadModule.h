/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



#ifndef EVENT_LOOP__FACTORY_PRELOAD_MODULE_H
#define EVENT_LOOP__FACTORY_PRELOAD_MODULE_H

#include <EventLoop/Global.h>

#include <EventLoop/Module.h>
#include <AsgTools/PropertyWrapper.h>

namespace EL
{
  namespace Detail
  {
    /// \brief a \ref Module preloading all the registered factories
    ///
    /// This is just a temporary module to test whether using factories reduces
    /// memory usage.

    class FactoryPreloadModule final : public Module
    {
      /// Public Members
      /// ==============

    public:

      Gaudi::Property<std::string> preloader {this, "preloader", "",
        "the preloader configuration to use"};

      using Module::Module;

      StatusCode onInitialize (ModuleData& data) override;
    };
  }
}

#endif
