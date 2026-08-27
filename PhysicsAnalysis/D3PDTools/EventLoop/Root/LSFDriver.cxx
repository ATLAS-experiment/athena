/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


//
// includes
//

#include <EventLoop/LSFDriver.h>

#include <AsgMessaging/StatusCode.h>
#include <EventLoop/Job.h>
#include <EventLoop/ManagerData.h>
#include <EventLoop/MessageCheck.h>
#include <TSystem.h>
#include <sstream>

//
// method implementations
//

ClassImp(EL::LSFDriver)

namespace EL
{
  void LSFDriver ::
  testInvariant () const
  {}



  LSFDriver ::
  LSFDriver ()
  {
    RCU_NEW_INVARIANT (this);
  }



  ::StatusCode LSFDriver ::
  doManagerStep (Detail::ManagerData& data) const
  {
    RCU_READ_INVARIANT (this);
    using namespace msgEventLoop;
    ANA_CHECK (BatchDriver::doManagerStep (data));
    switch (data.step)
    {
    case Detail::ManagerStep::submitJob:
    case Detail::ManagerStep::doResubmit:
      {
        // safely ignoring: resubmit

        std::ostringstream cmd;
        cmd << "cd " << data.submitDir << "/submit";
        for (std::size_t iter : data.batchJobIndices)
        {
          cmd << " && bsub " << data.options.castString (Job::optSubmitFlags);
          if (data.options.castBool (Job::optResetShell, true))
            cmd << " -L /bin/bash";
          cmd << " " << data.submitDir << "/submit/run " << iter;
        }
        if (gSystem->Exec (cmd.str().c_str()) != 0)
        {
          ANA_MSG_ERROR ("failed to execute: " << cmd.str());
          return StatusCode::FAILURE;
        }
        data.submitted = true;
      }
      break;

    default:
      break;
    }
    return ::StatusCode::SUCCESS;
  }
}
