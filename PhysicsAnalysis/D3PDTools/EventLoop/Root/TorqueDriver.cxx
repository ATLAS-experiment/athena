/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


//
// includes
//

#include <EventLoop/TorqueDriver.h>

#include <AsgMessaging/StatusCode.h>
#include <EventLoop/Job.h>
#include <EventLoop/ManagerData.h>
#include <EventLoop/MessageCheck.h>
#include <RootCoreUtils/ShellExec.h>
#include <TSystem.h>
#include <sstream>

//
// method implementations
//

ClassImp(EL::TorqueDriver)

namespace EL
{
  void TorqueDriver ::
  testInvariant () const
  {}



  TorqueDriver ::
  TorqueDriver ()
  {
    RCU_NEW_INVARIANT (this);
  }



  ::StatusCode TorqueDriver ::
  doManagerStep (Detail::ManagerData& data) const
  {
    RCU_READ_INVARIANT (this);
    using namespace msgEventLoop;
    ANA_CHECK (BatchDriver::doManagerStep (data));
    switch (data.step)
    {
    case Detail::ManagerStep::batchScriptVar:
      {
        data.batchJobId = "EL_JOBID=$PBS_ARRAYID\n";
      }
      break;


    case Detail::ManagerStep::submitJob:
    case Detail::ManagerStep::doResubmit:
      {
        if (data.resubmit)
        {
          ANA_MSG_ERROR ("resubmission not supported for the Torque driver");
          return StatusCode::FAILURE;
        }

        if (data.batchJobIndices.empty())
        {
          ANA_MSG_ERROR ("no job indices to submit");
          return ::StatusCode::FAILURE;
        }
        if (data.batchJobIndices.back() + 1 != data.batchJobIndices.size())
        {
          ANA_MSG_ERROR ("submitting a non-contiguous set of job indices is not supported");
          return ::StatusCode::FAILURE;
        }
        const std::size_t njob = data.batchJobIndices.size();

        std::ostringstream cmd;
        cmd << "cd " << RCU::Shell::quote (data.submitDir) << "/submit && qsub "
            << data.options.castString (Job::optSubmitFlags)
            << " -t 0-" << (njob-1) << " run";
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
