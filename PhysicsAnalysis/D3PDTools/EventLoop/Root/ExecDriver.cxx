/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


//
// includes
//

#include <EventLoop/ExecDriver.h>

#include <EventLoop/ManagerData.h>
#include <EventLoop/MessageCheck.h>
#include <RootCoreUtils/ThrowMsg.h>

//
// method implementations
//

ClassImp(EL::ExecDriver)

namespace EL
{
  void ExecDriver ::
  testInvariant () const
  {
    RCU_INVARIANT (this != 0);
  }



  ExecDriver ::
  ExecDriver ()
  {
    RCU_NEW_INVARIANT (this);
  }



  ::StatusCode ExecDriver ::
  doManagerStep (Detail::ManagerData& data) const
  {
    RCU_READ_INVARIANT (this);
    using namespace msgEventLoop;
    ANA_CHECK (BatchDriver::doManagerStep (data));
    switch (data.step)
    {
    case Detail::ManagerStep::batchScriptVar:
      {
        data.batchSkipReleaseSetup = true;
      }
      break;

    case Detail::ManagerStep::submitJob:
    case Detail::ManagerStep::doResubmit:
      {
        // safely ignoring: resubmit

        std::string maxIndex = std::to_string (data.batchJobIndices.size());
        const char *argv[] = { "eventloop_exec_worker", data.submitDir.c_str(), maxIndex.c_str(), nullptr };

        // this will replace the current program with a new one.  that means
        // that for better (or worse) we will not continue afterwards.  this is
        // fully intentional, as it releases all the memory we used for
        // configuring the job.  that is the whole point of this driver,
        // releasing the >1GB of memory we use for configuration (in large parts
        // ROOT-python dictionaries).
        execvp(argv[0], const_cast<char**>(argv));
        auto myerrno = errno;
        RCU_THROW_MSG ("failed to execute eventloop_exec_worker: " + std::string (strerror (myerrno)));
      }
      break;

    default:
      break;
    }
    return ::StatusCode::SUCCESS;
  }
}
