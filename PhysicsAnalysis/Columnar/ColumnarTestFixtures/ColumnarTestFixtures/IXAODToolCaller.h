/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_TEST_FIXTURES_I_XAOD_TOOL_CALLER_H
#define COLUMNAR_TEST_FIXTURES_I_XAOD_TOOL_CALLER_H

#include <AsgTools/AsgTool.h>

namespace columnar
{
  namespace TestUtils
  {
    /// @brief a wrapper around a CP tool in xAOD mdoe to call it in the
    /// PHYSLITE test
    ///
    /// This goes through the different steps needed to call a tool. For
    /// regular use, those are normally part of a single algorithm
    /// `execute` function, but splitting them up allows to measure
    /// their performance separately.
    ///
    /// It is expected that the instances of this interface will cache
    /// information between calls. That is safe as no function is
    /// `const` and this will never run in multi-threaded mode. It
    /// provides the easiest way to store what in an algorithm would be
    /// local variables.

    class IXAODToolCaller
    {
    public:
      virtual ~IXAODToolCaller () noexcept = default;

      /// the type used for the event store
      using EventStoreType = std::decay_t<decltype (*std::declval<asg::AsgTool>().evtStore())>;

      /// @brief retrieve everything we need from the event store
      virtual StatusCode retrieve (EventStoreType& evtStore) = 0;

      /// @brief do any copying and recording needed
      virtual StatusCode copyRecord (EventStoreType& evtStore, const std::string& postfix) = 0;

      /// @brief call the tool for a single event
      virtual StatusCode call () = 0;
    };
  }
}

#endif
