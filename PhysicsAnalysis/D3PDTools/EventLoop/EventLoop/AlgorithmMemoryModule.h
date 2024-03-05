/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



#ifndef EVENT_LOOP__ALGORITHM_MEMORY_MODULE_H
#define EVENT_LOOP__ALGORITHM_MEMORY_MODULE_H

#include <EventLoop/Global.h>

#include <EventLoop/Module.h>

namespace EL
{
  namespace Detail
  {
    /// \brief a \ref Module wrapping each algorithm with its own
    /// memory monitor
    ///
    /// @note This module is specifically intended to debug the issue with
    /// analysis jobs running out of memory (Feb 24).  Once that issue is
    /// resolved for good, or if there are fundamental issues that break this
    /// module it can be removed.
    ///
    /// @note There is a dedicated test in AnalysisAlgorithms config that runs a
    /// test job with this module enabled to ensure it runs and doesn't break
    /// the output.

    class AlgorithmMemoryModule final : public Module
    {
      /// Public Members
      /// ==============

    public:

      using Module::Module;

      virtual StatusCode firstInitialize (ModuleData& data) override;
    };
  }
}

#endif
