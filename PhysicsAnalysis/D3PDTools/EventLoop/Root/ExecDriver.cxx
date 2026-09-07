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

#include <cerrno>
#include <cstring>
#include <string>
#include <unistd.h>
#include <vector>

//
// method implementations
//

ClassImp(EL::ExecDriver)

namespace EL
{
  void ExecDriver ::
  testInvariant () const
  {}



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
        // pass the actual list of indices to process (on resubmit this is
        // the list of failed segments, not 0..N-1), plus whether this is a
        // resubmit so the worker can tolerate the already-existing scratch
        // directories.
        std::vector<std::string> args;
        args.push_back ("eventloop_exec_worker");
        args.push_back (data.submitDir);
        args.push_back (data.resubmit ? "1" : "0");
        for (std::size_t index : data.batchJobIndices)
          args.push_back (std::to_string (index));

        std::vector<char*> argv;
        for (std::string& arg : args)
          argv.push_back (const_cast<char*> (arg.c_str()));
        argv.push_back (nullptr);

        // this will replace the current program with a new one.  that means
        // that for better (or worse) we will not continue afterwards.  this is
        // fully intentional, as it releases all the memory we used for
        // configuring the job.  that is the whole point of this driver,
        // releasing the >1GB of memory we use for configuration (in large parts
        // ROOT-python dictionaries).
        execvp(argv[0], argv.data());
        auto myerrno = errno;
        ANA_MSG_ERROR ("failed to execute eventloop_exec_worker: " << strerror (myerrno));
        return ::StatusCode::FAILURE;
      }
      break;

    default:
      break;
    }
    return ::StatusCode::SUCCESS;
  }
}
