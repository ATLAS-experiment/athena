/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



#ifndef EVENT_LOOP__ALGORITHM_TIMER_MODULE_H
#define EVENT_LOOP__ALGORITHM_TIMER_MODULE_H

#include <EventLoop/Global.h>

#include <EventLoop/Module.h>

namespace EL
{
  namespace Detail
  {
    /// \brief a \ref Module wrapping each algorithm with its own
    /// timer
    ///
    /// @note There is a dedicated test in AnalysisAlgorithms config that runs a
    /// test job with this module enabled to ensure it runs and doesn't break
    /// the output.

    class AlgorithmTimerModule final : public Module
    {
      /// Public Members
      /// ==============

    public:

      virtual ::StatusCode firstInitialize (ModuleData& data) override;
    };
  }
}

#endif
