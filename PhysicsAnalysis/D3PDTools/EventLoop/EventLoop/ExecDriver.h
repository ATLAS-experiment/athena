/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef EVENT_LOOP_EXEC_DRIVER_H
#define EVENT_LOOP_EXEC_DRIVER_H

#include <EventLoop/Global.h>

#include <EventLoop/BatchDriver.h>
#include <SampleHandler/Global.h>

namespace EL
{
  /// \brief a \ref Driver for running batch jobs locally as a new process
  ///
  /// The main purpose here is to get rid of the memory overhead from
  /// configuration.  With all the necessary ROOT dictionaries loaded into
  /// python we are consuming more than 1GB of memory.  So the strategy is to
  /// run the configuration in one process, and then replace the existing
  /// process with a new process for running the job.

  class ExecDriver final : public BatchDriver
  {
    //
    // public interface
    //

    /// effects: test the invariant of this object
    /// guarantee: no-fail
  public:
    void testInvariant () const;


    /// effects: standard default constructor
    /// guarantee: strong
    /// failures: low level errors I
  public:
    ExecDriver ();



    //
    // interface inherited from BatchDriver
    //

  protected:
    virtual ::StatusCode
    doManagerStep (Detail::ManagerData& data) const override;



    //
    // private interface
    //

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#pragma GCC diagnostic ignored "-Winconsistent-missing-override"
    ClassDef(ExecDriver, 1);
#pragma GCC diagnostic pop
  };
}

#endif
