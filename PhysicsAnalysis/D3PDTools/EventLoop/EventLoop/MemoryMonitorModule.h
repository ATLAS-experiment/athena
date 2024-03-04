/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



#ifndef EVENT_LOOP__MEMORY_MONITOR_MODULE_H
#define EVENT_LOOP__MEMORY_MONITOR_MODULE_H

#include <EventLoop/Global.h>

#include <AsgMessaging/AsgMessaging.h>
#include <EventLoop/Module.h>
#include <cstdint>
#include <optional>

namespace EL
{
  namespace Detail
  {
    /// \brief a \ref Module monitoring memory usage at various points during
    /// the job
    ///
    /// @note This module is specifically intended to debug the issue with
    /// analysis jobs running out of memory (Feb 24).  Once that issue is
    /// resolved for good, or if there are fundamental issues that break this
    /// module it can be removed.
    ///
    /// @note There is a dedicated test in AnalysisAlgorithms config that runs a
    /// test job with this module enabled to ensure it runs and doesn't break
    /// the output.

    class MemoryMonitorModule final : public Module
    {
      /// Public Members
      /// ==============
    public:

      using Module::Module;

      virtual StatusCode firstInitialize (ModuleData& data) override;
      virtual StatusCode onInitialize (ModuleData& data) override;
      virtual StatusCode onExecute (ModuleData& data) override;
      virtual StatusCode postFirstEvent (ModuleData& data) override;
      virtual StatusCode onFinalize (ModuleData& data) override;
      virtual StatusCode onWorkerEnd (ModuleData& data) override;

      /// @brief the last RSS value read
      ///
      /// This is mostly meant for convenient testing.  It is not used by the
      /// module itself.
      std::optional<std::uint64_t> lastRSS;



      /// Private Members
      /// ===============
    private:

      std::uint64_t m_numExecute = 0;
      std::uint64_t m_executeNext = 1;
      std::uint64_t m_executeStep = 1;
      std::uint64_t m_executeTarget = 30;

      StatusCode printMemoryUsage (const std::string& location);
    };
  }
}

#endif
